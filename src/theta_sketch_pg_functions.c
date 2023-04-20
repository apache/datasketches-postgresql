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
#include "agg_state.h"

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_theta_sketch_build_agg);
PG_FUNCTION_INFO_V1(pg_theta_sketch_union_agg);
PG_FUNCTION_INFO_V1(pg_theta_sketch_intersection_agg);
PG_FUNCTION_INFO_V1(pg_theta_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_theta_sketch_get_estimate_from_internal);
PG_FUNCTION_INFO_V1(pg_theta_sketch_union_combine);
PG_FUNCTION_INFO_V1(pg_theta_sketch_intersection_combine);
PG_FUNCTION_INFO_V1(pg_theta_sketch_serialize_state);
PG_FUNCTION_INFO_V1(pg_theta_sketch_deserialize_state);
PG_FUNCTION_INFO_V1(pg_theta_sketch_get_estimate);
PG_FUNCTION_INFO_V1(pg_theta_sketch_get_estimate_and_bounds);
PG_FUNCTION_INFO_V1(pg_theta_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_theta_sketch_union);
PG_FUNCTION_INFO_V1(pg_theta_sketch_intersection);
PG_FUNCTION_INFO_V1(pg_theta_sketch_a_not_b);

/* function declarations */
Datum pg_theta_sketch_build_agg(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_union_agg(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_intersection_agg(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_union_combine(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_intersection_combine(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_serialize_state(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_deserialize_state(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_get_estimate(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_union(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_intersection(PG_FUNCTION_ARGS);
Datum pg_theta_sketch_a_not_b(PG_FUNCTION_ARGS);

Datum pg_theta_sketch_build_agg(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr;
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
    elog(ERROR, "theta_sketch_build_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct agg_state));
    stateptr->type = MUTABLE_SKETCH;
    stateptr->lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 0;
    p = PG_NARGS() > 3 ? PG_GETARG_FLOAT4(3) : 1;
    if (stateptr->lg_k) {
      stateptr->ptr = p ? theta_sketch_new_lgk_p(stateptr->lg_k, p) : theta_sketch_new_lgk(stateptr->lg_k);
    } else {
      stateptr->ptr = theta_sketch_new_default();
    }
  } else {
    stateptr = (struct agg_state*) PG_GETARG_POINTER(0);
  }

  element_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
  element = PG_GETARG_DATUM(1);
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  if (typlen == -1) {
    // varlena
    theta_sketch_update(stateptr->ptr, VARDATA_ANY(element), VARSIZE_ANY_EXHDR(element));
  } else if (typbyval) {
    // fixed-length passed by value
    theta_sketch_update(stateptr->ptr, &element, typlen);
  } else {
    // fixed-length passed by reference
    theta_sketch_update(stateptr->ptr, (void*)element, typlen);
  }

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_theta_sketch_union_agg(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr;
  bytea* sketch_bytes;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_union_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct agg_state));
    stateptr->type = UNION;
    stateptr->lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 0;
    stateptr->ptr = stateptr->lg_k ? theta_union_new(stateptr->lg_k) : theta_union_new_default();
  } else {
    stateptr = (struct agg_state*) PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  theta_union_update_with_bytes(stateptr->ptr, VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_theta_sketch_intersection_agg(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr;
  bytea* sketch_bytes;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_intersection_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct agg_state));
    stateptr->type = INTERSECTION;
    stateptr->ptr = theta_intersection_new_default();
  } else {
    stateptr = (struct agg_state*) PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  theta_intersection_update_with_bytes(stateptr->ptr, VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr->ptr);
}

Datum pg_theta_sketch_from_internal(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct agg_state*) PG_GETARG_POINTER(0);
  if (stateptr->type == MUTABLE_SKETCH) {
    stateptr->ptr = theta_sketch_compact(stateptr->ptr);
  } else if (stateptr->type == UNION) {
    stateptr->ptr = theta_union_get_result(stateptr->ptr);
  } else if (stateptr->type == INTERSECTION) {
    stateptr->ptr = theta_intersection_get_result(stateptr->ptr);
  }
  bytes_out = theta_sketch_serialize(stateptr->ptr, VARHDRSZ);
  theta_sketch_delete(stateptr->ptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr;
  double estimate;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_get_estimate_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct agg_state*) PG_GETARG_POINTER(0);
  estimate = theta_sketch_get_estimate(stateptr->ptr);
  theta_sketch_delete(stateptr->ptr);
  pfree(stateptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_FLOAT8(estimate);
}

Datum pg_theta_sketch_union_combine(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr1;
  struct agg_state* stateptr2;
  struct agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_combine called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr1 = (struct agg_state*) PG_GETARG_POINTER(0);
  stateptr2 = (struct agg_state*) PG_GETARG_POINTER(1);

  stateptr = palloc(sizeof(struct agg_state));
  stateptr->type = IMMUTABLE_SKETCH;
  stateptr->lg_k = stateptr1 ? stateptr1->lg_k : stateptr2->lg_k;
  stateptr->ptr = stateptr->lg_k ? theta_union_new(stateptr->lg_k) : theta_union_new_default();
  if (stateptr1) {
    if (stateptr1->type == UNION) {
      stateptr1->ptr = theta_union_get_result(stateptr1->ptr);
    }
    theta_union_update_with_sketch(stateptr->ptr, stateptr1->ptr);
    theta_sketch_delete(stateptr1->ptr);
    pfree(stateptr1);
  }
  if (stateptr2) {
    if (stateptr2->type == UNION) {
      stateptr2->ptr = theta_union_get_result(stateptr2->ptr);
    }
    theta_union_update_with_sketch(stateptr->ptr, stateptr2->ptr);
    theta_sketch_delete(stateptr2->ptr);
    pfree(stateptr2);
  }
  stateptr->ptr = theta_union_get_result(stateptr->ptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_theta_sketch_intersection_combine(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr1;
  struct agg_state* stateptr2;
  struct agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_combine called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr1 = (struct agg_state*) PG_GETARG_POINTER(0);
  stateptr2 = (struct agg_state*) PG_GETARG_POINTER(1);

  stateptr = palloc(sizeof(struct agg_state));
  stateptr->type = IMMUTABLE_SKETCH;
  stateptr->ptr = theta_intersection_new_default();
  if (stateptr1) {
    if (stateptr1->type == INTERSECTION) {
      stateptr1->ptr = theta_intersection_get_result(stateptr1->ptr);
    }
    theta_intersection_update_with_sketch(stateptr->ptr, stateptr1->ptr);
    theta_sketch_delete(stateptr1->ptr);
    pfree(stateptr1);
  }
  if (stateptr2) {
    if (stateptr2->type == INTERSECTION) {
      stateptr2->ptr = theta_intersection_get_result(stateptr2->ptr);
    }
    theta_intersection_update_with_sketch(stateptr->ptr, stateptr2->ptr);
    theta_sketch_delete(stateptr2->ptr);
    pfree(stateptr2);
  }
  stateptr->ptr = theta_intersection_get_result(stateptr->ptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_theta_sketch_serialize_state(PG_FUNCTION_ARGS) {
  struct agg_state* stateptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_serialize_state called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct agg_state*) PG_GETARG_POINTER(0);
  if (stateptr->type == MUTABLE_SKETCH) {
    stateptr->ptr = theta_sketch_compact(stateptr->ptr);
  } else if (stateptr->type == UNION) {
    stateptr->ptr = theta_union_get_result(stateptr->ptr);
  } else if (stateptr->type == INTERSECTION) {
    stateptr->ptr = theta_intersection_get_result(stateptr->ptr);
  }
  bytes_out = theta_sketch_serialize(stateptr->ptr, VARHDRSZ + 1);
  ((char*)bytes_out.ptr)[VARHDRSZ] = stateptr->lg_k;
  theta_sketch_delete(stateptr->ptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_deserialize_state(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  struct agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "theta_sketch_deserialize_state called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  bytes_in = PG_GETARG_BYTEA_P(0);
  stateptr = palloc(sizeof(struct agg_state));
  stateptr->type = IMMUTABLE_SKETCH;
  stateptr->lg_k = *VARDATA(bytes_in);
  stateptr->ptr = theta_sketch_deserialize(VARDATA(bytes_in) + 1, VARSIZE(bytes_in) - VARHDRSZ - 1);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
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
  num_std_devs = PG_NARGS() > 1 ? PG_GETARG_INT32(1) : 1;
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

Datum pg_theta_sketch_union(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* unionptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;
  unsigned lg_k;

  lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 0;
  unionptr = lg_k ? theta_union_new(lg_k) : theta_union_new_default();
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    theta_union_update_with_bytes(unionptr, VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    theta_union_update_with_bytes(unionptr, VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
  }
  sketchptr = theta_union_get_result(unionptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_intersection(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* interptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  interptr = theta_intersection_new_default();
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    theta_intersection_update_with_bytes(interptr, VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    theta_intersection_update_with_bytes(interptr, VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
  }
  sketchptr = theta_intersection_get_result(interptr);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_theta_sketch_a_not_b(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
    elog(ERROR, "theta_a_not_b expects two valid theta sketches");
  }

  bytes_in1 = PG_GETARG_BYTEA_P(0);
  bytes_in2 = PG_GETARG_BYTEA_P(1);
  sketchptr = theta_a_not_b(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ, VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
  bytes_out = theta_sketch_serialize(sketchptr, VARHDRSZ);
  theta_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}
