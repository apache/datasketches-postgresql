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

#include <frequent_items_sketch.hpp>

typedef std::basic_string<char, std::char_traits<char>, palloc_allocator<char>> string;

// delegate to std::hash<std::string>
struct hash_string {
  std::size_t operator()(const string& s) const {
    return std::hash<std::string>()(s.c_str());
  }
};

static inline void pg_check_memory_size(size_t requested_index, size_t capacity) {
  if (requested_index > capacity) {
    std::string msg("Attempt to access memory beyond limits: requested index "
    + std::to_string(requested_index) + ", capacity " + std::to_string(capacity));
    pg_error(msg.c_str());
  }
}

struct serde_string {
  size_t size_of_item(const string& item) const {
    return sizeof(uint32_t) + item.size();
  }

  size_t serialize(void* ptr, size_t capacity, const string* items, unsigned num) const {
    size_t size = sizeof(uint32_t) * num;
    for (unsigned i = 0; i < num; i++) {
      const uint32_t length = items[i].size();
      memcpy(ptr, &length, sizeof(length));
      ptr = static_cast<char*>(ptr) + sizeof(uint32_t);
      memcpy(ptr, items[i].c_str(), length);
      ptr = static_cast<char*>(ptr) + length;
      size += length;
    }
    return size;
  }

  size_t deserialize(const void* ptr, size_t capacity, string* items, unsigned num) const {
    size_t bytes_read = 0;
    for (unsigned i = 0; i < num; i++) {
      uint32_t length;
      pg_check_memory_size(bytes_read + sizeof(length), capacity);
      memcpy(&length, ptr, sizeof(length));
      ptr = static_cast<const char*>(ptr) + sizeof(uint32_t);
      bytes_read += sizeof(length);
      pg_check_memory_size(bytes_read + length, capacity);
      new (&items[i]) std::string(static_cast<const char*>(ptr), length);
      ptr = static_cast<const char*>(ptr) + length;
      bytes_read += length;
    }
    return bytes_read;
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
    auto str = static_cast<const frequent_strings_sketch*>(sketchptr)->to_string(print_items);
    const size_t len = str.length() + 1;
    char* buffer = (char*) palloc(len);
    strncpy(buffer, str.c_str(), len);
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
    p.size = bytes->size();
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
    auto result = (frequent_strings_sketch_result*) palloc(sizeof(frequent_strings_sketch_result));
    result->rows = rows;
    result->num = data.size();
    return result;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
