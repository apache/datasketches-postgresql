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

#include "theta_sketch_c_adapter.h"
#include "allocator.h"
#include "postgres_h_substitute.h"

#include <theta_sketch.hpp>
#include <theta_union.hpp>
#include <theta_intersection.hpp>
#include <theta_a_not_b.hpp>

typedef datasketches::theta_sketch_alloc<palloc_allocator<uint64_t>> theta_sketch_pg;
typedef datasketches::update_theta_sketch_alloc<palloc_allocator<uint64_t>> update_theta_sketch_pg;
typedef datasketches::compact_theta_sketch_alloc<palloc_allocator<uint64_t>> compact_theta_sketch_pg;
typedef datasketches::theta_union_alloc<palloc_allocator<uint64_t>> theta_union_pg;
typedef datasketches::theta_intersection_alloc<palloc_allocator<uint64_t>> theta_intersection_pg;
typedef datasketches::theta_a_not_b_alloc<palloc_allocator<uint64_t>> theta_a_not_b_pg;
typedef datasketches::wrapped_compact_theta_sketch_alloc<palloc_allocator<uint64_t>> wrapped_compact_theta_sketch_pg;

void* theta_sketch_new_default() {
  try {
    return new (palloc(sizeof(update_theta_sketch_pg))) update_theta_sketch_pg(update_theta_sketch_pg::builder().build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_sketch_new_lgk(unsigned lg_k) {
  try {
    return new (palloc(sizeof(update_theta_sketch_pg))) update_theta_sketch_pg(update_theta_sketch_pg::builder().set_lg_k(lg_k).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_sketch_new_lgk_p(unsigned lg_k, float p) {
  try {
    return new (palloc(sizeof(update_theta_sketch_pg))) update_theta_sketch_pg(update_theta_sketch_pg::builder().set_lg_k(lg_k).set_p(p).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void theta_sketch_delete(void* sketchptr) {
  try {
    static_cast<theta_sketch_pg*>(sketchptr)->~theta_sketch_pg();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void theta_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<update_theta_sketch_pg*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* theta_sketch_compact(void* sketchptr) {
  try {
    auto newptr = new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(static_cast<update_theta_sketch_pg*>(sketchptr)->compact());
    static_cast<update_theta_sketch_pg*>(sketchptr)->~update_theta_sketch_pg();
    pfree(sketchptr);
    return newptr;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

double theta_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const theta_sketch_pg*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* theta_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs) {
  try {
    Datum* est_and_bounds = (Datum*) palloc(sizeof(Datum) * 3);
    est_and_bounds[0] = pg_float8_get_datum(static_cast<const theta_sketch_pg*>(sketchptr)->get_estimate());
    est_and_bounds[1] = pg_float8_get_datum(static_cast<const theta_sketch_pg*>(sketchptr)->get_lower_bound(num_std_devs));
    est_and_bounds[2] = pg_float8_get_datum(static_cast<const theta_sketch_pg*>(sketchptr)->get_upper_bound(num_std_devs));
    return est_and_bounds;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

char* theta_sketch_to_string(const void* sketchptr) {
  try {
    auto str = static_cast<const theta_sketch_pg*>(sketchptr)->to_string();
    const size_t len = str.length() + 1;
    char* buffer = (char*) palloc(len);
    strncpy(buffer, str.c_str(), len);
    return buffer;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

ptr_with_size theta_sketch_serialize(const void* sketchptr, unsigned header_size) {
  try {
    ptr_with_size p;
    auto bytes = new (palloc(sizeof(compact_theta_sketch_pg::vector_bytes))) compact_theta_sketch_pg::vector_bytes(
      static_cast<const compact_theta_sketch_pg*>(sketchptr)->serialize(header_size)
    );
    p.ptr = bytes->data();
    p.size = bytes->size();
    return p;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(compact_theta_sketch_pg::deserialize(buffer, length));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_union_new_default() {
  try {
    return new (palloc(sizeof(theta_union_pg))) theta_union_pg(theta_union_pg::builder().build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_union_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(theta_union_pg))) theta_union_pg(theta_union_pg::builder().set_lg_k(lg_k).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void theta_union_delete(void* unionptr) {
  try {
    static_cast<theta_union_pg*>(unionptr)->~theta_union_pg();
    pfree(unionptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void theta_union_update(void* unionptr, const void* buffer, unsigned length) {
  try {
    static_cast<theta_union_pg*>(unionptr)->update(wrapped_compact_theta_sketch_pg::wrap(buffer, length));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* theta_union_get_result(const void* unionptr) {
  try {
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(static_cast<const theta_union_pg*>(unionptr)->get_result());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_intersection_new_default() {
  try {
    return new (palloc(sizeof(theta_intersection_pg))) theta_intersection_pg;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void theta_intersection_delete(void* interptr) {
  try {
    static_cast<theta_intersection_pg*>(interptr)->~theta_intersection_pg();
    pfree(interptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void theta_intersection_update(void* interptr, const void* buffer, unsigned length) {
  try {
    static_cast<theta_intersection_pg*>(interptr)->update(wrapped_compact_theta_sketch_pg::wrap(buffer, length));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* theta_intersection_get_result(const void* interptr) {
  try {
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(static_cast<const theta_intersection_pg*>(interptr)->get_result());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* theta_a_not_b(const void* sketchptr1, const void* sketchptr2) {
  try {
    theta_a_not_b_pg a_not_b;
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(a_not_b.compute(
      *static_cast<const theta_sketch_pg*>(sketchptr1),
      *static_cast<const theta_sketch_pg*>(sketchptr2)
    ));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
