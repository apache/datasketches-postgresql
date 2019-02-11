-- Copyright 2019, Verizon Media.
-- Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.

CREATE TYPE cpc_sketch;

CREATE OR REPLACE FUNCTION cpc_sketch_in(cstring) RETURNS cpc_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_out(cpc_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE cpc_sketch (
    INPUT = cpc_sketch_in,
    OUTPUT = cpc_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as cpc_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (cpc_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION cpc_sketch_add_item(internal, anyelement) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_add_item(internal, anyelement, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_get_estimate(cpc_sketch) RETURNS double precision
    AS '$libdir/datasketches', 'pg_cpc_sketch_get_estimate'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_from_internal(internal) RETURNS cpc_sketch
    AS '$libdir/datasketches', 'pg_cpc_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_get_estimate_from_internal(internal) RETURNS double precision
    AS '$libdir/datasketches', 'pg_cpc_sketch_get_estimate_from_internal'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_to_string(cpc_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_cpc_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_merge(internal, cpc_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_merge'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_sketch_merge(internal, cpc_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_cpc_sketch_merge'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION cpc_union_get_result(internal) RETURNS cpc_sketch
    AS '$libdir/datasketches', 'pg_cpc_union_get_result'
    LANGUAGE C STRICT IMMUTABLE;

CREATE AGGREGATE cpc_sketch_distinct(anyelement) (
    sfunc = cpc_sketch_add_item,
    stype = internal,
    finalfunc = cpc_sketch_get_estimate_from_internal
);

CREATE AGGREGATE cpc_sketch_distinct(anyelement, int) (
    sfunc = cpc_sketch_add_item,
    stype = internal,
    finalfunc = cpc_sketch_get_estimate_from_internal
);

CREATE AGGREGATE cpc_sketch_build(anyelement) (
    sfunc = cpc_sketch_add_item,
    stype = internal,
    finalfunc = cpc_sketch_from_internal
);

CREATE AGGREGATE cpc_sketch_build(anyelement, int) (
    sfunc = cpc_sketch_add_item,
    stype = internal,
    finalfunc = cpc_sketch_from_internal
);

CREATE AGGREGATE cpc_sketch_merge(cpc_sketch) (
    sfunc = cpc_sketch_merge,
    stype = internal,
    finalfunc = cpc_union_get_result
);

CREATE AGGREGATE cpc_sketch_merge(cpc_sketch, int) (
    sfunc = cpc_sketch_merge,
    stype = internal,
    finalfunc = cpc_union_get_result
);
