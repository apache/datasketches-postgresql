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

CREATE AGGREGATE quantiles_double_sketch_build(double precision) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_build_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE AGGREGATE quantiles_double_sketch_build(double precision, int) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_build_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE AGGREGATE quantiles_double_sketch_merge(quantiles_double_sketch) (
    STYPE = internal,
    SFUNC = quantiles_double_sketch_merge_agg,
    COMBINEFUNC = quantiles_double_sketch_combine,
    SERIALFUNC = quantiles_double_sketch_serialize,
    DESERIALFUNC = quantiles_double_sketch_deserialize, 
    FINALFUNC = quantiles_double_sketch_finalize,
    PARALLEL = SAFE
);

CREATE AGGREGATE quantiles_double_sketch_merge(quantiles_double_sketch, int) (
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
