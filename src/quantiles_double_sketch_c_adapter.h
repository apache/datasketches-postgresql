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

#ifndef QUANTILES_DOUBLE_SKETCH_C_ADAPTER_H
#define QUANTILES_DOUBLE_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptr_with_size.h"

static const unsigned DEFAULT_K = 128;

void* quantiles_double_sketch_new(unsigned k);
void quantiles_double_sketch_delete(void* sketchptr);

void quantiles_double_sketch_update(void* sketchptr, double value);
void quantiles_double_sketch_merge(void* sketchptr1, const void* sketchptr2);
double quantiles_double_sketch_get_rank(const void* sketchptr, double value);
double quantiles_double_sketch_get_quantile(const void* sketchptr, double rank);
unsigned long long quantiles_double_sketch_get_n(const void* sketchptr);
char* quantiles_double_sketch_to_string(const void* sketchptr);

struct ptr_with_size quantiles_double_sketch_serialize(const void* sketchptr, unsigned header_size);
void* quantiles_double_sketch_deserialize(const char* buffer, unsigned length);
unsigned quantiles_double_sketch_get_serialized_size_bytes(const void* sketchptr);

void** quantiles_double_sketch_get_pmf_or_cdf(const void* sketchptr, const double* split_points, unsigned num_split_points, bool is_cdf, bool scale);
void** quantiles_double_sketch_get_quantiles(const void* sketchptr, const double* fractions, unsigned num_fractions);

#ifdef __cplusplus
}
#endif

#endif
