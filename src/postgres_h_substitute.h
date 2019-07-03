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

// There is some problem compiling C++ code using GCC 4.8.5 (standard on current RHEL)
// with postgres.h included. This is to avoid including postgres.h

#ifndef POSTGRES_H_SUBSTITUTE
#define POSTGRES_H_SUBSTITUTE

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Datum;
extern Datum pg_float4_get_datum(float x);
extern Datum pg_float8_get_datum(double x);

extern void pg_error(const char* message);

// from postgresql c.h to hint compiler and avoid warnings
#if defined(HAVE__BUILTIN_UNREACHABLE) && !defined(USE_ASSERT_CHECKING)
#define pg_unreachable() __builtin_unreachable()
#elif defined(_MSC_VER) && !defined(USE_ASSERT_CHECKING)
#define pg_unreachable() __assume(0)
#else
#define pg_unreachable() abort()
#endif

#ifdef __cplusplus
}
#endif

#endif
