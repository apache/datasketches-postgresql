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

#ifndef HLL_SKETCH_C_ADAPTER_H
#define HLL_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptr_with_size.h"

void* hll_sketch_new(unsigned lg_k);
void* hll_sketch_new_tgt_type(unsigned lg_k, unsigned tgt_type);
void hll_sketch_delete(void* sketchptr);

void hll_sketch_update(void* sketchptr, const void* data, unsigned length);
void hll_sketch_merge(void* sketchptr1, const void* sketchptr2);
double hll_sketch_get_estimate(const void* sketchptr);
void** hll_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs);
char* hll_sketch_to_string(const void* sketchptr);

struct ptr_with_size hll_sketch_serialize(const void* sketchptr, unsigned header_size);
void* hll_sketch_deserialize(const char* buffer, unsigned length);

void* hll_union_new(unsigned lg_k);
void hll_union_delete(void* unionptr);
void hll_union_update(void* unionptr, const void* sketchptr);
void* hll_union_get_result(void* unionptr);
void* hll_union_get_result_tgt_type(void* unionptr, unsigned tgt_type);

#ifdef __cplusplus
}
#endif

#endif
