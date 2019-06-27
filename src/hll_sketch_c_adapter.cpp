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

#include <sstream>

#include <hll.hpp>

typedef datasketches::HllSketch<palloc_allocator<char>> hll_sketch;
typedef datasketches::HllUnion<palloc_allocator<char>> hll_union;

void* hll_sketch_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(hll_sketch))) hll_sketch(lg_k);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* hll_sketch_new_tgt_type(unsigned lg_k, unsigned tgt_type) {
  try {
    return new (palloc(sizeof(hll_sketch))) hll_sketch(
      lg_k,
      tgt_type == 4 ? datasketches::TgtHllType::HLL_4 : tgt_type == 6 ? datasketches::TgtHllType::HLL_6 : datasketches::TgtHllType::HLL_8
    );
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void hll_sketch_delete(void* sketchptr) {
  try {
    static_cast<hll_sketch*>(sketchptr)->~hll_sketch();
    pfree(sketchptr);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void hll_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<hll_sketch*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

double hll_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const hll_sketch*>(sketchptr)->getEstimate();
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

Datum* hll_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs) {
  try {
    Datum* est_and_bounds = (Datum*) palloc(sizeof(Datum) * 3);
    est_and_bounds[0] = Float8GetDatum(static_cast<const hll_sketch*>(sketchptr)->getEstimate());
    est_and_bounds[1] = Float8GetDatum(static_cast<const hll_sketch*>(sketchptr)->getLowerBound(num_std_devs));
    est_and_bounds[2] = Float8GetDatum(static_cast<const hll_sketch*>(sketchptr)->getUpperBound(num_std_devs));
    return est_and_bounds;
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void hll_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  try {
    std::stringstream s;
    static_cast<const hll_sketch*>(sketchptr)->to_string(s);
    snprintf(buffer, length, s.str().c_str());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* hll_sketch_serialize(const void* sketchptr) {
  try {
    auto data = static_cast<const hll_sketch*>(sketchptr)->serializeCompact(VARHDRSZ);
    bytea* buffer = (bytea*) data.first.release();
    const size_t length = data.second;
    SET_VARSIZE(buffer, length);
    return buffer;
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* hll_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    hll_sketch* sketchptr = new (palloc(sizeof(hll_sketch))) hll_sketch(hll_sketch::deserialize(buffer, length));
    return sketchptr;
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* hll_union_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(hll_union))) hll_union(lg_k);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void hll_union_delete(void* unionptr) {
  try {
    static_cast<hll_union*>(unionptr)->~hll_union();
    pfree(unionptr);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void hll_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<hll_union*>(unionptr)->update(*static_cast<const hll_sketch*>(sketchptr));
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* hll_union_get_result(void* unionptr) {
  try {
    return new (palloc(sizeof(hll_sketch))) hll_sketch(static_cast<hll_union*>(unionptr)->getResult());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* hll_union_get_result_tgt_type(void* unionptr, unsigned tgt_type) {
  try {
    return new (palloc(sizeof(hll_sketch))) hll_sketch(static_cast<hll_union*>(unionptr)->getResult(
      tgt_type == 4 ? datasketches::TgtHllType::HLL_4 : tgt_type == 6 ? datasketches::TgtHllType::HLL_6 : datasketches::TgtHllType::HLL_8
    ));
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}
