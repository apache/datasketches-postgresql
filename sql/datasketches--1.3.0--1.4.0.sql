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

CREATE OR REPLACE FUNCTION kll_float_sketch_get_histogram(kll_float_sketch) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION kll_float_sketch_get_histogram(kll_float_sketch, int) RETURNS double precision[]
    AS '$libdir/datasketches', 'pg_kll_float_sketch_get_histogram'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION theta_sketch_intersection_agg(internal, theta_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_theta_sketch_intersection_agg'
    LANGUAGE C IMMUTABLE;
 
CREATE OR REPLACE FUNCTION theta_intersection_get_result(internal) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_intersection_get_result'
    LANGUAGE C STRICT IMMUTABLE;

CREATE AGGREGATE theta_sketch_intersection(theta_sketch) (
    sfunc = theta_sketch_intersection_agg,
    stype = internal,
    finalfunc = theta_intersection_get_result
);

CREATE OR REPLACE FUNCTION theta_sketch_union(theta_sketch, theta_sketch) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_union'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION theta_sketch_union(theta_sketch, theta_sketch, int) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_union'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION theta_sketch_intersection(theta_sketch, theta_sketch) RETURNS theta_sketch
    AS '$libdir/datasketches', 'pg_theta_sketch_intersection'
    LANGUAGE C IMMUTABLE;
