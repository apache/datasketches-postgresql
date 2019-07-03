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

#include "hll_sketch_c_adapter.h"
#include "base64.h"

const unsigned HLL_DEFAULT_LG_K = 12;

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_hll_sketch_add_item);
PG_FUNCTION_INFO_V1(pg_hll_sketch_get_estimate);
PG_FUNCTION_INFO_V1(pg_hll_sketch_get_estimate_and_bounds);
PG_FUNCTION_INFO_V1(pg_hll_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_hll_sketch_union_agg);
PG_FUNCTION_INFO_V1(pg_hll_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_hll_sketch_get_estimate_from_internal);
PG_FUNCTION_INFO_V1(pg_hll_union_get_result);
PG_FUNCTION_INFO_V1(pg_hll_sketch_union);

/* function declarations */
Datum pg_hll_sketch_recv(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_send(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_add_item(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_get_estimate(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_union_agg(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS);
Datum pg_hll_union_get_result(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_union(PG_FUNCTION_ARGS);

Datum pg_hll_sketch_add_item(PG_FUNCTION_ARGS) {
  void* sketchptr;
  unsigned lg_k;
  unsigned tgt_type;

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
    elog(ERROR, "hll_sketch_add_item called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    lg_k = PG_GETARG_INT32(2);
    if (lg_k == 0) lg_k = HLL_DEFAULT_LG_K;
    tgt_type = PG_GETARG_INT32(3);
    if (tgt_type) {
      if ((tgt_type != 4) && (tgt_type != 6) && (tgt_type != 8)) {
        elog(ERROR, "hll_sketch_add_item: unsupported target type, must be 4, 6 or 8");
      }
      sketchptr = hll_sketch_new_tgt_type(lg_k, tgt_type);
    } else {
      sketchptr = hll_sketch_new(lg_k);
    }
  } else {
    sketchptr = PG_GETARG_POINTER(0);
  }

  element_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
  element = PG_GETARG_DATUM(1);
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  if (typlen == -1) {
    // varlena
    hll_sketch_update(sketchptr, VARDATA_ANY(element), VARSIZE_ANY_EXHDR(element));
  } else if (typbyval) {
    // fixed-length passed by value
    hll_sketch_update(sketchptr, &element, typlen);
  } else {
    // fixed-length passed by reference
    hll_sketch_update(sketchptr, (void*)element, typlen);
  }

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(sketchptr);
}

Datum pg_hll_sketch_get_estimate(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  double estimate;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = hll_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  estimate = hll_sketch_get_estimate(sketchptr);
  hll_sketch_delete(sketchptr);
  PG_RETURN_FLOAT8(estimate);
}

Datum pg_hll_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  int num_std_devs;

  // output array
  Datum* est_and_bounds;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = hll_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  num_std_devs = PG_GETARG_INT32(1);
  if (num_std_devs == 0) num_std_devs = 1; // default
  est_and_bounds = (Datum*) hll_sketch_get_estimate_and_bounds(sketchptr, num_std_devs);
  hll_sketch_delete(sketchptr);

  // construct output array
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(est_and_bounds, 3, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);
  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_hll_sketch_to_string(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  char str[1024];
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = hll_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  hll_sketch_to_string(sketchptr, str, 1024);
  hll_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
}

struct hll_union_state {
  void* unionptr;
  unsigned tgt_type;
};

Datum pg_hll_sketch_union_agg(PG_FUNCTION_ARGS) {
  struct hll_union_state* stateptr;
  bytea* sketch_bytes;
  void* sketchptr;
  unsigned lg_k;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_union_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    lg_k = PG_GETARG_INT32(2);
    stateptr = palloc(sizeof(struct hll_union_state));
    stateptr->unionptr = hll_union_new(lg_k ? lg_k : HLL_DEFAULT_LG_K);
    stateptr->tgt_type = PG_GETARG_INT32(3);
    if (stateptr->tgt_type) {
      if ((stateptr->tgt_type != 4) && (stateptr->tgt_type != 6) && (stateptr->tgt_type != 8)) {
        elog(ERROR, "hll_sketch_union_agg: unsupported target type, must be 4, 6 or 8");
      }
    }
  } else {
    stateptr = (struct hll_union_state*) PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = hll_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  hll_union_update(stateptr->unionptr, sketchptr);
  hll_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_hll_sketch_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  sketchptr = PG_GETARG_POINTER(0);
  bytes_out = hll_sketch_serialize(sketchptr, VARHDRSZ);
  hll_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_hll_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  double estimate;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  sketchptr = PG_GETARG_POINTER(0);
  estimate = hll_sketch_get_estimate(sketchptr);
  hll_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_FLOAT8(estimate);
}

Datum pg_hll_union_get_result(PG_FUNCTION_ARGS) {
  struct hll_union_state* stateptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_union_get_result called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct hll_union_state*) PG_GETARG_POINTER(0);
  if (stateptr->tgt_type) {
    sketchptr = hll_union_get_result_tgt_type(stateptr->unionptr, stateptr->tgt_type);
  } else {
    sketchptr = hll_union_get_result(stateptr->unionptr);
  }
  bytes_out = hll_sketch_serialize(sketchptr, VARHDRSZ);
  hll_sketch_delete(sketchptr);
  hll_union_delete(stateptr->unionptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_hll_sketch_union(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* unionptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;
  unsigned lg_k;
  unsigned tgt_type;

  lg_k = PG_GETARG_INT32(2);
  tgt_type = PG_GETARG_INT32(3);
  if (tgt_type) {
    if ((tgt_type != 4) && (tgt_type != 6) && (tgt_type != 8)) {
      elog(ERROR, "hll_sketch_union: unsupported target type, must be 4, 6 or 8");
    }
  }
  unionptr = hll_union_new(lg_k ? lg_k : HLL_DEFAULT_LG_K);
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    sketchptr1 = hll_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
    hll_union_update(unionptr, sketchptr1);
    hll_sketch_delete(sketchptr1);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    sketchptr2 = hll_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
    hll_union_update(unionptr, sketchptr2);
    hll_sketch_delete(sketchptr2);
  }
  if (tgt_type) {
    sketchptr = hll_union_get_result_tgt_type(unionptr, tgt_type);
  } else {
    sketchptr = hll_union_get_result(unionptr);
  }
  hll_union_delete(unionptr);
  bytes_out = hll_sketch_serialize(sketchptr, VARHDRSZ);
  hll_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}
