-- Licensed to the Apache Software Foundation (ASF) under one
-- or more contributor license agreements.  See the NOTICE file
-- distributed with this work for additional information
-- regarding copyright ownership.  The ASF licenses this file
-- to you under the Apache License, Version 2.0 (the
-- "License"); you may not use this file except in compliance
-- with the License.  You may obtain a copy of the License at
--
--   http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing,
-- software distributed under the License is distributed on an
-- "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
-- KIND, either express or implied.  See the License for the
-- specific language governing permissions and limitations
-- under the License.

CREATE TYPE aod_sketch;

CREATE OR REPLACE FUNCTION aod_sketch_in(cstring) RETURNS aod_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_out(aod_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE TYPE aod_sketch (
    INPUT = aod_sketch_in,
    OUTPUT = aod_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as aod_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (aod_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION aod_sketch_build_agg(internal, anyelement, double precision[]) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_build_agg(internal, anyelement, double precision[], int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_build_agg(internal, anyelement, double precision[], int, real) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_from_internal(internal) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_union_agg(internal, aod_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_union_agg(internal, aod_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_union_agg(internal, aod_sketch, int, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection_agg(internal, aod_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;    

CREATE OR REPLACE FUNCTION aod_sketch_intersection_agg(internal, aod_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;    

CREATE OR REPLACE FUNCTION aod_sketch_union_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_serialize_state(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_aod_sketch_serialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_deserialize_state(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_deserialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE aod_sketch_build(anyelement, double precision[]) (
    STYPE = internal,
    SFUNC = aod_sketch_build_agg,
    COMBINEFUNC = aod_sketch_union_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_build(anyelement, double precision[], int) (
    STYPE = internal,
    SFUNC = aod_sketch_build_agg,
    COMBINEFUNC = aod_sketch_union_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_build(anyelement, double precision[], int, real) (
    STYPE = internal,
    SFUNC = aod_sketch_build_agg,
    COMBINEFUNC = aod_sketch_union_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_union(aod_sketch) (
    STYPE = internal,
    SFUNC = aod_sketch_union_agg,
    COMBINEFUNC = aod_sketch_union_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_union(aod_sketch, int) (
    STYPE = internal,
    SFUNC = aod_sketch_union_agg,
    COMBINEFUNC = aod_sketch_union_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_union(aod_sketch, int, int) (
    STYPE = internal,
    SFUNC = aod_sketch_union_agg,
    COMBINEFUNC = aod_sketch_union_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_intersection(aod_sketch) (
    STYPE = internal,
    SFUNC = aod_sketch_intersection_agg,
    COMBINEFUNC = aod_sketch_intersection_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE aod_sketch_intersection(aod_sketch, int) (
    STYPE = internal,
    SFUNC = aod_sketch_intersection_agg,
    COMBINEFUNC = aod_sketch_intersection_combine,
    SERIALFUNC = aod_sketch_serialize_state,
    DESERIALFUNC = aod_sketch_deserialize_state, 
    FINALFUNC = aod_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION aod_sketch_get_estimate(aod_sketch) RETURNS double precision
    AS '$libdir/datasketches', 'pg_aod_sketch_get_estimate'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_get_estimate_and_bounds(aod_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_get_estimate_and_bounds(aod_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_to_string(aod_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_aod_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_to_string(aod_sketch, boolean) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_aod_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_union(aod_sketch, aod_sketch) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_union'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_union(aod_sketch, aod_sketch, int) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_union'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection(aod_sketch, aod_sketch) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection(aod_sketch, aod_sketch, int) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_a_not_b(aod_sketch, aod_sketch) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_a_not_b'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_a_not_b(aod_sketch, aod_sketch, int) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_a_not_b'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_to_kll_float_sketch(aod_sketch, int) RETURNS kll_float_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_to_kll_float_sketch'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_to_kll_float_sketch(aod_sketch, int, int) RETURNS kll_float_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_to_kll_float_sketch'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_students_t_test(aod_sketch, aod_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_students_t_test'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_to_means(aod_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_to_means'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_to_variances(aod_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_to_variances'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;
