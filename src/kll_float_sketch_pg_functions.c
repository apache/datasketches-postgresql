/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include <postgres.h>
#include <fmgr.h>
#include <utils/lsyscache.h>
#include <utils/builtins.h>

#include "kll_float_sketch_c_adapter.h"
#include "base64.h"

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_recv);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_send);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_add_item_default);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_rank);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_get_quantile);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_merge_default);
PG_FUNCTION_INFO_V1(pg_kll_float_sketch_from_internal);

/* function declarations */
Datum pg_kll_float_sketch_recv(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_send(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_add_item_default(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_rank(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_get_quantile(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_merge_default(PG_FUNCTION_ARGS);
Datum pg_kll_float_sketch_from_internal(PG_FUNCTION_ARGS);

static const unsigned DEFAULT_K = 200;

// external binary to type
Datum pg_kll_float_sketch_recv(PG_FUNCTION_ARGS) {
  // not invoked for nulls
  elog(FATAL, "pg_kll_float_sketch_recv is not implemented yet");
  PG_RETURN_NULL();
}

// type to external binary
Datum pg_kll_float_sketch_send(PG_FUNCTION_ARGS) {
  // not invoked for nulls
  elog(FATAL, "pg_kll_float_sketch_send is not implemented yet");
  PG_RETURN_BYTEA_P(0);
}

Datum pg_kll_float_sketch_add_item_default(PG_FUNCTION_ARGS) {
  void* sketchptr;
  float value;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "kll_float_sketch_add_item_default called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    sketchptr = kll_float_sketch_new(DEFAULT_K);
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

Datum pg_kll_float_sketch_merge_default(PG_FUNCTION_ARGS) {
  void* unionptr;
  bytea* sketch_bytes;
  void* sketchptr;

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
    unionptr = kll_float_sketch_new(DEFAULT_K);
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
  unsigned length;
  bytea* bytes_out;
  MemoryContext aggcontext;
  if (PG_ARGISNULL(0)) PG_RETURN_NULL();
  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "kll_float_sketch_from_internal called in non-aggregate context");
  }
  sketchptr = PG_GETARG_POINTER(0);
  length = VARHDRSZ + kll_float_sketch_get_serialized_size_bytes(sketchptr);
  bytes_out = palloc(length);
  SET_VARSIZE(bytes_out, length);
  kll_float_sketch_serialize(sketchptr, VARDATA(bytes_out));
  kll_float_sketch_delete(sketchptr);
  PG_RETURN_BYTEA_P(bytes_out);
}
