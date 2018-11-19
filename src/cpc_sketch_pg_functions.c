/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include <postgres.h>
#include <fmgr.h>
#include <utils/lsyscache.h>
#include <utils/builtins.h>

#include "cpc_sketch_c_adapter.h"
#include "base64.h"

const unsigned DEFAULT_LG_K = 12;

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_cpc_sketch_recv);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_send);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_add_item_default);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_get_estimate);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_merge_default);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_cpc_sketch_get_estimate_from_internal);
PG_FUNCTION_INFO_V1(pg_cpc_union_get_result);

/* function declarations */
Datum pg_cpc_sketch_recv(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_send(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_add_item_default(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_get_estimate(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_merge_default(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_cpc_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS);
Datum pg_cpc_union_get_result(PG_FUNCTION_ARGS);

// external binary to type
Datum pg_cpc_sketch_recv(PG_FUNCTION_ARGS) {
  // not invoked for nulls
  elog(FATAL, "pg_cpc_sketch_recv is not implemented yet");
  PG_RETURN_NULL();
}

// type to external binary
Datum pg_cpc_sketch_send(PG_FUNCTION_ARGS) {
  // not invoked for nulls
  elog(FATAL, "pg_cpc_sketch_send is not implemented yet");
  PG_RETURN_BYTEA_P(0);
}

Datum pg_cpc_sketch_add_item_default(PG_FUNCTION_ARGS) {
  void* sketchptr;

  // anyelement
  Oid   element_type;
  Datum element;
  int16 typlen;
  bool  typbyval;
  char  typalign;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "cpc_sketch_add_item_default called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    sketchptr = cpc_sketch_new(DEFAULT_LG_K);
  } else {
    sketchptr = PG_GETARG_POINTER(0);
  }

  element_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
  element = PG_GETARG_DATUM(1);
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  if (typlen == -1) {
    // varlena
    cpc_sketch_update(sketchptr, VARDATA_ANY(element), VARSIZE_ANY_EXHDR(element));
  } else if (typbyval) {
    // fixed-length passed by value
    cpc_sketch_update(sketchptr, &element, typlen);
  } else {
    // fixed-length passed by reference
    cpc_sketch_update(sketchptr, (void*)element, typlen);
  }

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(sketchptr);
}

Datum pg_cpc_sketch_get_estimate(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  double estimate;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = cpc_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  estimate = cpc_sketch_get_estimate(sketchptr);
  cpc_sketch_delete(sketchptr);
  PG_RETURN_FLOAT8(estimate);
}

Datum pg_cpc_sketch_to_string(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  char str[1024];
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = cpc_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  cpc_sketch_to_string(sketchptr, str, 1024);
  cpc_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
}

Datum pg_cpc_sketch_merge_default(PG_FUNCTION_ARGS) {
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
    elog(ERROR, "cpc_sketch_merge_default called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    //elog(LOG, "pg_cpc_sketch_merge_defalut: initializing union state");
    unionptr = cpc_union_new(DEFAULT_LG_K);
  } else {
    //elog(LOG, "pg_cpc_sketch_merge_defalut: loading existing union state");
    unionptr = PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = cpc_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  cpc_union_update(unionptr, sketchptr);
  cpc_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(unionptr);
}

Datum pg_cpc_sketch_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  bytea* bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "cpc_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  sketchptr = PG_GETARG_POINTER(0);
  bytes_out = cpc_sketch_serialize(sketchptr);
  cpc_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out);
}

Datum pg_cpc_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  double estimate;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "cpc_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  sketchptr = PG_GETARG_POINTER(0);
  estimate = cpc_sketch_get_estimate(sketchptr);
  cpc_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_FLOAT8(estimate);
}

Datum pg_cpc_union_get_result(PG_FUNCTION_ARGS) {
  void* unionptr;
  void* sketchptr;
  bytea* bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "cpc_union_get_result called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  unionptr = PG_GETARG_POINTER(0);
  sketchptr = cpc_union_get_result(unionptr);
  bytes_out = cpc_sketch_serialize(sketchptr);
  cpc_sketch_delete(sketchptr);
  cpc_union_delete(unionptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out);
}
