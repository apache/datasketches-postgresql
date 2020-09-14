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

#ifndef AOD_SKETCH_C_ADAPTER_H
#define AOD_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptr_with_size.h"

void* aod_sketch_new(unsigned num_values);
void* aod_sketch_new_lgk(unsigned num_values, unsigned lg_k);
void* aod_sketch_new_lgk_p(unsigned num_values, unsigned lg_k, float p);
void update_aod_sketch_delete(void* sketchptr);
void compact_aod_sketch_delete(void* sketchptr);

void aod_sketch_update(void* sketchptr, const void* data, unsigned length, const double* values);
void* aod_sketch_compact(void* sketchptr);
void aod_sketch_union(void* sketchptr1, const void* sketchptr2);
double update_aod_sketch_get_estimate(const void* sketchptr);
double compact_aod_sketch_get_estimate(const void* sketchptr);
void** aod_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs);
char* aod_sketch_to_string(const void* sketchptr, bool print_entries);

struct ptr_with_size aod_sketch_serialize(const void* sketchptr, unsigned header_size);
void* aod_sketch_deserialize(const char* buffer, unsigned length);

void* aod_union_new(unsigned num_values);
void* aod_union_new_lgk(unsigned num_values, unsigned lg_k);
void aod_union_delete(void* unionptr);
void aod_union_update(void* unionptr, const void* sketchptr);
void* aod_union_get_result(const void* unionptr);

void* aod_intersection_new(unsigned num_values);
void aod_intersection_delete(void* interptr);
void aod_intersection_update(void* interptr, const void* sketchptr);
void* aod_intersection_get_result(const void* interptr);

void* aod_a_not_b(const void* sketchptr1, const void* sketchptr2);

void* aod_sketch_to_kll_float_sketch(const void* sketchptr, unsigned column_index, unsigned k);

void** aod_sketch_students_t_test(const void* sketchptr1, const void* sketchptr2, unsigned* arr_len_out);
void** aod_sketch_to_means(const void* sketchptr, unsigned* arr_len_out);
void** aod_sketch_to_variances(const void* sketchptr, unsigned* arr_len_out);

#ifdef __cplusplus
}
#endif

#endif
