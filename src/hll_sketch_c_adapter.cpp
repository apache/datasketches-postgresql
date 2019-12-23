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

#include "hll_sketch_c_adapter.h"
#include "allocator.h"
#include "postgres_h_substitute.h"

#include <sstream>

#include <hll.hpp>

typedef datasketches::hll_sketch_alloc<palloc_allocator<char>> hll_sketch_pg;
typedef datasketches::hll_union_alloc<palloc_allocator<char>> hll_union_pg;

void* hll_sketch_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(hll_sketch_pg))) hll_sketch_pg(lg_k);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* hll_sketch_new_tgt_type(unsigned lg_k, unsigned tgt_type) {
  try {
    return new (palloc(sizeof(hll_sketch_pg))) hll_sketch_pg(
      lg_k,
      tgt_type == 4 ? datasketches::target_hll_type::HLL_4 : tgt_type == 6 ? datasketches::target_hll_type::HLL_6 : datasketches::target_hll_type::HLL_8
    );
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void hll_sketch_delete(void* sketchptr) {
  try {
    static_cast<hll_sketch_pg*>(sketchptr)->~hll_sketch_pg();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void hll_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<hll_sketch_pg*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

double hll_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const hll_sketch_pg*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* hll_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs) {
  try {
    Datum* est_and_bounds = (Datum*) palloc(sizeof(Datum) * 3);
    est_and_bounds[0] = pg_float8_get_datum(static_cast<const hll_sketch_pg*>(sketchptr)->get_estimate());
    est_and_bounds[1] = pg_float8_get_datum(static_cast<const hll_sketch_pg*>(sketchptr)->get_lower_bound(num_std_devs));
    est_and_bounds[2] = pg_float8_get_datum(static_cast<const hll_sketch_pg*>(sketchptr)->get_upper_bound(num_std_devs));
    return est_and_bounds;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void hll_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  try {
    std::stringstream s;
    static_cast<const hll_sketch_pg*>(sketchptr)->to_string(s);
    snprintf(buffer, length, "%s", s.str().c_str());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

ptr_with_size hll_sketch_serialize(const void* sketchptr, unsigned header_size) {
  try {
    ptr_with_size p;
    auto bytes = new (palloc(sizeof(hll_sketch_pg::vector_bytes))) hll_sketch_pg::vector_bytes(
      static_cast<const hll_sketch_pg*>(sketchptr)->serialize_compact(header_size)
    );
    p.ptr = bytes->data();
    p.size = bytes->size();
    return p;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* hll_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    hll_sketch_pg* sketchptr = new (palloc(sizeof(hll_sketch_pg))) hll_sketch_pg(hll_sketch_pg::deserialize(buffer, length));
    return sketchptr;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* hll_union_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(hll_union_pg))) hll_union_pg(lg_k);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void hll_union_delete(void* unionptr) {
  try {
    static_cast<hll_union_pg*>(unionptr)->~hll_union_pg();
    pfree(unionptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void hll_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<hll_union_pg*>(unionptr)->update(*static_cast<const hll_sketch_pg*>(sketchptr));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* hll_union_get_result(void* unionptr) {
  try {
    return new (palloc(sizeof(hll_sketch_pg))) hll_sketch_pg(static_cast<hll_union_pg*>(unionptr)->get_result());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* hll_union_get_result_tgt_type(void* unionptr, unsigned tgt_type) {
  try {
    return new (palloc(sizeof(hll_sketch_pg))) hll_sketch_pg(static_cast<hll_union_pg*>(unionptr)->get_result(
      tgt_type == 4 ? datasketches::target_hll_type::HLL_4 : tgt_type == 6 ? datasketches::target_hll_type::HLL_6 : datasketches::target_hll_type::HLL_8
    ));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
