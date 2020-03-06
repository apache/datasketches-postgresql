/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "frequent_strings_sketch_c_adapter.h"
#include "allocator.h"
#include "postgres_h_substitute.h"

#include <string>
#include <sstream>

#include <frequent_items_sketch.hpp>

typedef std::basic_string<char, std::char_traits<char>, palloc_allocator<char>> string;

// delegate to std::hash<std::string>
struct hash_string {
  std::size_t operator()(const string& s) const {
    return std::hash<std::string>()(s.c_str());
  }
};

struct serde_string {
  void serialize(std::ostream& os, const string* items, unsigned num) {
    for (unsigned i = 0; i < num; i++) {
      uint32_t length = items[i].size();
      os.write((char*)&length, sizeof(length));
      os.write(items[i].c_str(), length);
    }
  }
  void deserialize(std::istream& is, string* items, unsigned num) {
    for (unsigned i = 0; i < num; i++) {
      uint32_t length;
      is.read((char*)&length, sizeof(length));
      new (&items[i]) string;
      items[i].reserve(length);
      auto it = std::istreambuf_iterator<char>(is);
      for (uint32_t j = 0; j < length; j++) {
        items[i].push_back(*it);
        ++it;
      }
    }
  }
  size_t size_of_item(const string& item) {
    return sizeof(uint32_t) + item.size();
  }
  size_t serialize(void* ptr, const string* items, unsigned num) {
    size_t size = sizeof(uint32_t) * num;
    for (unsigned i = 0; i < num; i++) {
      uint32_t length = items[i].size();
      memcpy(ptr, &length, sizeof(length));
      ptr = static_cast<char*>(ptr) + sizeof(uint32_t);
      memcpy(ptr, items[i].c_str(), length);
      ptr = static_cast<char*>(ptr) + length;
      size += length;
    }
    return size;
  }
  size_t deserialize(const void* ptr, string* items, unsigned num) {
    size_t size = sizeof(uint32_t) * num;
    for (unsigned i = 0; i < num; i++) {
      uint32_t length;
      memcpy(&length, ptr, sizeof(length));
      ptr = static_cast<const char*>(ptr) + sizeof(uint32_t);
      new (&items[i]) string(static_cast<const char*>(ptr), length);
      ptr = static_cast<const char*>(ptr) + length;
      size += length;
    }
    return size;
  }
};

typedef datasketches::frequent_items_sketch<string, uint64_t, hash_string, std::equal_to<string>, serde_string, palloc_allocator<string>> frequent_strings_sketch;

void* frequent_strings_sketch_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(frequent_strings_sketch))) frequent_strings_sketch(lg_k);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void frequent_strings_sketch_delete(void* sketchptr) {
  try {
    static_cast<frequent_strings_sketch*>(sketchptr)->~frequent_strings_sketch();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void frequent_strings_sketch_update(void* sketchptr, const char* str, unsigned length, unsigned long long weight) {
  try {
    static_cast<frequent_strings_sketch*>(sketchptr)->update(string(str, length), weight);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void frequent_strings_sketch_merge(void* sketchptr1, const void* sketchptr2) {
  try {
    static_cast<frequent_strings_sketch*>(sketchptr1)->merge(*static_cast<const frequent_strings_sketch*>(sketchptr2));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

char* frequent_strings_sketch_to_string(const void* sketchptr, bool print_items) {
  try {
    std::stringstream s;
    static_cast<const frequent_strings_sketch*>(sketchptr)->to_stream(s, print_items);
    const unsigned len = (unsigned) s.tellp() + 1;
    char* buffer = (char*) palloc(len);
    strncpy(buffer, s.str().c_str(), len);
    return buffer;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

ptr_with_size frequent_strings_sketch_serialize(const void* sketchptr, unsigned header_size) {
  try {
    ptr_with_size p;
    auto bytes = new (palloc(sizeof(frequent_strings_sketch::vector_bytes))) frequent_strings_sketch::vector_bytes(
      static_cast<const frequent_strings_sketch*>(sketchptr)->serialize(header_size)
    );
    p.ptr = bytes->data();
    p.size = bytes->size();;
    return p;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* frequent_strings_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    frequent_strings_sketch* sketchptr = new (palloc(sizeof(frequent_strings_sketch)))
      frequent_strings_sketch(frequent_strings_sketch::deserialize(buffer, length));
    return sketchptr;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

unsigned frequent_strings_sketch_get_serialized_size_bytes(const void* sketchptr) {
  try {
    return static_cast<const frequent_strings_sketch*>(sketchptr)->get_serialized_size_bytes();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

frequent_strings_sketch_result* frequent_strings_sketch_get_frequent_items(void* sketchptr, bool no_false_positives, unsigned long long threshold) {
  try {
    auto data = static_cast<const frequent_strings_sketch*>(sketchptr)->get_frequent_items(
      no_false_positives ? datasketches::frequent_items_error_type::NO_FALSE_POSITIVES : datasketches::frequent_items_error_type::NO_FALSE_NEGATIVES,
      threshold
    );
    auto rows = (frequent_strings_sketch_result_row*) palloc(sizeof(frequent_strings_sketch_result_row) * data.size());
    unsigned i = 0;
    for (auto& it: data) {
      const string& str = it.get_item();
      rows[i].str = (char*) palloc(str.length() + 1);
      strncpy(rows[i].str, str.c_str(), str.length() + 1);
      rows[i].estimate = it.get_estimate();
      rows[i].lower_bound = it.get_lower_bound();
      rows[i].upper_bound = it.get_upper_bound();
      ++i;
    }
    auto result = (frequent_strings_sketch_result*) palloc(sizeof(frequent_strings_sketch_result));;
    result->rows = rows;
    result->num = data.size();
    return result;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
