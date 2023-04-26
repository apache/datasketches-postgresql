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

CREATE TYPE cpc_sketch;

CREATE OR REPLACE FUNCTION cpc_sketch_in(cstring) RETURNS cpc_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION cpc_sketch_out(cpc_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE TYPE cpc_sketch (
    INPUT = cpc_sketch_in,
    OUTPUT = cpc_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as cpc_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (cpc_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

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
