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
#include <funcapi.h>

#include "frequent_strings_sketch_c_adapter.h"
#include "base64.h"

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_frequent_strings_sketch_add_item);
PG_FUNCTION_INFO_V1(pg_frequent_strings_sketch_merge);
PG_FUNCTION_INFO_V1(pg_frequent_strings_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_frequent_strings_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_frequent_strings_sketch_result_no_false_positives);
PG_FUNCTION_INFO_V1(pg_frequent_strings_sketch_result_no_false_negatives);

/* function declarations */
Datum pg_frequent_strings_sketch_recv(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_send(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_add_item(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_merge(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_result_no_false_positives(PG_FUNCTION_ARGS);
Datum pg_frequent_strings_sketch_result_no_false_negatives(PG_FUNCTION_ARGS);

Datum frequent_strings_sketch_get_result(PG_FUNCTION_ARGS, bool);


Datum pg_frequent_strings_sketch_add_item(PG_FUNCTION_ARGS) {
  void* sketchptr;
  unsigned lg_k;
  const VarChar* str;
  unsigned long long weight;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(2)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(2)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "frequent_strings_sketch_add_item called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    lg_k = PG_GETARG_INT32(1);
    sketchptr = frequent_strings_sketch_new(lg_k);
  } else {
    sketchptr = PG_GETARG_POINTER(0);
  }

  str = PG_GETARG_VARCHAR_P(2);

  // optional weight
  weight = PG_NARGS() > 3 ? PG_GETARG_INT64(3) : 1;

  frequent_strings_sketch_update(sketchptr, VARDATA(str), VARSIZE(str) - VARHDRSZ, weight);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(sketchptr);
}

Datum pg_frequent_strings_sketch_merge(PG_FUNCTION_ARGS) {
  void* unionptr;
  bytea* sketch_bytes;
  void* sketchptr;
  unsigned lg_k;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(2)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(2)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "frequent_strings_sketch_merge called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    lg_k = PG_GETARG_INT32(1);
    unionptr = frequent_strings_sketch_new(lg_k);
  } else {
    unionptr = PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(2);
  sketchptr = frequent_strings_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  frequent_strings_sketch_merge(unionptr, sketchptr);
  frequent_strings_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(unionptr);
}

Datum pg_frequent_strings_sketch_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  struct ptr_with_size bytes_out;
  MemoryContext aggcontext;
  if (PG_ARGISNULL(0)) PG_RETURN_NULL();
  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "frequent_strings_sketch_from_internal called in non-aggregate context");
  }
  sketchptr = PG_GETARG_POINTER(0);
  bytes_out = frequent_strings_sketch_serialize(sketchptr, VARHDRSZ);
  frequent_strings_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_frequent_strings_sketch_to_string(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  bool print_items;
  char* str;
  bytes_in = PG_GETARG_BYTEA_P(0);
  print_items = PG_NARGS() > 1 ? PG_GETARG_BOOL(1) : false;
  sketchptr = frequent_strings_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  str = frequent_strings_sketch_to_string(sketchptr, print_items);
  frequent_strings_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
}

Datum frequent_strings_sketch_get_result(PG_FUNCTION_ARGS, bool no_false_positives) {
  FuncCallContext* funcctx;
  TupleDesc tupdesc;
  const bytea* bytes_in;
  void* sketchptr;
  unsigned long long threshold;

  if (SRF_IS_FIRSTCALL()) {
    MemoryContext oldcontext;
    funcctx = SRF_FIRSTCALL_INIT();
    oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE) {
      ereport(
        ERROR,
        (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("function returning record called in context that cannot accept type record"))
      );
    }

    if (PG_ARGISNULL(0)) SRF_RETURN_DONE(funcctx);
    bytes_in = PG_GETARG_BYTEA_P(0);
    sketchptr = frequent_strings_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
    threshold = PG_NARGS() > 1 ? PG_GETARG_INT64(1) : 0;

    funcctx->user_fctx = frequent_strings_sketch_get_frequent_items(sketchptr, no_false_positives, threshold);
    funcctx->max_calls = ((struct frequent_strings_sketch_result*) funcctx->user_fctx)->num;
    funcctx->attinmeta = TupleDescGetAttInMetadata(tupdesc);

    frequent_strings_sketch_delete(sketchptr);

    MemoryContextSwitchTo(oldcontext);
  }

  funcctx = SRF_PERCALL_SETUP();

  if (funcctx->call_cntr < funcctx->max_calls) {
    char       **values;
    HeapTuple    tuple;
    Datum        result;

    struct frequent_strings_sketch_result* frequent_strings = funcctx->user_fctx;
    struct frequent_strings_sketch_result_row* row = &frequent_strings->rows[funcctx->call_cntr];

    values = (char**) palloc(4 * sizeof(char*));
    values[0] = row->str;
    values[1] = (char*) palloc(21);
    values[2] = (char*) palloc(21);
    values[3] = (char*) palloc(21);

    snprintf(values[1], 21, "%llu", row->estimate);
    snprintf(values[2], 21, "%llu", row->lower_bound);
    snprintf(values[3], 21, "%llu", row->upper_bound);

    tuple = BuildTupleFromCStrings(funcctx->attinmeta, values);

    result = HeapTupleGetDatum(tuple);

    pfree(values[1]);
    pfree(values[2]);
    pfree(values[3]);
    pfree(values);

    SRF_RETURN_NEXT(funcctx, result);
  } else {
    SRF_RETURN_DONE(funcctx);
  }
}

Datum pg_frequent_strings_sketch_result_no_false_positives(PG_FUNCTION_ARGS) {
  return frequent_strings_sketch_get_result(fcinfo, true);
}

Datum pg_frequent_strings_sketch_result_no_false_negatives(PG_FUNCTION_ARGS) {
  return frequent_strings_sketch_get_result(fcinfo, false);
}
