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

#include "theta_sketch_c_adapter.h"
#include "base64.h"

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_theta_sketch_add_item);
PG_FUNCTION_INFO_V1(pg_theta_sketch_get_estimate);
PG_FUNCTION_INFO_V1(pg_theta_sketch_get_estimate_and_bounds);
PG_FUNCTION_INFO_V1(pg_theta_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_theta_sketch_union_agg);
PG_FUNCTION_INFO_V1(pg_theta_sketch_intersection_agg);
PG_FUNCTION_INFO_V1(pg_theta_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_theta_sketch_get_estimate_from_internal);
PG_FUNCTION_INFO_V1(pg_theta_union_get_result);
PG_FUNCTION_INFO_V1(pg_theta_intersection_get_result);
PG_FUNCTION_INFO_V1(pg_theta_sketch_union);
PG_FUNCTION_INFO_V1(pg_theta_sketch_intersection);
PG_FUNCTION_INFO_V1(pg_theta_sketch_a_not_b);

/* function declarations */
Datum pg_theta_sketch_recv(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_send(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_add_item(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_get_estimate(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_union_agg(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_intersection_agg(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS);
Datum pg_theta_union_get_result(PG_FUNCTION_ARGS);
Datum pg_theta_intersection_get_result(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_union(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_intersection(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_a_not_b(PG_FUNCTION_ARGS);

Datum pg_theta_sketch_add_item(PG_FUNCTION_ARGS) {
  void* sketchptr;
  int lg_k;
  float p;

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
    elog(ERROR, "theta_sketch_add_item called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 0;
    p = PG_NARGS() > 3 ? PG_GETARG_FLOAT4(3) : 1;
    if (lg_k) {
      sketchptr = p ? theta_sketch_new_lgk_p(lg_k, p) : theta_sketch_new_lgk(lg_k);
    } else {
      sketchptr = theta_sketch_new_default();
    }
  } else {
    sketchptr = PG_GETARG_POINTER(0);
  }

  element_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
  element = PG_GETARG_DATUM(1);
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  if (typlen == -1) {
    // varlena
    theta_sketch_update(sketchptr, VARDATA_ANY(element), VARSIZE_ANY_EXHDR(element));
  } else if (typbyval) {
    // fixed-length passed by value
    theta_sketch_update(sketchptr, &element, typlen);
  } else {
    // fixed-length passed by reference
    theta_sketch_update(sketchptr, (void*)element, typlen);
  }

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(sketchptr);
}

Datum pg_theta_sketch_get_estimate(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  double estimate;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = theta_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  estimate = theta_sketch_get_estimate(sketchptr);
  theta_sketch_delete(sketchptr);
  PG_RETURN_FLOAT8(estimate);
}

Datum pg_theta_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS) {
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
  sketchptr = theta_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  num_std_devs = PG_GETARG_INT32(1);
  if (num_std_devs == 0) num_std_devs = 1; // default
  est_and_bounds = (Datum*) theta_sketch_get_estimate_and_bounds(sketchptr, num_std_devs);
  theta_sketch_delete(sketchptr);

  // construct output array
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(est_and_bounds, 3, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);
  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_theta_sketch_to_string(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  char* str;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = theta_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  str = theta_sketch_to_string(sketchptr);
  theta_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
}

Datum pg_theta_sketch_intersection_agg(PG_FUNCTION_ARGS) {
  void* interptr;
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
    elog(ERROR, "theta_sketch_intersect called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    interptr = theta_intersection_new_default();
  } else {
    interptr = PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = theta_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  theta_intersection_update(interptr, sketchptr);
  theta_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(interptr);
}

Datum pg_theta_sketch_union_agg(PG_FUNCTION_ARGS) {
  void* unionptr;
  bytea* sketch_bytes;
  void* sketchptr;
  int lg_k;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_merge called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 0;
    unionptr = lg_k ? theta_union_new(lg_k) : theta_union_new_default();
  } else {
    unionptr = PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = theta_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  theta_union_update(unionptr, sketchptr);
  theta_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(unionptr);
}

Datum pg_theta_sketch_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  sketchptr = PG_GETARG_POINTER(0);
  sketchptr = theta_sketch_compact(sketchptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS) {
  void* sketchptr;
  double estimate;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  sketchptr = PG_GETARG_POINTER(0);
  estimate = theta_sketch_get_estimate(sketchptr);
  theta_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_FLOAT8(estimate);
}

Datum pg_theta_union_get_result(PG_FUNCTION_ARGS) {
  void* unionptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_union_get_result called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  unionptr = PG_GETARG_POINTER(0);
  sketchptr = theta_union_get_result(unionptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  theta_union_delete(unionptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_intersection_get_result(PG_FUNCTION_ARGS) {
  void* interptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_intersection_get_result called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  interptr = PG_GETARG_POINTER(0);
  sketchptr = theta_intersection_get_result(interptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  theta_intersection_delete(interptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_union(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* unionptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;
  int lg_k;
  
  lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 0;
  unionptr = lg_k ? theta_union_new(lg_k) : theta_union_new_default();
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    sketchptr1 = theta_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
    theta_union_update(unionptr, sketchptr1);
    theta_sketch_delete(sketchptr1);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    sketchptr2 = theta_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
    theta_union_update(unionptr, sketchptr2);
    theta_sketch_delete(sketchptr2);
  }
  sketchptr = theta_union_get_result(unionptr);
  theta_union_delete(unionptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_intersection(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* interptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;
  
  interptr = theta_intersection_new_default();
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    sketchptr1 = theta_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
    theta_intersection_update(interptr, sketchptr1);
    theta_sketch_delete(sketchptr1);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    sketchptr2 = theta_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
    theta_intersection_update(interptr, sketchptr2);
    theta_sketch_delete(sketchptr2);
  }
  sketchptr = theta_intersection_get_result(interptr);
  theta_intersection_delete(interptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_a_not_b(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
    elog(ERROR, "theta_a_not_b expects two valid theta sketches");
  }

  bytes_in1 = PG_GETARG_BYTEA_P(0);
  sketchptr1 = theta_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
  bytes_in2 = PG_GETARG_BYTEA_P(1);
  sketchptr2 = theta_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
  sketchptr = theta_a_not_b(sketchptr1, sketchptr2);
  theta_sketch_delete(sketchptr1);
  theta_sketch_delete(sketchptr2);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}
