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

#ifndef CPC_SKETCH_C_ADAPTER_H
#define CPC_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

void cpc_init();
void cpc_cleanup();

void* cpc_sketch_new(unsigned lg_k);
void cpc_sketch_delete(void* sketchptr);

void cpc_sketch_update(void* sketchptr, const void* data, unsigned length);
void cpc_sketch_merge(void* sketchptr1, const void* sketchptr2);
double cpc_sketch_get_estimate(const void* sketchptr);
void** cpc_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs);
void cpc_sketch_to_string(const void* sketchptr, char* buffer, unsigned length);

struct ptr_with_size {
  void* ptr;
  unsigned long long size;
};

struct ptr_with_size cpc_sketch_serialize(const void* sketchptr, unsigned header_size);
void* cpc_sketch_deserialize(const char* buffer, unsigned length);

void* cpc_union_new(unsigned lg_k);
void cpc_union_delete(void* unionptr);
void cpc_union_update(void* unionptr, const void* sketchptr);
void* cpc_union_get_result(void* unionptr);

#ifdef __cplusplus
}
#endif

#endif
