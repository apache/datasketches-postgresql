/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "theta_sketch_c_adapter.h"
#include "allocator.h"

extern "C" {
#include <postgres.h>
}

#include <sstream>

#include <theta_sketch.hpp>
#include <theta_union.hpp>
#include <theta_intersection.hpp>
#include <theta_a_not_b.hpp>

typedef datasketches::theta_sketch_alloc<palloc_allocator<void>> theta_sketch_pg;
typedef datasketches::update_theta_sketch_alloc<palloc_allocator<void>> update_theta_sketch_pg;
typedef datasketches::compact_theta_sketch_alloc<palloc_allocator<void>> compact_theta_sketch_pg;
typedef datasketches::theta_union_alloc<palloc_allocator<void>> theta_union_pg;
typedef datasketches::theta_intersection_alloc<palloc_allocator<void>> theta_intersection_pg;
typedef datasketches::theta_a_not_b_alloc<palloc_allocator<void>> theta_a_not_b_pg;

void* theta_sketch_new_default() {
  try {
    return new (palloc(sizeof(update_theta_sketch_pg))) update_theta_sketch_pg(update_theta_sketch_pg::builder().build());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_sketch_new_lgk(unsigned lg_k) {
  try {
    return new (palloc(sizeof(update_theta_sketch_pg))) update_theta_sketch_pg(update_theta_sketch_pg::builder().set_lg_k(lg_k).build());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_sketch_new_lgk_p(unsigned lg_k, float p) {
  try {
    return new (palloc(sizeof(update_theta_sketch_pg))) update_theta_sketch_pg(update_theta_sketch_pg::builder().set_lg_k(lg_k).set_p(p).build());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_sketch_delete(void* sketchptr) {
  try {
    static_cast<theta_sketch_pg*>(sketchptr)->~theta_sketch_pg();
    pfree(sketchptr);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<update_theta_sketch_pg*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_sketch_compact(void* sketchptr) {
  try {
    auto newptr = new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(static_cast<update_theta_sketch_pg*>(sketchptr)->compact());
    static_cast<update_theta_sketch_pg*>(sketchptr)->~update_theta_sketch_pg();
    pfree(sketchptr);
    return newptr;
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

double theta_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const theta_sketch_pg*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  try {
    std::stringstream s;
    static_cast<const theta_sketch_pg*>(sketchptr)->to_stream(s);
    snprintf(buffer, length, s.str().c_str());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_sketch_serialize(const void* sketchptr) {
  try {
    auto data = static_cast<const theta_sketch_pg*>(sketchptr)->serialize(VARHDRSZ);
    bytea* buffer = (bytea*) data.first.release();
    const size_t length = data.second;
    SET_VARSIZE(buffer, length);
    return buffer;
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    auto ptr = theta_sketch_pg::deserialize(buffer, length);
    return ptr.release();
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_union_new_default() {
  try {
    return new (palloc(sizeof(theta_union_pg))) theta_union_pg(theta_union_pg::builder().build());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_union_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(theta_union_pg))) theta_union_pg(theta_union_pg::builder().set_lg_k(lg_k).build());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_union_delete(void* unionptr) {
  try {
    static_cast<theta_union_pg*>(unionptr)->~theta_union_pg();
    pfree(unionptr);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<theta_union_pg*>(unionptr)->update(*static_cast<const theta_sketch_pg*>(sketchptr));
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_union_get_result(const void* unionptr) {
  try {
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(static_cast<const theta_union_pg*>(unionptr)->get_result());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_intersection_new_default() {
  try {
    return new (palloc(sizeof(theta_intersection_pg))) theta_intersection_pg;
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_intersection_delete(void* interptr) {
  try {
    static_cast<theta_intersection_pg*>(interptr)->~theta_intersection_pg();
    pfree(interptr);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void theta_intersection_update(void* interptr, const void* sketchptr) {
  try {
    static_cast<theta_intersection_pg*>(interptr)->update(*static_cast<const theta_sketch_pg*>(sketchptr));
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_intersection_get_result(const void* interptr) {
  try {
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(static_cast<const theta_intersection_pg*>(interptr)->get_result());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* theta_a_not_b(const void* sketchptr1, const void* sketchptr2) {
  try {
    theta_a_not_b_pg a_not_b;
    return new (palloc(sizeof(compact_theta_sketch_pg))) compact_theta_sketch_pg(a_not_b.compute(
      *static_cast<const theta_sketch_pg*>(sketchptr1),
      *static_cast<const theta_sketch_pg*>(sketchptr2)
    ));
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}
