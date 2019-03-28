/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef FREQUENT_STRINGS_SKETCH_C_ADAPTER_H
#define FREQUENT_STRINGS_SKETCH_C_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <postgres.h>

void* frequent_strings_sketch_new(unsigned lg_k);
void frequent_strings_sketch_delete(void* sketchptr);

void frequent_strings_sketch_update(void* sketchptr, const char* str, unsigned length, unsigned long long weight);
void frequent_strings_sketch_merge(void* sketchptr1, const void* sketchptr2);
char* frequent_strings_sketch_to_string(const void* sketchptr, bool print_items);

void* frequent_strings_sketch_serialize(const void* sketchptr);
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
