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

#include "cpc_sketch_c_adapter.h"
#include "allocator.h"
#include "postgres_h_substitute.h"

#include <cpc_sketch.hpp>
#include <cpc_union.hpp>

typedef datasketches::cpc_sketch_alloc<palloc_allocator<char>> cpc_sketch_pg;
typedef datasketches::cpc_union_alloc<palloc_allocator<char>> cpc_union_pg;

void cpc_init() {
  datasketches::cpc_init<palloc_allocator<char>>();
}

void cpc_cleanup() {
}

void* cpc_sketch_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(cpc_sketch_pg))) cpc_sketch_pg(lg_k, datasketches::DEFAULT_SEED);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void cpc_sketch_delete(void* sketchptr) {
  try {
    static_cast<cpc_sketch_pg*>(sketchptr)->~cpc_sketch_pg();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void cpc_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<cpc_sketch_pg*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

double cpc_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const cpc_sketch_pg*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* cpc_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs) {
  try {
    Datum* est_and_bounds = (Datum*) palloc(sizeof(Datum) * 3);
    est_and_bounds[0] = pg_float8_get_datum(static_cast<const cpc_sketch_pg*>(sketchptr)->get_estimate());
    est_and_bounds[1] = pg_float8_get_datum(static_cast<const cpc_sketch_pg*>(sketchptr)->get_lower_bound(num_std_devs));
    est_and_bounds[2] = pg_float8_get_datum(static_cast<const cpc_sketch_pg*>(sketchptr)->get_upper_bound(num_std_devs));
    return est_and_bounds;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

char* cpc_sketch_to_string(const void* sketchptr) {
  try {
    auto str = static_cast<const cpc_sketch_pg*>(sketchptr)->to_string();
    const size_t len = str.length() + 1;
    char* buffer = (char*) palloc(len);
    strncpy(buffer, str.c_str(), len);
    return buffer;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

struct ptr_with_size cpc_sketch_serialize(const void* sketchptr, unsigned header_size) {
  try {
    ptr_with_size p;
    auto bytes = new (palloc(sizeof(cpc_sketch_pg::vector_bytes))) cpc_sketch_pg::vector_bytes(
      static_cast<const cpc_sketch_pg*>(sketchptr)->serialize(header_size)
    );
    p.ptr = bytes->data();
    p.size = bytes->size();
    return p;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* cpc_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    return new (palloc(sizeof(cpc_sketch_pg))) cpc_sketch_pg(cpc_sketch_pg::deserialize(buffer, length, datasketches::DEFAULT_SEED));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* cpc_union_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(cpc_union_pg))) cpc_union_pg(lg_k, datasketches::DEFAULT_SEED);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void cpc_union_delete(void* unionptr) {
  try {
    static_cast<cpc_union_pg*>(unionptr)->~cpc_union_pg();
    pfree(unionptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void cpc_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<cpc_union_pg*>(unionptr)->update(*static_cast<const cpc_sketch_pg*>(sketchptr));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* cpc_union_get_result(void* unionptr) {
  try {
    return new (palloc(sizeof(cpc_sketch_pg))) cpc_sketch_pg(static_cast<cpc_union_pg*>(unionptr)->get_result());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
