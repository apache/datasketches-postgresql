/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef CPC_SKETCH_C_ADAPTER_H
#define CPC_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

void* cpc_sketch_new(unsigned lg_k);
void cpc_sketch_delete(void* sketchptr);

void cpc_sketch_update(void* sketchptr, const void* data, unsigned length);
void cpc_sketch_merge(void* sketchptr1, const void* sketchptr2);
double cpc_sketch_get_estimate(const void* sketchptr);
void cpc_sketch_to_string(const void* sketchptr, char* buffer, unsigned length);

void* cpc_sketch_serialize(const void* sketchptr);
void* cpc_sketch_deserialize(const char* buffer, unsigned length);

void* cpc_union_new(unsigned lg_k);
void cpc_union_update(void* unionptr, const void* sketchptr);
void* cpc_union_get_result(void* unionptr);

#ifdef __cplusplus
}
#endif

#endif
