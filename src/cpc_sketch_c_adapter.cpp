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

#include <sstream>

#include <cpc_sketch.hpp>
#include <cpc_union.hpp>

void* cpc_sketch_new(unsigned lg_k) {
  try {
    datasketches::cpc_init(&palloc, &pfree);
    return new (palloc(sizeof(datasketches::cpc_sketch))) datasketches::cpc_sketch(lg_k, datasketches::DEFAULT_SEED);
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void cpc_sketch_delete(void* sketchptr) {
  try {
    static_cast<datasketches::cpc_sketch*>(sketchptr)->~cpc_sketch();
    pfree(sketchptr);
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void cpc_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<datasketches::cpc_sketch*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

double cpc_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const datasketches::cpc_sketch*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

Datum* cpc_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs) {
  try {
    Datum* est_and_bounds = (Datum*) palloc(sizeof(Datum) * 3);
    est_and_bounds[0] = Float8GetDatum(static_cast<const datasketches::cpc_sketch*>(sketchptr)->get_estimate());
    est_and_bounds[1] = Float8GetDatum(static_cast<const datasketches::cpc_sketch*>(sketchptr)->get_lower_bound(num_std_devs));
    est_and_bounds[2] = Float8GetDatum(static_cast<const datasketches::cpc_sketch*>(sketchptr)->get_upper_bound(num_std_devs));
    return est_and_bounds;
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void cpc_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  try {
    std::stringstream s;
    s << *(static_cast<const datasketches::cpc_sketch*>(sketchptr));
    snprintf(buffer, length, "%s", s.str().c_str());
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void* cpc_sketch_serialize(const void* sketchptr) {
  try {
    auto data = static_cast<const datasketches::cpc_sketch*>(sketchptr)->serialize(VARHDRSZ);
    bytea* buffer = (bytea*) data.first.release();
    const size_t length = data.second;
    SET_VARSIZE(buffer, length);
    return buffer;
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void* cpc_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    datasketches::cpc_init(&palloc, &pfree);
    auto ptr = datasketches::cpc_sketch::deserialize(buffer, length, datasketches::DEFAULT_SEED);
    return ptr.release();
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void* cpc_union_new(unsigned lg_k) {
  try {
    datasketches::cpc_init(&palloc, &pfree);
    return new (palloc(sizeof(datasketches::cpc_union))) datasketches::cpc_union(lg_k, datasketches::DEFAULT_SEED);
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void cpc_union_delete(void* unionptr) {
  try {
    static_cast<datasketches::cpc_union*>(unionptr)->~cpc_union();
    pfree(unionptr);
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void cpc_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<datasketches::cpc_union*>(unionptr)->update(*static_cast<const datasketches::cpc_sketch*>(sketchptr));
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}

void* cpc_union_get_result(void* unionptr) {
  try {
    auto ptr = static_cast<datasketches::cpc_union*>(unionptr)->get_result();
    return ptr.release();
  } catch (std::exception& e) {
    elog(ERROR, "%s", e.what());
  }
}
