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

#include <assert.h>
#include "base64.h"

static const char bin_to_b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char b64_to_bin[128] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,
   0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,
   0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0
};

// with full padding
// produces exactly b64_enc_len(srclen) chars
// no \0 termination
void b64_encode(const char* src, unsigned srclen, char* dst) {
  unsigned buf = 0;
  int pos = 2;

  while (srclen--) {
    buf |= (unsigned char)*(src++) << ((pos--) << 3);

    if (pos < 0) {
      *dst++ = bin_to_b64[(buf >> 18) & 0x3f];
      *dst++ = bin_to_b64[(buf >> 12) & 0x3f];
      *dst++ = bin_to_b64[(buf >> 6) & 0x3f];
      *dst++ = bin_to_b64[buf & 0x3f];
      pos = 2;
      buf = 0;
    }
  }
  if (pos != 2) {
    *dst++ = bin_to_b64[(buf >> 18) & 0x3f];
    *dst++ = bin_to_b64[(buf >> 12) & 0x3f];
    *dst++ = (pos == 0) ? bin_to_b64[(buf >> 6) & 0x3f] : '=';
    *dst++ = '=';
  }
}

// supports no padding or partial padding (one =)
// ignores invalid chars (fills zeros instead)
// produces exactly b64_dec_len(src, srclen) bytes
void b64_decode(const char* src, unsigned srclen, char* dst) {
  unsigned buf = 0;
  char c;
  int bits = 0;
  int pos = 0;
  int pad = 0;

  while (srclen--) {
    c = *src++;
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
    bits = 0;
    if (c != '=') {
      if (c > 0 && c < 127) bits = b64_to_bin[(int)c];
    } else {
      pad++;
    }
    buf = (buf << 6) | bits;
    pos++;
    if (pos == 4) {
      *dst++ = (buf >> 16) & 0xff;
      if (pad < 2) *dst++ = (buf >> 8) & 0xff;
      if (pad == 0) *dst++ = buf & 0xff;
      buf = 0;
      pos = 0;
    }
  }
  // no padding or partial padding. pos must be 2 or 3
  if (pos == 2) {
    *dst++ = (buf >> 4) & 0xff;
  } else if (pos == 3) {
    *dst++ = (buf >> 10) & 0xff;
    if (pad == 0) *dst++ = (buf >> 2) & 0xff;
  }
}

// with padding
unsigned b64_enc_len(unsigned srclen) {
  return ((srclen + 2) / 3) * 4;
}

unsigned b64_dec_len(const char* src, unsigned srclen) {
  unsigned pad = 0;
  if (srclen > 0 && src[srclen - 1] == '=') pad++;
  if (srclen > 1 && src[srclen - 2] == '=') pad++;
  return ((srclen * 3) >> 2) - pad;
}
