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

#include "aod_sketch_c_adapter.h"
#include "kll_float_sketch_c_adapter.h"

enum aod_agg_state_type { MUTABLE_SKETCH, IMMUTABLE_SKETCH, UNION, INTERSECTION };

struct aod_agg_state {
  enum aod_agg_state_type type;
  unsigned lg_k;
  unsigned num_values;
  void* ptr;
};

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_aod_sketch_build_agg);
PG_FUNCTION_INFO_V1(pg_aod_sketch_union_agg);
PG_FUNCTION_INFO_V1(pg_aod_sketch_intersection_agg);
PG_FUNCTION_INFO_V1(pg_aod_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_aod_sketch_union_combine);
PG_FUNCTION_INFO_V1(pg_aod_sketch_intersection_combine);
PG_FUNCTION_INFO_V1(pg_aod_sketch_serialize_state);
PG_FUNCTION_INFO_V1(pg_aod_sketch_deserialize_state);
PG_FUNCTION_INFO_V1(pg_aod_sketch_get_estimate);
PG_FUNCTION_INFO_V1(pg_aod_sketch_get_estimate_and_bounds);
PG_FUNCTION_INFO_V1(pg_aod_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_aod_sketch_union);
PG_FUNCTION_INFO_V1(pg_aod_sketch_intersection);
PG_FUNCTION_INFO_V1(pg_aod_sketch_a_not_b);
PG_FUNCTION_INFO_V1(pg_aod_sketch_to_kll_float_sketch);
PG_FUNCTION_INFO_V1(pg_aod_sketch_students_t_test);
PG_FUNCTION_INFO_V1(pg_aod_sketch_to_means);
PG_FUNCTION_INFO_V1(pg_aod_sketch_to_variances);

