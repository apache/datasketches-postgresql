CREATE TYPE kll_float_sketch;

CREATE FUNCTION kll_float_sketch_in(cstring) RETURNS kll_float_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_out(kll_float_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_recv(internal) RETURNS kll_float_sketch
     AS '$libdir/datasketches', 'pg_kll_float_sketch_recv'
     LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_send(kll_float_sketch) RETURNS bytea
     AS '$libdir/datasketches', 'pg_kll_float_sketch_send'
     LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE kll_float_sketch (
    INPUT = kll_float_sketch_in,
    OUTPUT = kll_float_sketch_out,
    STORAGE = EXTERNAL,
    RECEIVE = kll_float_sketch_recv,
    SEND = kll_float_sketch_send
);

CREATE CAST (bytea as kll_float_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (kll_float_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION kll_float_sketch_add_item_default(internal, real) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_add_item_default'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_rank(kll_float_sketch, real) RETURNS double precision
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_rank'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_quantile(kll_float_sketch, double precision) RETURNS real
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_quantile'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_to_string(kll_float_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_kll_float_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_merge_default(internal, kll_float_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_kll_float_sketch_merge_default'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_from_internal(internal) RETURNS kll_float_sketch
    AS '$libdir/datasketches', 'pg_kll_float_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE;

CREATE AGGREGATE kll_float_sketch_build(real) (
    sfunc = kll_float_sketch_add_item_default,
    stype = internal,
    finalfunc = kll_float_sketch_from_internal
);

CREATE AGGREGATE kll_float_sketch_merge(kll_float_sketch) (
    sfunc = kll_float_sketch_merge_default,
    stype = internal,
    finalfunc = kll_float_sketch_from_internal
);
