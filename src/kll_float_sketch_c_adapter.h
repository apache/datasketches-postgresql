/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef KLL_FLOAT_SKETCH_C_ADAPTER_H
#define KLL_FLOAT_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

void* kll_float_sketch_new(unsigned k);
void kll_float_sketch_delete(void* sketchptr);

void kll_float_sketch_update(void* sketchptr, float value);
void kll_float_sketch_merge(void* sketchptr1, const void* sketchptr2);
double kll_float_sketch_get_rank(const void* sketchptr, float value);
float kll_float_sketch_get_quantile(const void* sketchptr, double rank);
void kll_float_sketch_to_string(const void* sketchptr, char* buffer, unsigned length);

void* kll_float_sketch_serialize(const void* sketchptr);
void* kll_float_sketch_deserialize(const char* buffer, unsigned length);
unsigned kll_float_sketch_get_serialized_size_bytes(const void* sketchptr);

#ifdef __cplusplus
}
#endif

#endif
