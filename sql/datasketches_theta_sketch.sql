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

CREATE TYPE theta_sketch;

CREATE OR REPLACE FUNCTION theta_sketch_in(cstring) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_sketch_in'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION theta_sketch_out(theta_sketch) RETURNS cstring
    AS '$libdir/datasketches', 'pg_sketch_out'
    LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE TYPE theta_sketch (
    INPUT = theta_sketch_in,
    OUTPUT = theta_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as theta_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (theta_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

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

CREATE AGGREGATE theta_sketch_build(anyelement) (
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
