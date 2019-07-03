/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <postgres.h>
#include <utils/builtins.h>

#include "base64.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_sketch_in);
PG_FUNCTION_INFO_V1(pg_sketch_out);

Datum pg_sketch_in(PG_FUNCTION_ARGS);
Datum pg_sketch_out(PG_FUNCTION_ARGS);

void pg_error(const char* message);
Datum pg_float4_get_datum(float x);
Datum pg_float8_get_datum(double x);

// cstring to type
Datum pg_sketch_in(PG_FUNCTION_ARGS) {
  // not invoked for nulls
  bytea* decoded;
  char* encoded = PG_GETARG_CSTRING(0);
  const unsigned encoded_length = strlen(encoded);
  const unsigned decoded_length = b64_dec_len(encoded, encoded_length);
  decoded = palloc(VARHDRSZ + decoded_length);
  b64_decode(encoded, encoded_length, VARDATA(decoded));
  SET_VARSIZE(decoded, VARHDRSZ + decoded_length);
  PG_RETURN_BYTEA_P(decoded);
}

// type to cstring
Datum pg_sketch_out(PG_FUNCTION_ARGS) {
  // not invoked for nulls
  char* encoded;
  bytea* bytes = PG_GETARG_BYTEA_P(0);
  const unsigned encoded_length = b64_enc_len(VARSIZE(bytes) - VARHDRSZ);
  encoded = palloc(encoded_length + 1);
  b64_encode(VARDATA(bytes), VARSIZE(bytes) - VARHDRSZ, encoded);
  encoded[encoded_length] = '\0';
  PG_RETURN_CSTRING(encoded);
}

// These are implementations of redirects defined in postgres_h_substitute.h

Datum pg_float4_get_datum(float x) {
  return Float4GetDatum(x);
}

Datum pg_float8_get_datum(double x) {
  return Float8GetDatum(x);
}

void pg_error(const char* message) {
  ereport(
    ERROR,
    (
      errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("%s", message)
    )
  );
}
