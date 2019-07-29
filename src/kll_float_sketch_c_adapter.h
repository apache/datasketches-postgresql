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
unsigned long long kll_float_sketch_get_n(const void* sketchptr);
void kll_float_sketch_to_string(const void* sketchptr, char* buffer, unsigned length);

struct ptr_with_size {
  void* ptr;
  unsigned long long size;
};

struct ptr_with_size kll_float_sketch_serialize(const void* sketchptr, unsigned header_size);
void* kll_float_sketch_deserialize(const char* buffer, unsigned length);
unsigned kll_float_sketch_get_serialized_size_bytes(const void* sketchptr);

void** kll_float_sketch_get_pmf_or_cdf(const void* sketchptr, const float* split_points, unsigned num_split_points, bool is_cdf);
void** kll_float_sketch_get_quantiles(const void* sketchptr, const double* fractions, unsigned num_fractions);

#ifdef __cplusplus
}
#endif

#endif
