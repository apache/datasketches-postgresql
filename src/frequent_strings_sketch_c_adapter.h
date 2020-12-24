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

#ifndef FREQUENT_STRINGS_SKETCH_C_ADAPTER_H
#define FREQUENT_STRINGS_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptr_with_size.h"

void* frequent_strings_sketch_new(unsigned lg_k);
void frequent_strings_sketch_delete(void* sketchptr);

void frequent_strings_sketch_update(void* sketchptr, const char* str, unsigned length, unsigned long long weight);
void frequent_strings_sketch_merge(void* sketchptr1, const void* sketchptr2);
char* frequent_strings_sketch_to_string(const void* sketchptr, bool print_items);

struct ptr_with_size frequent_strings_sketch_serialize(const void* sketchptr, unsigned header_size);
void* frequent_strings_sketch_deserialize(const char* buffer, unsigned length);
unsigned frequent_strings_sketch_get_serialized_size_bytes(const void* sketchptr);

struct frequent_strings_sketch_result_row {
  char* str;
  unsigned long long estimate;
  unsigned long long lower_bound;
  unsigned long long upper_bound;
};

struct frequent_strings_sketch_result {
  struct frequent_strings_sketch_result_row* rows;
  unsigned num;
};

struct frequent_strings_sketch_result* frequent_strings_sketch_get_frequent_items(void* sketchptr, bool no_false_positives, unsigned long long threshold);

#ifdef __cplusplus
}
#endif

#endif
