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

enum hll_agg_state_type { SKETCH, UNION };

struct hll_agg_state {
  enum hll_agg_state_type type;
  unsigned lg_k;
  unsigned tgt_type;
  void* ptr;
};

const unsigned HLL_DEFAULT_LG_K = 12;

/* PG_FUNCTION_INFO_V1 macro to pass functions to postgres */
PG_FUNCTION_INFO_V1(pg_hll_sketch_build_agg);
PG_FUNCTION_INFO_V1(pg_hll_sketch_union_agg);
PG_FUNCTION_INFO_V1(pg_hll_sketch_from_internal);
PG_FUNCTION_INFO_V1(pg_hll_sketch_get_estimate_from_internal);
PG_FUNCTION_INFO_V1(pg_hll_sketch_combine);
PG_FUNCTION_INFO_V1(pg_hll_sketch_serialize_state);
PG_FUNCTION_INFO_V1(pg_hll_sketch_deserialize_state);
PG_FUNCTION_INFO_V1(pg_hll_sketch_get_estimate);
PG_FUNCTION_INFO_V1(pg_hll_sketch_get_estimate_and_bounds);
PG_FUNCTION_INFO_V1(pg_hll_sketch_to_string);
PG_FUNCTION_INFO_V1(pg_hll_sketch_union);