/* function declarations */
Datum pg_aod_sketch_build_agg(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_union_agg(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_intersection_agg(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_union_combine(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_intersection_combine(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_serialize_state(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_deserialize_state(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_get_estimate(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_union(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_intersection(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_a_not_b(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_to_kll_float_sketch(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_students_t_test(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_to_means(PG_FUNCTION_ARGS);
Datum pg_aod_sketch_to_variances(PG_FUNCTION_ARGS);

Datum pg_aod_sketch_build_agg(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr;
  float p;

  // anyelement
  Oid   element_type;
  Datum element;
  int16 typlen;
  bool  typbyval;
  char  typalign;

  // input array of doubles
  ArrayType* arr_in;
  Oid elmtype_in;
  int16 elmlen_in;
  bool elmbyval_in;
  char elmalign_in;
  Datum* data_in;
  bool* nulls_in;
  int arr_len;
  double* values;
  int i;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  } else if (PG_ARGISNULL(1)) {
    PG_RETURN_POINTER(PG_GETARG_POINTER(0)); // no update value. return unmodified state
  }

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "aod_sketch_build_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  // look at the array of values first to know the array length in case we need to create a new sketch
  arr_in = PG_GETARG_ARRAYTYPE_P(2);
  elmtype_in = ARR_ELEMTYPE(arr_in);
  get_typlenbyvalalign(elmtype_in, &elmlen_in, &elmbyval_in, &elmalign_in);
  deconstruct_array(arr_in, elmtype_in, elmlen_in, elmbyval_in, elmalign_in, &data_in, &nulls_in, &arr_len);

  values = palloc(sizeof(double) * arr_len);
  for (i = 0; i < arr_len; i++) {
    values[i] = DatumGetFloat8(data_in[i]);
  }

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct aod_agg_state));
    stateptr->type = MUTABLE_SKETCH;
    stateptr->lg_k = PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 0;
    stateptr->num_values = arr_len;
    p = PG_NARGS() > 4 ? PG_GETARG_FLOAT4(4) : 0;
    if (stateptr->lg_k) {
      stateptr->ptr = p ? aod_sketch_new_lgk_p(arr_len, stateptr->lg_k, p) : aod_sketch_new_lgk(arr_len, stateptr->lg_k);
    } else {
      stateptr->ptr = aod_sketch_new(arr_len);
    }
  } else {
    stateptr = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  }

  element_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
  element = PG_GETARG_DATUM(1);
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  if (typlen == -1) {
    // varlena
    aod_sketch_update(stateptr->ptr, VARDATA_ANY(element), VARSIZE_ANY_EXHDR(element), values);
  } else if (typbyval) {
    // fixed-length passed by value
    aod_sketch_update(stateptr->ptr, &element, typlen, values);
  } else {
    // fixed-length passed by reference
    aod_sketch_update(stateptr->ptr, (void*)element, typlen, values);
  }
  pfree(values);
  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_aod_sketch_union_agg(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr;
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
    elog(ERROR, "aod_sketch_union_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct aod_agg_state));
    stateptr->type = UNION;
    stateptr->num_values = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 1;
    stateptr->lg_k = PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 0;
    stateptr->ptr = stateptr->lg_k ? aod_union_new_lgk(stateptr->num_values, stateptr->lg_k) : aod_union_new(stateptr->num_values);
  } else {
    stateptr = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = aod_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  if (stateptr->num_values != aod_sketch_get_num_values(sketchptr)) {
    compact_aod_sketch_delete(sketchptr);
    elog(ERROR, "pg_aod_sketch_union_agg expects the same num_values in sketches");
  }
  aod_union_update(stateptr->ptr, sketchptr);
  compact_aod_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_aod_sketch_intersection_agg(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr;
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
    elog(ERROR, "aod_sketch_intersection_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct aod_agg_state));
    stateptr->type = INTERSECTION;
    stateptr->num_values = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 1;
    stateptr->ptr = aod_intersection_new(stateptr->num_values);
  } else {
    stateptr = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = aod_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  aod_intersection_update(stateptr->ptr, sketchptr);
  compact_aod_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_aod_sketch_from_internal(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "aod_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  if (stateptr->type == MUTABLE_SKETCH) {
    stateptr->ptr = aod_sketch_compact(stateptr->ptr);
  } else if (stateptr->type == UNION) {
    stateptr->ptr = aod_union_get_result(stateptr->ptr);
  } else if (stateptr->type == INTERSECTION) {
    stateptr->ptr = aod_intersection_get_result(stateptr->ptr);
  }
  bytes_out = aod_sketch_serialize(stateptr->ptr, VARHDRSZ);
  compact_aod_sketch_delete(stateptr->ptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_aod_sketch_union_combine(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr1;
  struct aod_agg_state* stateptr2;
  struct aod_agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "aod_sketch_union_combine called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr1 = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  stateptr2 = (struct aod_agg_state*) PG_GETARG_POINTER(1);

  stateptr = palloc(sizeof(struct aod_agg_state));
  stateptr->type = IMMUTABLE_SKETCH;
  stateptr->lg_k = stateptr1 ? stateptr1->lg_k : stateptr2->lg_k;
  stateptr->num_values = stateptr1 ? stateptr1->num_values : stateptr2->num_values;
  stateptr->ptr = stateptr->lg_k ? aod_union_new_lgk(stateptr->num_values, stateptr->lg_k) : aod_union_new(stateptr->num_values);
  if (stateptr1) {
    if (stateptr1->type == UNION) {
      stateptr1->ptr = aod_union_get_result(stateptr1->ptr);
    } else if (stateptr1->type == MUTABLE_SKETCH) {
      stateptr1->ptr = aod_sketch_compact(stateptr1->ptr);
    }
    aod_union_update(stateptr->ptr, stateptr1->ptr);
    compact_aod_sketch_delete(stateptr1->ptr);
    pfree(stateptr1);
  }
  if (stateptr2) {
    if (stateptr2->type == UNION) {
      stateptr2->ptr = aod_union_get_result(stateptr2->ptr);
    } else if (stateptr2->type == MUTABLE_SKETCH) {
      stateptr2->ptr = aod_sketch_compact(stateptr2->ptr);
    }
    aod_union_update(stateptr->ptr, stateptr2->ptr);
    compact_aod_sketch_delete(stateptr2->ptr);
    pfree(stateptr2);
  }
  stateptr->ptr = aod_union_get_result(stateptr->ptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_aod_sketch_intersection_combine(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr1;
  struct aod_agg_state* stateptr2;
  struct aod_agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "aod_sketch_intersection_combine called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr1 = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  stateptr2 = (struct aod_agg_state*) PG_GETARG_POINTER(1);

  stateptr = palloc(sizeof(struct aod_agg_state));
  stateptr->type = IMMUTABLE_SKETCH;
  stateptr->num_values = stateptr1 ? stateptr1->num_values : stateptr2->num_values;
  stateptr->ptr = aod_intersection_new(stateptr->num_values);
  if (stateptr1) {
    if (stateptr1->type == INTERSECTION) {
      stateptr1->ptr = aod_intersection_get_result(stateptr1->ptr);
    }
    aod_intersection_update(stateptr->ptr, stateptr1->ptr);
    compact_aod_sketch_delete(stateptr1->ptr);
    pfree(stateptr1);
  }
  if (stateptr2) {
    if (stateptr2->type == INTERSECTION) {
      stateptr2->ptr = aod_intersection_get_result(stateptr2->ptr);
    }
    aod_intersection_update(stateptr->ptr, stateptr2->ptr);
    compact_aod_sketch_delete(stateptr2->ptr);
    pfree(stateptr2);
  }
  stateptr->ptr = aod_intersection_get_result(stateptr->ptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_aod_sketch_serialize_state(PG_FUNCTION_ARGS) {
  struct aod_agg_state* stateptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "aod_sketch_serialize_state called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct aod_agg_state*) PG_GETARG_POINTER(0);
  if (stateptr->type == MUTABLE_SKETCH) {
    stateptr->ptr = aod_sketch_compact(stateptr->ptr);
  } else if (stateptr->type == UNION) {
    stateptr->ptr = aod_union_get_result(stateptr->ptr);
  } else if (stateptr->type == INTERSECTION) {
    stateptr->ptr = aod_intersection_get_result(stateptr->ptr);
  }
  bytes_out = aod_sketch_serialize(stateptr->ptr, VARHDRSZ + 2);
  ((char*)bytes_out.ptr)[VARHDRSZ] = stateptr->lg_k;
  ((char*)bytes_out.ptr)[VARHDRSZ + 1] = stateptr->num_values;
  compact_aod_sketch_delete(stateptr->ptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_aod_sketch_deserialize_state(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  struct aod_agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "aod_sketch_deserialize_state called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  bytes_in = PG_GETARG_BYTEA_P(0);
  stateptr = palloc(sizeof(struct aod_agg_state));
  stateptr->type = IMMUTABLE_SKETCH;
  stateptr->lg_k = *VARDATA(bytes_in);
  stateptr->num_values = *(VARDATA(bytes_in) + 1);
  stateptr->ptr = aod_sketch_deserialize(VARDATA(bytes_in) + 2, VARSIZE(bytes_in) - VARHDRSZ - 2);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_aod_sketch_get_estimate(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  double estimate;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = aod_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  estimate = compact_aod_sketch_get_estimate(sketchptr);
  compact_aod_sketch_delete(sketchptr);
  PG_RETURN_FLOAT8(estimate);
}

Datum pg_aod_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS) {
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
  sketchptr = aod_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  num_std_devs = PG_NARGS() > 1 ? PG_GETARG_INT32(1) : 1;
  est_and_bounds = (Datum*) aod_sketch_get_estimate_and_bounds(sketchptr, num_std_devs);
  compact_aod_sketch_delete(sketchptr);

  // construct output array
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(est_and_bounds, 3, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);
  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_aod_sketch_to_string(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;
  bool print_entries;
  char* str;
  bytes_in = PG_GETARG_BYTEA_P(0);
  print_entries = PG_NARGS() > 1 ? PG_GETARG_BOOL(1) : false;
  sketchptr = aod_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  str = aod_sketch_to_string(sketchptr, print_entries);
  compact_aod_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
}

Datum pg_aod_sketch_union(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* unionptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;
  int num_values;
  int lg_k;
  
  num_values = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 1;
  lg_k = PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 0;
  unionptr = lg_k ? aod_union_new_lgk(num_values, lg_k) : aod_union_new(num_values);
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    sketchptr1 = aod_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
    aod_union_update(unionptr, sketchptr1);
    compact_aod_sketch_delete(sketchptr1);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    sketchptr2 = aod_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
    aod_union_update(unionptr, sketchptr2);
    compact_aod_sketch_delete(sketchptr2);
  }
  sketchptr = aod_union_get_result(unionptr);
  aod_union_delete(unionptr);
  bytes_out = aod_sketch_serialize(sketchptr, VARHDRSZ);
  compact_aod_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_aod_sketch_intersection(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* interptr;
  void* sketchptr;
  struct ptr_with_size bytes_out;
  int num_values;
  
  num_values = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : 1;
  interptr = aod_intersection_new(num_values);
  if (!PG_ARGISNULL(0)) {
    bytes_in1 = PG_GETARG_BYTEA_P(0);
    sketchptr1 = aod_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
    aod_intersection_update(interptr, sketchptr1);
    compact_aod_sketch_delete(sketchptr1);
  }
  if (!PG_ARGISNULL(1)) {
    bytes_in2 = PG_GETARG_BYTEA_P(1);
    sketchptr2 = aod_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
    aod_intersection_update(interptr, sketchptr2);
    compact_aod_sketch_delete(sketchptr2);
  }
  sketchptr = aod_intersection_get_result(interptr);
  aod_intersection_delete(interptr);
  bytes_out = aod_sketch_serialize(sketchptr, VARHDRSZ);
  compact_aod_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_aod_sketch_a_not_b(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;
  void* sketchptr;
  struct ptr_with_size bytes_out;

  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
    elog(ERROR, "aod_a_not_b expects two valid aod sketches");
  }

  bytes_in1 = PG_GETARG_BYTEA_P(0);
  sketchptr1 = aod_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
  bytes_in2 = PG_GETARG_BYTEA_P(1);
  sketchptr2 = aod_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
  sketchptr = aod_a_not_b(sketchptr1, sketchptr2);
  compact_aod_sketch_delete(sketchptr1);
  compact_aod_sketch_delete(sketchptr2);
  bytes_out = aod_sketch_serialize(sketchptr, VARHDRSZ);
  compact_aod_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_aod_sketch_to_kll_float_sketch(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* aodptr;
  int column_index;
  int k;
  void* kllptr;
  struct ptr_with_size bytes_out;

  bytes_in = PG_GETARG_BYTEA_P(0);
  aodptr = aod_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  column_index = PG_GETARG_INT32(1);
  k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : DEFAULT_K;
  kllptr = aod_sketch_to_kll_float_sketch(aodptr, column_index, k);
  bytes_out = kll_float_sketch_serialize(kllptr, VARHDRSZ);
  kll_float_sketch_delete(kllptr);
  compact_aod_sketch_delete(aodptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_aod_sketch_students_t_test(PG_FUNCTION_ARGS) {
  const bytea* bytes_in1;
  const bytea* bytes_in2;
  void* sketchptr1;
  void* sketchptr2;

  // output array of p-values
  Datum* p_values;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;
  unsigned arr_len_out;

  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
    elog(ERROR, "aod_a_not_b expects two valid aod sketches");
  }

  bytes_in1 = PG_GETARG_BYTEA_P(0);
  sketchptr1 = aod_sketch_deserialize(VARDATA(bytes_in1), VARSIZE(bytes_in1) - VARHDRSZ);
  bytes_in2 = PG_GETARG_BYTEA_P(1);
  sketchptr2 = aod_sketch_deserialize(VARDATA(bytes_in2), VARSIZE(bytes_in2) - VARHDRSZ);
  p_values = (Datum*) aod_sketch_students_t_test(sketchptr1, sketchptr2, &arr_len_out);
  compact_aod_sketch_delete(sketchptr1);
  compact_aod_sketch_delete(sketchptr2);

  // construct output array of p-values
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(p_values, arr_len_out, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_aod_sketch_to_means(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;

  // output array
  Datum* means;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;
  unsigned arr_len_out;

  if (PG_ARGISNULL(0)) {
    elog(ERROR, "aod_sketch_to_means expects a valid aod sketch");
  }

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = aod_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);

  means = (Datum*) aod_sketch_to_means(sketchptr, &arr_len_out);
  compact_aod_sketch_delete(sketchptr);

  // construct output array
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(means, arr_len_out, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}

Datum pg_aod_sketch_to_variances(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  void* sketchptr;

  // output array
  Datum* variances;
  ArrayType* arr_out;
  int16 elmlen_out;
  bool elmbyval_out;
  char elmalign_out;
  unsigned arr_len_out;

  if (PG_ARGISNULL(0)) {
    elog(ERROR, "aod_sketch_to_variances expects a valid aod sketch");
  }

  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = aod_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);

  variances = (Datum*) aod_sketch_to_variances(sketchptr, &arr_len_out);
  compact_aod_sketch_delete(sketchptr);

  // construct output array
  get_typlenbyvalalign(FLOAT8OID, &elmlen_out, &elmbyval_out, &elmalign_out);
  arr_out = construct_array(variances, arr_len_out, FLOAT8OID, elmlen_out, elmbyval_out, elmalign_out);

  PG_RETURN_ARRAYTYPE_P(arr_out);
}
