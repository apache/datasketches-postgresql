/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include <postgres.h>
#include <utils/builtins.h>

#include "base64.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_sketch_in);
PG_FUNCTION_INFO_V1(pg_sketch_out);

Datum pg_sketch_in(PG_FUNCTION_ARGS);
Datum pg_sketch_out(PG_FUNCTION_ARGS);

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
  PG_RETURN_CSTRING(encoded); // who should free it?
}