/* function declarations */
Datum pg_hll_sketch_build_agg(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_union_agg(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_from_internal(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_combine(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_serialize_state(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_deserialize_state(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_get_estimate(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_get_estimate_and_bounds(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_to_string(PG_FUNCTION_ARGS);
Datum pg_hll_sketch_union(PG_FUNCTION_ARGS);

Datum pg_hll_sketch_build_agg(PG_FUNCTION_ARGS) {
  struct hll_agg_state* stateptr;

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
    elog(ERROR, "hll_sketch_build_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct hll_agg_state));
    stateptr->type = SKETCH;
    stateptr->lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : HLL_DEFAULT_LG_K;
    stateptr->tgt_type = PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 0;
    if (stateptr->tgt_type) {
      if ((stateptr->tgt_type != 4) && (stateptr->tgt_type != 6) && (stateptr->tgt_type != 8)) {
        elog(ERROR, "hll_sketch_build_agg: unsupported target type, must be 4, 6 or 8");
      }
      stateptr->ptr = hll_sketch_new_tgt_type(stateptr->lg_k, stateptr->tgt_type);
    } else {
      stateptr->ptr = hll_sketch_new(stateptr->lg_k);
    }
  } else {
    stateptr = (struct hll_agg_state*) PG_GETARG_POINTER(0);
  }

  element_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
  element = PG_GETARG_DATUM(1);
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  if (typlen == -1) {
    // varlena
    hll_sketch_update(stateptr->ptr, VARDATA_ANY(element), VARSIZE_ANY_EXHDR(element));
  } else if (typbyval) {
    // fixed-length passed by value
    hll_sketch_update(stateptr->ptr, &element, typlen);
  } else {
    // fixed-length passed by reference
    hll_sketch_update(stateptr->ptr, (void*)element, typlen);
  }

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_hll_sketch_union_agg(PG_FUNCTION_ARGS) {
  struct hll_agg_state* stateptr;
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
    elog(ERROR, "hll_sketch_union_agg called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  if (PG_ARGISNULL(0)) {
    stateptr = palloc(sizeof(struct hll_agg_state));
    stateptr->type = UNION;
    stateptr->lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : HLL_DEFAULT_LG_K;
    stateptr->tgt_type = PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 0;
    if (stateptr->tgt_type) {
      if ((stateptr->tgt_type != 4) && (stateptr->tgt_type != 6) && (stateptr->tgt_type != 8)) {
        elog(ERROR, "hll_sketch_union_agg: unsupported target type, must be 4, 6 or 8");
      }
    }
    stateptr->ptr = hll_union_new(stateptr->lg_k);
  } else {
    stateptr = (struct hll_agg_state*) PG_GETARG_POINTER(0);
  }

  sketch_bytes = PG_GETARG_BYTEA_P(1);
  sketchptr = hll_sketch_deserialize(VARDATA(sketch_bytes), VARSIZE(sketch_bytes) - VARHDRSZ);
  hll_union_update(stateptr->ptr, sketchptr);
  hll_sketch_delete(sketchptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_hll_sketch_from_internal(PG_FUNCTION_ARGS) {
  struct hll_agg_state* stateptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct hll_agg_state*) PG_GETARG_POINTER(0);
  if (stateptr->type == UNION) {
    if (stateptr->tgt_type) {
      stateptr->ptr = hll_union_get_result_tgt_type(stateptr->ptr, stateptr->tgt_type);
    } else {
      stateptr->ptr = hll_union_get_result(stateptr->ptr);
    }
  }
  bytes_out = hll_sketch_serialize(stateptr->ptr, VARHDRSZ);
  hll_sketch_delete(stateptr->ptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_hll_sketch_get_estimate_from_internal(PG_FUNCTION_ARGS) {
  struct hll_agg_state* stateptr;
  double estimate;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_get_estimate_from_internal called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct hll_agg_state*) PG_GETARG_POINTER(0);
  estimate = hll_sketch_get_estimate(stateptr->ptr);
  hll_sketch_delete(stateptr->ptr);
  pfree(stateptr);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_FLOAT8(estimate);
}

Datum pg_hll_sketch_combine(PG_FUNCTION_ARGS) {
  struct hll_agg_state* stateptr1;
  struct hll_agg_state* stateptr2;
  struct hll_agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0) && PG_ARGISNULL(1)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_combine called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr1 = (struct hll_agg_state*) PG_GETARG_POINTER(0);
  stateptr2 = (struct hll_agg_state*) PG_GETARG_POINTER(1);

  stateptr = palloc(sizeof(struct hll_agg_state));
  stateptr->type = SKETCH;
  stateptr->lg_k = stateptr1 ? stateptr1->lg_k : stateptr2->lg_k;
  stateptr->tgt_type = stateptr1 ? stateptr1->tgt_type : stateptr2->tgt_type;
  stateptr->ptr = hll_union_new(stateptr->lg_k);

  if (stateptr1) {
    if (stateptr1->type == UNION) {
      if (stateptr1->tgt_type) {
        stateptr1->ptr = hll_union_get_result_tgt_type(stateptr1->ptr, stateptr1->tgt_type);
      } else {
        stateptr1->ptr = hll_union_get_result(stateptr1->ptr);
      }
    }
    hll_union_update(stateptr->ptr, stateptr1->ptr);
    hll_sketch_delete(stateptr1->ptr);
    pfree(stateptr1);
  }
  if (stateptr2) {
    if (stateptr2->type == UNION) {
      if (stateptr2->tgt_type) {
        stateptr2->ptr = hll_union_get_result_tgt_type(stateptr2->ptr, stateptr2->tgt_type);
      } else {
        stateptr2->ptr = hll_union_get_result(stateptr2->ptr);
      }
    }
    hll_union_update(stateptr->ptr, stateptr2->ptr);
    hll_sketch_delete(stateptr2->ptr);
    pfree(stateptr2);
  }
  if (stateptr->tgt_type) {
    stateptr->ptr = hll_union_get_result_tgt_type(stateptr->ptr, stateptr->tgt_type);
  } else {
    stateptr->ptr = hll_union_get_result(stateptr->ptr);
  }

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
}

Datum pg_hll_sketch_serialize_state(PG_FUNCTION_ARGS) {
  struct hll_agg_state* stateptr;
  struct ptr_with_size bytes_out;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_serialize_state called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  stateptr = (struct hll_agg_state*) PG_GETARG_POINTER(0);
  if (stateptr->type == UNION) {
    if (stateptr->tgt_type) {
      stateptr->ptr = hll_union_get_result_tgt_type(stateptr->ptr, stateptr->tgt_type);
    } else {
      stateptr->ptr = hll_union_get_result(stateptr->ptr);
    }
  }
  bytes_out = hll_sketch_serialize(stateptr->ptr, VARHDRSZ + 2);
  ((char*)bytes_out.ptr)[VARHDRSZ] = stateptr->lg_k;
  ((char*)bytes_out.ptr)[VARHDRSZ + 1] = stateptr->tgt_type;
  hll_sketch_delete(stateptr->ptr);
  pfree(stateptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_BYTEA_P(bytes_out.ptr);
}

Datum pg_hll_sketch_deserialize_state(PG_FUNCTION_ARGS) {
  const bytea* bytes_in;
  struct hll_agg_state* stateptr;

  MemoryContext oldcontext;
  MemoryContext aggcontext;

  if (PG_ARGISNULL(0)) PG_RETURN_NULL();

  if (!AggCheckCallContext(fcinfo, &aggcontext)) {
    elog(ERROR, "hll_sketch_deserialize_state called in non-aggregate context");
  }
  oldcontext = MemoryContextSwitchTo(aggcontext);

  bytes_in = PG_GETARG_BYTEA_P(0);
  stateptr = palloc(sizeof(struct hll_agg_state));
  stateptr->type = SKETCH;
  stateptr->lg_k = *VARDATA(bytes_in);
  stateptr->tgt_type = *(VARDATA(bytes_in) + 1);
  stateptr->ptr = hll_sketch_deserialize(VARDATA(bytes_in) + 2, VARSIZE(bytes_in) - VARHDRSZ - 2);

  MemoryContextSwitchTo(oldcontext);

  PG_RETURN_POINTER(stateptr);
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
  num_std_devs = PG_NARGS() > 1 ? PG_GETARG_INT32(1) : 1;
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
  char* str;
  bytes_in = PG_GETARG_BYTEA_P(0);
  sketchptr = hll_sketch_deserialize(VARDATA(bytes_in), VARSIZE(bytes_in) - VARHDRSZ);
  str = hll_sketch_to_string(sketchptr);
  hll_sketch_delete(sketchptr);
  PG_RETURN_TEXT_P(cstring_to_text(str));
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

  lg_k = PG_NARGS() > 2 ? PG_GETARG_INT32(2) : HLL_DEFAULT_LG_K;
  tgt_type = PG_NARGS() > 3 ? PG_GETARG_INT32(3) : 0;
  if (tgt_type) {
    if ((tgt_type != 4) && (tgt_type != 6) && (tgt_type != 8)) {
      elog(ERROR, "hll_sketch_union: unsupported target type, must be 4, 6 or 8");
    }
  }
  unionptr = hll_union_new(lg_k);
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
  bytes_out = hll_sketch_serialize(sketchptr, VARHDRSZ);
  hll_sketch_delete(sketchptr);
  SET_VARSIZE(bytes_out.ptr, bytes_out.size);
  
  PG_RETURN_BYTEA_P(bytes_out.ptr);
}
