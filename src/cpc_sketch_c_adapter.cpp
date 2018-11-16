/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "cpc_sketch_c_adapter.h"

extern "C" {
#include <postgres.h>
}

#include <sstream>

#include <cpc_sketch.hpp>
#include <cpc_union.hpp>

void* cpc_sketch_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(datasketches::cpc_sketch))) datasketches::cpc_sketch(lg_k, datasketches::DEFAULT_SEED, &palloc, &pfree);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void cpc_sketch_delete(void* sketchptr) {
  try {
    static_cast<datasketches::cpc_sketch*>(sketchptr)->~cpc_sketch();
    pfree(sketchptr);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void cpc_sketch_update(void* sketchptr, const void* data, unsigned length) {
  try {
    static_cast<datasketches::cpc_sketch*>(sketchptr)->update(data, length);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

double cpc_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const datasketches::cpc_sketch*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void cpc_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  try {
    std::stringstream s;
    s << *(static_cast<const datasketches::cpc_sketch*>(sketchptr));
    snprintf(buffer, length, s.str().c_str());
  } catch (std::exception& e) {
    elog(ERROR, e.what());
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
    elog(ERROR, e.what());
  }
}

void* cpc_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    auto ptr = datasketches::cpc_sketch::deserialize(buffer, length, datasketches::DEFAULT_SEED, &palloc, &pfree);
    return ptr.release();
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* cpc_union_new(unsigned lg_k) {
  try {
    return new (palloc(sizeof(datasketches::cpc_union))) datasketches::cpc_union(lg_k, datasketches::DEFAULT_SEED, &palloc, &pfree);
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void cpc_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<datasketches::cpc_union*>(unionptr)->update(*static_cast<const datasketches::cpc_sketch*>(sketchptr));
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}

void* cpc_union_get_result(void* unionptr) {
  try {
    auto ptr = static_cast<datasketches::cpc_union*>(unionptr)->get_result();
    return ptr.release();
  } catch (std::exception& e) {
    elog(ERROR, e.what());
  }
}
