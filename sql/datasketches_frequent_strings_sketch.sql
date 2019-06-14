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

CREATE TYPE frequent_strings_sketch;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_in(cstring) RETURNS frequent_strings_sketch
     AS '$libdir/datasketches', 'pg_sketch_in'
     LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_out(frequent_strings_sketch) RETURNS cstring
     AS '$libdir/datasketches', 'pg_sketch_out'
     LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE frequent_strings_sketch (
    INPUT = frequent_strings_sketch_in,
    OUTPUT = frequent_strings_sketch_out,
    STORAGE = EXTERNAL
);

CREATE CAST (bytea as frequent_strings_sketch) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (frequent_strings_sketch as bytea) WITHOUT FUNCTION AS ASSIGNMENT;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_add_item(internal, int, varchar) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_add_item(internal, int, varchar, bigint) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_add_item'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_merge(internal, int, frequent_strings_sketch) RETURNS internal
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_merge'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_from_internal(internal) RETURNS frequent_strings_sketch
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_from_internal'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_to_string(frequent_strings_sketch) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_to_string(frequent_strings_sketch, boolean) RETURNS TEXT
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_to_string'
    LANGUAGE C STRICT IMMUTABLE;

CREATE AGGREGATE frequent_strings_sketch_build(int, varchar) (
    sfunc = frequent_strings_sketch_add_item,
    stype = internal,
    finalfunc = frequent_strings_sketch_from_internal
);

CREATE AGGREGATE frequent_strings_sketch_build(int, varchar, bigint) (
    sfunc = frequent_strings_sketch_add_item,
    stype = internal,
    finalfunc = frequent_strings_sketch_from_internal
);

CREATE AGGREGATE frequent_strings_sketch_merge(int, frequent_strings_sketch) (
    sfunc = frequent_strings_sketch_merge,
    stype = internal,
    finalfunc = frequent_strings_sketch_from_internal
);

CREATE TYPE frequent_strings_sketch_row AS (str varchar, estimate bigint, lower_bound bigint, upper_bound bigint);

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_positives(frequent_strings_sketch)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_positives'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_positives(frequent_strings_sketch, bigint)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_positives'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_negatives'
    LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch, bigint)
    RETURNS setof frequent_strings_sketch_row
    AS '$libdir/datasketches', 'pg_frequent_strings_sketch_result_no_false_negatives'
    LANGUAGE C STRICT IMMUTABLE;

