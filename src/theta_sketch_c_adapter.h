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

#ifndef THETA_SKETCH_C_ADAPTER_H
#define THETA_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

void* theta_sketch_new_default();
void* theta_sketch_new_lgk(unsigned lg_k);
void* theta_sketch_new_lgk_p(unsigned lg_k, float p);
void theta_sketch_delete(void* sketchptr);

void theta_sketch_update(void* sketchptr, const void* data, unsigned length);
void* theta_sketch_compact(void* sketchptr);
void theta_sketch_union(void* sketchptr1, const void* sketchptr2);
double theta_sketch_get_estimate(const void* sketchptr);
void** theta_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs);
char* theta_sketch_to_string(const void* sketchptr);

struct ptr_with_size {
  void* ptr;
  unsigned long long size;
};

struct ptr_with_size theta_sketch_serialize(const void* sketchptr, unsigned header_size);
void* theta_sketch_deserialize(const char* buffer, unsigned length);

void* theta_union_new_default();
void* theta_union_new(unsigned lg_k);
void theta_union_delete(void* unionptr);
void theta_union_update(void* unionptr, const void* sketchptr);
void* theta_union_get_result(const void* unionptr);

void* theta_intersection_new_default();
void theta_intersection_delete(void* interptr);
void theta_intersection_update(void* interptr, const void* sketchptr);
void* theta_intersection_get_result(const void* interptr);

void* theta_a_not_b(const void* sketchptr1, const void* sketchptr2);

#ifdef __cplusplus
}
#endif

#endif
