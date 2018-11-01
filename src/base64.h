/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef _BASE64_H_
#define _BASE64_H_

void b64_encode(const char *src, unsigned srclen, char *dst);
void b64_decode(const char *src, unsigned srclen, char *dst);
unsigned b64_enc_len(unsigned srclen);
unsigned b64_dec_len(const char* src, unsigned srclen);

#endif // _BASE64_H_
