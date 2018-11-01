/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "kll_float_sketch_c_adapter.h"
#include "allocator.h"

#include <sstream>

#include <kll_sketch.hpp>

typedef datasketches::kll_sketch<float, palloc_allocator<void>> kll_float_sketch;

void* kll_float_sketch_new(unsigned k) {
  return new (palloc(sizeof(kll_float_sketch))) kll_float_sketch(k);
}

void kll_float_sketch_delete(void* sketchptr) {
  static_cast<kll_float_sketch*>(sketchptr)->~kll_float_sketch();
  pfree(sketchptr);
}

void kll_float_sketch_update(void* sketchptr, float value) {
  static_cast<kll_float_sketch*>(sketchptr)->update(value);
}

void kll_float_sketch_merge(void* sketchptr1, const void* sketchptr2) {
  static_cast<kll_float_sketch*>(sketchptr1)->merge(*static_cast<const kll_float_sketch*>(sketchptr2));
}

double kll_float_sketch_get_rank(const void* sketchptr, float value) {
  return static_cast<const kll_float_sketch*>(sketchptr)->get_rank(value);
}

float kll_float_sketch_get_quantile(const void* sketchptr, double rank) {
  return static_cast<const kll_float_sketch*>(sketchptr)->get_quantile(rank);
}

void kll_float_sketch_to_string(const void* sketchptr, char* buffer, unsigned length) {
  std::stringstream s;
  s << *(static_cast<const kll_float_sketch*>(sketchptr));
  snprintf(buffer, length, s.str().c_str());
}

void kll_float_sketch_serialize(const void* sketchptr, char* buffer) {
  // intermediate copy in unavoidable with the current kll_sketch API
  // potential improvement
  std::stringstream s;
  static_cast<const kll_float_sketch*>(sketchptr)->serialize(s);
  s.read(buffer, s.tellp());
}

void* kll_float_sketch_deserialize(const char* buffer, unsigned length) {
  // intermediate copy in unavoidable with the current kll_sketch API
  // potential improvement
  std::stringstream s;
  s.write(buffer, length);
  auto ptr = kll_float_sketch::deserialize(s);
  return ptr.release();
}

unsigned kll_float_sketch_get_serialized_size_bytes(const void* sketchptr) {
  return static_cast<const kll_float_sketch*>(sketchptr)->get_serialized_size_bytes();
}
