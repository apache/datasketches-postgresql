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
  return new (palloc(sizeof(datasketches::cpc_sketch))) datasketches::cpc_sketch(lg_k, datasketches::DEFAULT_SEED, &palloc, &pfree);
}

void cpc_sketch_delete(void* sketchptr) {
  static_cast<datasketches::cpc_sketch*>(sketchptr)->~cpc_sketch();
  pfree(sketchptr);
}

void cpc_sketch_update(void* sketchptr, const void* data, unsigned length) {
  static_cast<datasketches::cpc_sketch*>(sketchptr)->update(data, length);
}

double cpc_sketch_get_estimate(const void* sketchptr) {
  return static_cast<const datasketches::cpc_sketch*>(sketchptr)->get_estimate();
}

void cpc_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  std::stringstream s;
  s << *(static_cast<const datasketches::cpc_sketch*>(sketchptr));
  snprintf(buffer, length, s.str().c_str());
}

void* cpc_sketch_serialize(const void* sketchptr) {
  // intermediate copy in unavoidable with the current cpc_sketch API
  // potential improvement
  std::stringstream s;
  static_cast<const datasketches::cpc_sketch*>(sketchptr)->serialize(s);
  unsigned length = s.tellp();
  bytea* buffer = (bytea*) palloc(length + VARHDRSZ);
  SET_VARSIZE(buffer, length + VARHDRSZ);
  s.read(VARDATA(buffer), length);
  return buffer;
}

void* cpc_sketch_deserialize(const char* buffer, unsigned length) {
  // intermediate copy in unavoidable with the current cpc_sketch API
  // potential improvement
  std::stringstream s;
  s.write(buffer, length);
  auto ptr = datasketches::cpc_sketch::deserialize(s, datasketches::DEFAULT_SEED, &palloc, &pfree);
  return ptr.release();
}

void* cpc_union_new(unsigned lg_k) {
  return new (palloc(sizeof(datasketches::cpc_union))) datasketches::cpc_union(lg_k, datasketches::DEFAULT_SEED, &palloc, &pfree);
}

void cpc_union_update(void* unionptr, const void* sketchptr) {
  static_cast<datasketches::cpc_union*>(unionptr)->update(*static_cast<const datasketches::cpc_sketch*>(sketchptr));
}

void* cpc_union_get_result(void* unionptr) {
  auto ptr = static_cast<datasketches::cpc_union*>(unionptr)->get_result();
  return ptr.release();
}
