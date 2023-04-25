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

CREATE OR REPLACE FUNCTION cpc_sketch_in(cstring) RETURNS cpc_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_out(cpc_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_build_agg(internal, anyelement) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_build_agg(internal, anyelement, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_from_internal(internal) RETURNS cpc_sketch
    AS '$libdir/datasketches', 'pg_cpc_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_get_estimate_from_internal(internal) RETURNS double precision
    AS '$libdir/datasketches', 'pg_cpc_sketch_get_estimate_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_union_agg(internal, cpc_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_union_agg(internal, cpc_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_serialize_state(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_cpc_sketch_serialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_deserialize_state(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_deserialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE cpc_sketch_distinct(anyelement) (
    STYPE = internal,
    SFUNC = cpc_sketch_build_agg,
    COMBINEFUNC = cpc_sketch_combine,
    SERIALFUNC = cpc_sketch_serialize_state,
    DESERIALFUNC = cpc_sketch_deserialize_state, 
    FINALFUNC = cpc_sketch_get_estimate_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE cpc_sketch_distinct(anyelement, int) (
    STYPE = internal,
    SFUNC = cpc_sketch_build_agg,
    COMBINEFUNC = cpc_sketch_combine,
    SERIALFUNC = cpc_sketch_serialize_state,
    DESERIALFUNC = cpc_sketch_deserialize_state, 
    FINALFUNC = cpc_sketch_get_estimate_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE cpc_sketch_build(anyelement) (
    STYPE = internal,
    SFUNC = cpc_sketch_build_agg,
    COMBINEFUNC = cpc_sketch_combine,
    SERIALFUNC = cpc_sketch_serialize_state,
    DESERIALFUNC = cpc_sketch_deserialize_state, 
    FINALFUNC = cpc_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE cpc_sketch_build(anyelement, int) (
    STYPE = internal,
    SFUNC = cpc_sketch_build_agg,
    COMBINEFUNC = cpc_sketch_combine,
    SERIALFUNC = cpc_sketch_serialize_state,
    DESERIALFUNC = cpc_sketch_deserialize_state, 
    FINALFUNC = cpc_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE cpc_sketch_union(cpc_sketch) (
    STYPE = internal,
    SFUNC = cpc_sketch_union_agg,
    COMBINEFUNC = cpc_sketch_combine,
    SERIALFUNC = cpc_sketch_serialize_state,
    DESERIALFUNC = cpc_sketch_deserialize_state, 
    FINALFUNC = cpc_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE cpc_sketch_union(cpc_sketch, int) (
    STYPE = internal,
    SFUNC = cpc_sketch_union_agg,
    COMBINEFUNC = cpc_sketch_combine,
    SERIALFUNC = cpc_sketch_serialize_state,
    DESERIALFUNC = cpc_sketch_deserialize_state, 
    FINALFUNC = cpc_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION cpc_sketch_get_estimate(cpc_sketch) RETURNS double precision
    AS '$libdir/datasketches', 'pg_cpc_sketch_get_estimate'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_get_estimate_and_bounds(cpc_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_cpc_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_get_estimate_and_bounds(cpc_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_cpc_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_to_string(cpc_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_cpc_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_union(cpc_sketch, cpc_sketch) RETURNS cpc_sketch
    AS '$libdir/datasketches', 'pg_cpc_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_union(cpc_sketch, cpc_sketch, int) RETURNS cpc_sketch
    AS '$libdir/datasketches', 'pg_cpc_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

DROP FUNCTION cpc_sketch_add_item(internal, anyelement);
DROP FUNCTION cpc_sketch_add_item(internal, anyelement, int);


CREATE OR REPLACE FUNCTION kll_float_sketch_in(cstring) RETURNS kll_float_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_out(kll_float_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_build_agg(internal, real) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_build_agg(internal, real, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_merge_agg(internal, kll_float_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_merge_agg(internal, kll_float_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_serialize(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_kll_float_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_deserialize(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_deserialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_finalize(internal) RETURNS kll_float_sketch
    AS '$libdir/datasketches', 'pg_kll_float_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE kll_float_sketch_build(real) (
    STYPE = internal,
    SFUNC = kll_float_sketch_build_agg,
    COMBINEFUNC = kll_float_sketch_combine,
    SERIALFUNC = kll_float_sketch_serialize,
    DESERIALFUNC = kll_float_sketch_deserialize, 
    FINALFUNC = kll_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE kll_float_sketch_build(real, int) (
    STYPE = internal,
    SFUNC = kll_float_sketch_build_agg,
    COMBINEFUNC = kll_float_sketch_combine,
    SERIALFUNC = kll_float_sketch_serialize,
    DESERIALFUNC = kll_float_sketch_deserialize, 
    FINALFUNC = kll_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE kll_float_sketch_merge(kll_float_sketch) (
    STYPE = internal,
    SFUNC = kll_float_sketch_merge_agg,
    COMBINEFUNC = kll_float_sketch_combine,
    SERIALFUNC = kll_float_sketch_serialize,
    DESERIALFUNC = kll_float_sketch_deserialize, 
    FINALFUNC = kll_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE kll_float_sketch_merge(kll_float_sketch, int) (
    STYPE = internal,
    SFUNC = kll_float_sketch_merge_agg,
    COMBINEFUNC = kll_float_sketch_combine,
    SERIALFUNC = kll_float_sketch_serialize,
    DESERIALFUNC = kll_float_sketch_deserialize, 
    FINALFUNC = kll_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION kll_float_sketch_get_rank(kll_float_sketch, real) RETURNS double precision
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_rank'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_quantile(kll_float_sketch, double precision) RETURNS real
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_quantile'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_n(kll_float_sketch) RETURNS bigint
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_n'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_to_string(kll_float_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_kll_float_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_pmf(kll_float_sketch, real[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_pmf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_cdf(kll_float_sketch, real[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_cdf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_quantiles(kll_float_sketch, double precision[]) RETURNS real[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_quantiles'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_histogram(kll_float_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_histogram(kll_float_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

DROP FUNCTION kll_float_sketch_add_item(internal, real);
DROP FUNCTION kll_float_sketch_add_item(internal, real, int);
DROP FUNCTION kll_float_sketch_merge(internal, kll_float_sketch);
DROP FUNCTION kll_float_sketch_merge(internal, kll_float_sketch, int);
DROP FUNCTION kll_float_sketch_from_internal(internal);


CREATE TYPE kll_double_sketch;

CREATE OR REPLACE FUNCTION kll_double_sketch_in(cstring) RETURNS kll_double_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_out(kll_double_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE TYPE kll_double_sketch (
    INPUT = kll_double_sketch_in,
    OUTPUT = kll_double_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as kll_double_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (kll_double_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION kll_double_sketch_build_agg(internal, double precision) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_double_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_build_agg(internal, double precision, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_double_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_merge_agg(internal, kll_double_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_double_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_merge_agg(internal, kll_double_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_double_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_serialize(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_kll_double_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_deserialize(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_double_sketch_deserialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_double_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_finalize(internal) RETURNS kll_double_sketch
    AS '$libdir/datasketches', 'pg_kll_double_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE kll_double_sketch_build(double precision) (
    STYPE = internal,
    SFUNC = kll_double_sketch_build_agg,
    COMBINEFUNC = kll_double_sketch_combine,
    SERIALFUNC = kll_double_sketch_serialize,
    DESERIALFUNC = kll_double_sketch_deserialize, 
    FINALFUNC = kll_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE kll_double_sketch_build(double precision, int) (
    STYPE = internal,
    SFUNC = kll_double_sketch_build_agg,
    COMBINEFUNC = kll_double_sketch_combine,
    SERIALFUNC = kll_double_sketch_serialize,
    DESERIALFUNC = kll_double_sketch_deserialize, 
    FINALFUNC = kll_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE kll_double_sketch_merge(kll_double_sketch) (
    STYPE = internal,
    SFUNC = kll_double_sketch_merge_agg,
    COMBINEFUNC = kll_double_sketch_combine,
    SERIALFUNC = kll_double_sketch_serialize,
    DESERIALFUNC = kll_double_sketch_deserialize, 
    FINALFUNC = kll_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE kll_double_sketch_merge(kll_double_sketch, int) (
    STYPE = internal,
    SFUNC = kll_double_sketch_merge_agg,
    COMBINEFUNC = kll_double_sketch_combine,
    SERIALFUNC = kll_double_sketch_serialize,
    DESERIALFUNC = kll_double_sketch_deserialize, 
    FINALFUNC = kll_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION kll_double_sketch_get_rank(kll_double_sketch, double precision) RETURNS double precision
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_rank'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_quantile(kll_double_sketch, double precision) RETURNS double precision
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_quantile'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_n(kll_double_sketch) RETURNS bigint
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_n'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_to_string(kll_double_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_kll_double_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_pmf(kll_double_sketch, double precision[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_pmf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_cdf(kll_double_sketch, double precision[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_cdf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_quantiles(kll_double_sketch, double precision[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_quantiles'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_histogram(kll_double_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kll_double_sketch_get_histogram(kll_double_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_double_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;


CREATE OR REPLACE FUNCTION theta_sketch_in(cstring) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_sketch_in'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_out(theta_sketch) RETURNS cstring
    AS '$libdir/datasketches', 'pg_sketch_out'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_build_agg(internal, anyelement) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_build_agg(internal, anyelement, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_build_agg(internal, anyelement, int, real) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_from_internal(internal) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_get_estimate_from_internal(internal) RETURNS double precision
    AS '$libdir/datasketches', 'pg_theta_sketch_get_estimate_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_union_agg(internal, theta_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_union_agg(internal, theta_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_intersection_agg(internal, theta_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_intersection_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_union_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_union_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_intersection_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_intersection_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_serialize_state(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_theta_sketch_serialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_deserialize_state(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_deserialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE theta_sketch_distinct(anyelement) (
    STYPE = internal,
    SFUNC = theta_sketch_build_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_get_estimate_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_distinct(anyelement, int) (
    STYPE = internal,
    SFUNC = theta_sketch_build_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_get_estimate_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_build(anyelement) (
    STYPE = internal,
    SFUNC = theta_sketch_build_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_build(anyelement, int) (
    STYPE = internal,
    SFUNC = theta_sketch_build_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_build(anyelement, int, real) (
    STYPE = internal,
    SFUNC = theta_sketch_build_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_union(theta_sketch) (
    STYPE = internal,
    SFUNC = theta_sketch_union_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_union(theta_sketch, int) (
    STYPE = internal,
    SFUNC = theta_sketch_union_agg,
    COMBINEFUNC = theta_sketch_union_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE theta_sketch_intersection(theta_sketch) (
    STYPE = internal,
    SFUNC = theta_sketch_intersection_agg,
    COMBINEFUNC = theta_sketch_intersection_combine,
    SERIALFUNC = theta_sketch_serialize_state,
    DESERIALFUNC = theta_sketch_deserialize_state, 
    FINALFUNC = theta_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION theta_sketch_get_estimate(theta_sketch) RETURNS double precision
    AS '$libdir/datasketches', 'pg_theta_sketch_get_estimate'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_get_estimate_and_bounds(theta_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_theta_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_get_estimate_and_bounds(theta_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_theta_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_to_string(theta_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_theta_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_union(theta_sketch, theta_sketch) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_union(theta_sketch, theta_sketch, int) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_intersection(theta_sketch, theta_sketch) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_intersection'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_a_not_b(theta_sketch, theta_sketch) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_a_not_b'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

DROP FUNCTION theta_sketch_add_item(internal, anyelement);
DROP FUNCTION theta_sketch_add_item(internal, anyelement, int);
DROP FUNCTION theta_sketch_add_item(internal, anyelement, int, real);


CREATE OR REPLACE FUNCTION frequent_strings_sketch_in(cstring) RETURNS frequent_strings_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_out(frequent_strings_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_build_agg(internal, int, varchar) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_build_agg(internal, int, varchar, bigint) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_merge_agg(internal, int, frequent_strings_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_serialize(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_deserialize(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_deserialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_finalize(internal) RETURNS frequent_strings_sketch
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE frequent_strings_sketch_build(int, varchar) (
    STYPE = internal,
    SFUNC = frequent_strings_sketch_build_agg,
    COMBINEFUNC = frequent_strings_sketch_combine,
    SERIALFUNC = frequent_strings_sketch_serialize,
    DESERIALFUNC = frequent_strings_sketch_deserialize, 
    FINALFUNC = frequent_strings_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE frequent_strings_sketch_build(int, varchar, bigint) (
    STYPE = internal,
    SFUNC = frequent_strings_sketch_build_agg,
    COMBINEFUNC = frequent_strings_sketch_combine,
    SERIALFUNC = frequent_strings_sketch_serialize,
    DESERIALFUNC = frequent_strings_sketch_deserialize, 
    FINALFUNC = frequent_strings_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE frequent_strings_sketch_merge(int, frequent_strings_sketch) (
    STYPE = internal,
    SFUNC = frequent_strings_sketch_merge_agg,
    COMBINEFUNC = frequent_strings_sketch_combine,
    SERIALFUNC = frequent_strings_sketch_serialize,
    DESERIALFUNC = frequent_strings_sketch_deserialize, 
    FINALFUNC = frequent_strings_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION frequent_strings_sketch_to_string(frequent_strings_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_to_string(frequent_strings_sketch, boolean) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_positives(frequent_strings_sketch)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_positives'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_positives(frequent_strings_sketch, bigint)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_positives'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_negatives'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch, bigint)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_negatives'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

DROP FUNCTION frequent_strings_sketch_add_item(internal, int, varchar);
DROP FUNCTION frequent_strings_sketch_add_item(internal, int, varchar, bigint);
DROP FUNCTION frequent_strings_sketch_merge(internal, int, frequent_strings_sketch);
DROP FUNCTION frequent_strings_sketch_from_internal(internal);


CREATE OR REPLACE FUNCTION hll_sketch_in(cstring) RETURNS hll_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_out(hll_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_build_agg(internal, anyelement) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_build_agg(internal, anyelement, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_build_agg(internal, anyelement, int, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_from_internal(internal) RETURNS hll_sketch
    AS '$libdir/datasketches', 'pg_hll_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_get_estimate_from_internal(internal) RETURNS double precision
    AS '$libdir/datasketches', 'pg_hll_sketch_get_estimate_from_internal'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_union_agg(internal, hll_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_union_agg(internal, hll_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_union_agg(internal, hll_sketch, int, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_union_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_serialize_state(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_hll_sketch_serialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_deserialize_state(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_hll_sketch_deserialize_state'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE hll_sketch_distinct(anyelement) (
    STYPE = internal,
    SFUNC = hll_sketch_build_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_get_estimate_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_distinct(anyelement, int) (
    STYPE = internal,
    SFUNC = hll_sketch_build_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_get_estimate_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_build(anyelement) (
    STYPE = internal,
    SFUNC = hll_sketch_build_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_build(anyelement, int) (
    STYPE = internal,
    SFUNC = hll_sketch_build_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_build(anyelement, int, int) (
    STYPE = internal,
    SFUNC = hll_sketch_build_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_union(hll_sketch) (
    STYPE = internal,
    SFUNC = hll_sketch_union_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_union(hll_sketch, int) (
    STYPE = internal,
    SFUNC = hll_sketch_union_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE hll_sketch_union(hll_sketch, int, int) (
    STYPE = internal,
    SFUNC = hll_sketch_union_agg,
    COMBINEFUNC = hll_sketch_combine,
    SERIALFUNC = hll_sketch_serialize_state,
    DESERIALFUNC = hll_sketch_deserialize_state, 
    FINALFUNC = hll_sketch_from_internal,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION hll_sketch_get_estimate(hll_sketch) RETURNS double precision
    AS '$libdir/datasketches', 'pg_hll_sketch_get_estimate'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_get_estimate_and_bounds(hll_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_hll_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_get_estimate_and_bounds(hll_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_hll_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_to_string(hll_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_hll_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_union(hll_sketch, hll_sketch) RETURNS hll_sketch
    AS '$libdir/datasketches', 'pg_hll_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_union(hll_sketch, hll_sketch, int) RETURNS hll_sketch
    AS '$libdir/datasketches', 'pg_hll_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION hll_sketch_union(hll_sketch, hll_sketch, int, int) RETURNS hll_sketch
    AS '$libdir/datasketches', 'pg_hll_sketch_union'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

DROP FUNCTION hll_sketch_add_item(internal, anyelement);
DROP FUNCTION hll_sketch_add_item(internal, anyelement, int);
DROP FUNCTION hll_sketch_add_item(internal, anyelement, int, int);


CREATE OR REPLACE FUNCTION aod_sketch_in(cstring) RETURNS aod_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION aod_sketch_out(aod_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

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

DROP FUNCTION aod_sketch_add_item(internal, anyelement, double precision[]);
DROP FUNCTION aod_sketch_add_item(internal, anyelement, double precision[], int);
DROP FUNCTION aod_sketch_add_item(internal, anyelement, double precision[], int, real);


CREATE OR REPLACE FUNCTION req_float_sketch_in(cstring) RETURNS req_float_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_out(req_float_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_build_agg(internal, real) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_build_agg(internal, real, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_build_agg(internal, real, int, boolean) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_merge_agg(internal, req_float_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_merge_agg(internal, req_float_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_merge_agg(internal, req_float_sketch, int, boolean) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_serialize(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_req_float_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_deserialize(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_deserialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_req_float_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_finalize(internal) RETURNS req_float_sketch
    AS '$libdir/datasketches', 'pg_req_float_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE req_float_sketch_build(real) (
    STYPE = internal,
    SFUNC = req_float_sketch_build_agg,
    COMBINEFUNC = req_float_sketch_combine,
    SERIALFUNC = req_float_sketch_serialize,
    DESERIALFUNC = req_float_sketch_deserialize, 
    FINALFUNC = req_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE req_float_sketch_build(real, int) (
    STYPE = internal,
    SFUNC = req_float_sketch_build_agg,
    COMBINEFUNC = req_float_sketch_combine,
    SERIALFUNC = req_float_sketch_serialize,
    DESERIALFUNC = req_float_sketch_deserialize, 
    FINALFUNC = req_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE req_float_sketch_build(real, int, boolean) (
    STYPE = internal,
    SFUNC = req_float_sketch_build_agg,
    COMBINEFUNC = req_float_sketch_combine,
    SERIALFUNC = req_float_sketch_serialize,
    DESERIALFUNC = req_float_sketch_deserialize, 
    FINALFUNC = req_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE req_float_sketch_merge(req_float_sketch) (
    STYPE = internal,
    SFUNC = req_float_sketch_merge_agg,
    COMBINEFUNC = req_float_sketch_combine,
    SERIALFUNC = req_float_sketch_serialize,
    DESERIALFUNC = req_float_sketch_deserialize, 
    FINALFUNC = req_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE req_float_sketch_merge(req_float_sketch, int) (
    STYPE = internal,
    SFUNC = req_float_sketch_merge_agg,
    COMBINEFUNC = req_float_sketch_combine,
    SERIALFUNC = req_float_sketch_serialize,
    DESERIALFUNC = req_float_sketch_deserialize, 
    FINALFUNC = req_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE req_float_sketch_merge(req_float_sketch, int, boolean) (
    STYPE = internal,
    SFUNC = req_float_sketch_merge_agg,
    COMBINEFUNC = req_float_sketch_combine,
    SERIALFUNC = req_float_sketch_serialize,
    DESERIALFUNC = req_float_sketch_deserialize, 
    FINALFUNC = req_float_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION req_float_sketch_get_rank(req_float_sketch, real) RETURNS double precision
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_rank'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_rank(req_float_sketch, real, boolean) RETURNS double precision
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_rank'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_quantile(req_float_sketch, double precision) RETURNS real
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_quantile'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_quantile(req_float_sketch, double precision, boolean) RETURNS real
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_quantile'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_n(req_float_sketch) RETURNS bigint
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_n'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_to_string(req_float_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_req_float_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_pmf(req_float_sketch, real[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_pmf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_pmf(req_float_sketch, real[], boolean) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_pmf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_cdf(req_float_sketch, real[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_cdf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_cdf(req_float_sketch, real[], boolean) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_cdf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_quantiles(req_float_sketch, double precision[]) RETURNS real[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_quantiles'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_quantiles(req_float_sketch, double precision[], boolean) RETURNS real[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_quantiles'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_histogram(req_float_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_histogram(req_float_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION req_float_sketch_get_histogram(req_float_sketch, int, boolean) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_req_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

DROP FUNCTION req_float_sketch_add_item(internal, real);
DROP FUNCTION req_float_sketch_add_item(internal, real, int);
DROP FUNCTION req_float_sketch_add_item(internal, real, int, boolean);
DROP FUNCTION req_float_sketch_merge(internal, req_float_sketch);
DROP FUNCTION req_float_sketch_merge(internal, req_float_sketch, int);
DROP FUNCTION req_float_sketch_merge(internal, req_float_sketch, int, boolean);
DROP FUNCTION req_float_sketch_from_internal(internal);


CREATE TYPE quantiles_double_sketch;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_in(cstring) RETURNS quantiles_double_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_out(quantiles_double_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE TYPE quantiles_double_sketch (
    INPUT = quantiles_double_sketch_in,
    OUTPUT = quantiles_double_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as quantiles_double_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (quantiles_double_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_build_agg(internal, double precision) RETURNS internal
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_build_agg(internal, double precision, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_build_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_merge_agg(internal, quantiles_double_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_merge_agg(internal, quantiles_double_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_merge_agg'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_serialize(internal) RETURNS bytea
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_deserialize(bytea, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_deserialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_combine(internal, internal) RETURNS internal
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_combine'
    LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_finalize(internal) RETURNS quantiles_double_sketch
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_serialize'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE AGGREGATE quantiles_double_sketch_build(double precision) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_build_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE quantiles_double_sketch_build(double precision, int) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_build_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE quantiles_double_sketch_merge(quantiles_double_sketch) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_merge_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE AGGREGATE quantiles_double_sketch_merge(quantiles_double_sketch, int) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_merge_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_rank(quantiles_double_sketch, double precision) RETURNS double precision
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_rank'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_quantile(quantiles_double_sketch, double precision) RETURNS double precision
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_quantile'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_n(quantiles_double_sketch) RETURNS bigint
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_n'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_to_string(quantiles_double_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_pmf(quantiles_double_sketch, double precision[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_pmf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_cdf(quantiles_double_sketch, double precision[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_cdf'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_quantiles(quantiles_double_sketch, double precision[]) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_quantiles'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_histogram(quantiles_double_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION quantiles_double_sketch_get_histogram(quantiles_double_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_quantiles_double_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;
