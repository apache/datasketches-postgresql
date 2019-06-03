/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef THETA_SKETCH_C_ADAPTER_H
#define THETA_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

void* theta_sketch_new_default();
void* theta_sketch_new(unsigned lg_k);
void theta_sketch_delete(void* sketchptr);

void theta_sketch_update(void* sketchptr, const void* data, unsigned length);
void* theta_sketch_compact(void* sketchptr);
void theta_sketch_union(void* sketchptr1, const void* sketchptr2);
double theta_sketch_get_estimate(const void* sketchptr);
void theta_sketch_to_string(const void* sketchptr, char* buffer, unsigned length);

void* theta_sketch_serialize(const void* sketchptr);
void* theta_sketch_deserialize(const char* buffer, unsigned length);

void* theta_union_new_default();
void* theta_union_new(unsigned lg_k);
void theta_union_delete(void* unionptr);
void theta_union_update(void* unionptr, const void* sketchptr);
void* theta_union_get_result(void* unionptr);

#ifdef __cplusplus
}
#endif

#endif
