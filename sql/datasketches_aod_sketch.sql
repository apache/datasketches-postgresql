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
     LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_out(aod_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE aod_sketch (
    INPUT = aod_sketch_in,
    OUTPUT = aod_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as aod_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (aod_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION aod_sketch_add_item(internal, anyelement, double precision[]) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_add_item(internal, anyelement, double precision[], int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_add_item(internal, anyelement, double precision[], int, real) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_get_estimate(aod_sketch) RETURNS double precision
    AS '$libdir/datasketches', 'pg_aod_sketch_get_estimate'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_get_estimate_and_bounds(aod_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_get_estimate_and_bounds(aod_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_aod_sketch_get_estimate_and_bounds'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_from_internal(internal) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_to_string(aod_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_aod_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_to_string(aod_sketch, boolean) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_aod_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_union_agg(internal, aod_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_agg'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_union_agg(internal, aod_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_agg'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_union_agg(internal, aod_sketch, int, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_union_agg'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection_agg(internal, aod_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection_agg'
    LANGUAGE C IMMUTABLE;    

CREATE OR REPLACE FUNCTION aod_sketch_intersection_agg(internal, aod_sketch, int) RETURNS internal
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection_agg'
    LANGUAGE C IMMUTABLE;    

CREATE OR REPLACE FUNCTION aod_union_get_result(internal) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_union_get_result'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_intersection_get_result(internal) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_intersection_get_result'
    LANGUAGE C STRICT IMMUTABLE;    

CREATE AGGREGATE aod_sketch_build(anyelement, double precision[]) (
    sfunc = aod_sketch_add_item,
    stype = internal,
    finalfunc = aod_sketch_from_internal
);

CREATE AGGREGATE aod_sketch_build(anyelement, double precision[], int) (
    sfunc = aod_sketch_add_item,
    stype = internal,
    finalfunc = aod_sketch_from_internal
);

CREATE AGGREGATE aod_sketch_build(anyelement, double precision[], int, real) (
    sfunc = aod_sketch_add_item,
    stype = internal,
    finalfunc = aod_sketch_from_internal
);

CREATE AGGREGATE aod_sketch_union(aod_sketch) (
    sfunc = aod_sketch_union_agg,
    stype = internal,
    finalfunc = aod_union_get_result
);

CREATE AGGREGATE aod_sketch_union(aod_sketch, int) (
    sfunc = aod_sketch_union_agg,
    stype = internal,
    finalfunc = aod_union_get_result
);

CREATE AGGREGATE aod_sketch_union(aod_sketch, int, int) (
    sfunc = aod_sketch_union_agg,
    stype = internal,
    finalfunc = aod_union_get_result
);

CREATE AGGREGATE aod_sketch_intersection(aod_sketch) (
    sfunc = aod_sketch_intersection_agg,
    stype = internal,
    finalfunc = aod_intersection_get_result
);

CREATE AGGREGATE aod_sketch_intersection(aod_sketch, int) (
    sfunc = aod_sketch_intersection_agg,
    stype = internal,
    finalfunc = aod_intersection_get_result
);

CREATE OR REPLACE FUNCTION aod_sketch_union(aod_sketch, aod_sketch) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_union'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_union(aod_sketch, aod_sketch, int) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_union'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection(aod_sketch, aod_sketch) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_intersection(aod_sketch, aod_sketch, int) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_intersection'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_a_not_b(aod_sketch, aod_sketch) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_a_not_b'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_a_not_b(aod_sketch, aod_sketch, int) RETURNS aod_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_a_not_b'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_to_kll_float_sketch(aod_sketch, int) RETURNS kll_float_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_to_kll_float_sketch'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION aod_sketch_to_kll_float_sketch(aod_sketch, int, int) RETURNS kll_float_sketch
    AS '$libdir/datasketches', 'pg_aod_sketch_to_kll_float_sketch'
    LANGUAGE C STRICT IMMUTABLE;
