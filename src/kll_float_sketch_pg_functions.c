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

#include <postgres.h>
#include <fmgr.h>
#include <utils/lsyscache.h>
#include <utils/builtins.h>
#include <utils/array.h>
#include <catalog/pg_type.h>

#include "kll_float_sketch_c_adapter.h"
#include "base64.h"

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_add_item);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_rank);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_quantile);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_n);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_merge);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_pmf);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_cdf);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_quantiles);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_histogram);

/* function declarations */
Datum pg_kll_float_sketch_recv(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_send(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_add_item(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_rank(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_quantile(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_n(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_merge(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_pmf(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_cdf(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_quantiles(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_histogram(PG_FUNCTION_ARGS);

static const unsigned DEFAULT_K = 200;
static const unsigned DEFAULT_NUM_BINS = 10;

Datum pg_kll_float_sketch_add_item(PG_FUNCTION_ARGS) {
  void* sketchptr;
  float value;
  int k;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "kll_float_sketch_add_item called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    k = PG_GETARG_INT32(2);
    sketchptr = kll_float_sketch_new(k ? k : DEFAULT_K);
  } else {
    sketchptr = PG_GETARG_POINTER(0);
  }

  value = PG_GETARG_FLOAT4(1);
  kll_float_sketch_update(sketchptr, value);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(sketchptr);
}

Datum pg_kll_float_sketch_get_rank(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  float value;
  double rank;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  value = PG_GETARG_FLOAT4(1);
  rank = kll_float_sketch_get_rank(sketchptr, value);
  kll_float_sketch_delete(sketchptr);
  PG_RETURN_FLOAT8(rank);
}

Datum pg_kll_float_sketch_get_quantile(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  float value;
  double rank;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  rank = PG_GETARG_FLOAT8(1);
  value = kll_float_sketch_get_quantile(sketchptr, rank);
  kll_float_sketch_delete(sketchptr);
  PG_RETURN_FLOAT4(value);
}

Datum pg_kll_float_sketch_get_n(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  uint64 n;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  n = kll_float_sketch_get_n(sketchptr);
  kll_float_sketch_delete(sketchptr);
  PG_RETURN_INT64(n);
}

Datum pg_kll_float_sketch_to_string(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  char str[1024];
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  kll_float_sketch_to_string(sketchptr, str, 1024);
  kll_float_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
}

Datum pg_kll_float_sketch_merge(PG_FUNCTION_ARGS) {
  void* unionptr;
  bytea* sketch_bytes;
  void* sketchptr;
  int k;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "kll_float_sketch_merge called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    k = PG_GETARG_INT32(2);
    unionptr = kll_float_sketch_new(k ? k : DEFAULT_K);
  } else {
    unionptr = PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = kll_float_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  kll_float_sketch_merge(unionptr, sketchptr);
  kll_float_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(unionptr);
}

Datum pg_kll_float_sketch_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  struct ptr_with_size bytes_out;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();
  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "kll_float_sketch_from_internal called in non-aggregate context");
  }
  sketchptr = PG_GETARG_POINTER(0);
  bytes_out = kll_float_sketch_serialize(sketchptr, VARHDRSZ);
  kll_float_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_kll_float_sketch_get_pmf(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;

  // input array of split points
  ArrayType* arr_in;
  Oid elmtype_in;
  int16 elmlen_in;
  bool elmbyval_in;
  char elmalign_in;
  Datum* data_in;
  bool* nulls_in;
  int arr_len_in;
  float* split_points;

  // output array of fractions
  Datum* result;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;
  int arr_len_out;

  int i;

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);

  arr_in = PG_GETARG_ARRAYTYPE_P(1);
  elmtype_in = ARR_ELEMTYPE(arr_in);
  get_typlenbyvalalign(elmtype_in, &elmlen_in, &elmbyval_in, &elmalign_in);
  deconstruct_array(arr_in, elmtype_in, elmlen_in, elmbyval_in, elmalign_in, &data_in, &nulls_in, &arr_len_in);

  split_points = palloc(sizeof(float) * arr_len_in);
  for (i = 0; i < arr_len_in; i++) {
    split_points[i] = DatumGetFloat4(data_in[i]);
  }
  result = (Datum*) kll_float_sketch_get_pmf_or_cdf(sketchptr, split_points, arr_len_in, false, false);
  pfree(split_points);

  // construct output array of fractions
  arr_len_out = arr_len_in + 1; // N split points devide the number line into N+1 intervals
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(result, arr_len_out, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);

  kll_float_sketch_delete(sketchptr);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_kll_float_sketch_get_cdf(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;

  // input array of split points
  ArrayType* arr_in;
  Oid elmtype_in;
  int16 elmlen_in;
  bool elmbyval_in;
  char elmalign_in;
  Datum* data_in;
  bool* nulls_in;
  int arr_len_in;
  float* split_points;

  // output array of fractions
  Datum* result;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;
  int arr_len_out;

  int i;

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);

  arr_in = PG_GETARG_ARRAYTYPE_P(1);
  elmtype_in = ARR_ELEMTYPE(arr_in);
  get_typlenbyvalalign(elmtype_in, &elmlen_in, &elmbyval_in, &elmalign_in);
  deconstruct_array(arr_in, elmtype_in, elmlen_in, elmbyval_in, elmalign_in, &data_in, &nulls_in, &arr_len_in);

  split_points = palloc(sizeof(float) * arr_len_in);
  for (i = 0; i < arr_len_in; i++) {
    split_points[i] = DatumGetFloat4(data_in[i]);
  }
  result = (Datum*) kll_float_sketch_get_pmf_or_cdf(sketchptr, split_points, arr_len_in, true, false);
  pfree(split_points);

  // construct output array of fractions
  arr_len_out = arr_len_in + 1; // N split points devide the number line into N+1 intervals
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(result, arr_len_out, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);

  kll_float_sketch_delete(sketchptr);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_kll_float_sketch_get_quantiles(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;

  // input array of fractions
  ArrayType* arr_in;
  Oid elmtype_in;
  int16 elmlen_in;
  bool elmbyval_in;
  char elmalign_in;
  Datum* data_in;
  bool* nulls_in;
  int arr_len;
  double* fractions;

  // output array of quantiles
  Datum* quantiles;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;

  int i;

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);

  arr_in = PG_GETARG_ARRAYTYPE_P(1);
  elmtype_in = ARR_ELEMTYPE(arr_in);
  get_typlenbyvalalign(elmtype_in, &elmlen_in, &elmbyval_in, &elmalign_in);
  deconstruct_array(arr_in, elmtype_in, elmlen_in, elmbyval_in, elmalign_in, &data_in, &nulls_in, &arr_len);

  fractions = palloc(sizeof(double) * arr_len);
  for (i = 0; i < arr_len; i++) {
    fractions[i] = DatumGetFloat8(data_in[i]);
  }
  quantiles = (Datum*) kll_float_sketch_get_quantiles(sketchptr, fractions, arr_len);
  pfree(fractions);

  // construct output array of quantiles
  get_typlenbyvalalign(FLOAT4OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(quantiles, arr_len, FLOAT4OID, elmlen_out, elmbyval_out, elmalign_out);

  kll_float_sketch_delete(sketchptr);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_kll_float_sketch_get_histogram(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  int num_bins;

  // output array of bins
  Datum* result;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;
  int arr_len_out;

  int i;

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = kll_float_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);

  num_bins = PG_GETARG_INT32(1);
  if (num_bins == 0) num_bins = DEFAULT_NUM_BINS;
  if (num_bins < 2) {
    elog(ERROR, "at least two bins expected");
  }

  float* split_points = palloc(sizeof(float) * (num_bins - 1));
  const float min_value = kll_float_sketch_get_quantile(sketchptr, 0);
  const float max_value = kll_float_sketch_get_quantile(sketchptr, 1);
  const float delta = (max_value - min_value) / num_bins;
  for (i = 0; i < num_bins - 1; i++) {
    split_points[i] = min_value + delta * (i + 1);
  }
  result = (Datum*) kll_float_sketch_get_pmf_or_cdf(sketchptr, split_points, num_bins - 1, false, true);
  pfree(split_points);

  // construct output array
  arr_len_out = num_bins;
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(result, arr_len_out, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);

  kll_float_sketch_delete(sketchptr);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}
