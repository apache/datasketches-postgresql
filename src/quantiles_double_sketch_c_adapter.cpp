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

#include "quantiles_double_sketch_c_adapter.h"
#include "allocator.h"
#include "postgres_h_substitute.h"

#include <quantiles_sketch.hpp>

using quantiles_double_sketch = datasketches::quantiles_sketch<double, std::less<double>, palloc_allocator<double>>;

void* quantiles_double_sketch_new(unsigned k) {
  try {
    return new (palloc(sizeof(quantiles_double_sketch))) quantiles_double_sketch(k);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void quantiles_double_sketch_delete(void* sketchptr) {
  try {
    static_cast<quantiles_double_sketch*>(sketchptr)->~quantiles_double_sketch();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void quantiles_double_sketch_update(void* sketchptr, double value) {
  try {
    static_cast<quantiles_double_sketch*>(sketchptr)->update(value);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void quantiles_double_sketch_merge(void* sketchptr1, const void* sketchptr2) {
  try {
    static_cast<quantiles_double_sketch*>(sketchptr1)->merge(*static_cast<const quantiles_double_sketch*>(sketchptr2));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

double quantiles_double_sketch_get_rank(const void* sketchptr, double value) {
  try {
    return static_cast<const quantiles_double_sketch*>(sketchptr)->get_rank(value);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

double quantiles_double_sketch_get_quantile(const void* sketchptr, double rank) {
  try {
    return static_cast<const quantiles_double_sketch*>(sketchptr)->get_quantile(rank);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

unsigned long long quantiles_double_sketch_get_n(const void* sketchptr) {
  try {
    return static_cast<const quantiles_double_sketch*>(sketchptr)->get_n();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

char* quantiles_double_sketch_to_string(const void* sketchptr) {
  try {
    auto str = static_cast<const quantiles_double_sketch*>(sketchptr)->to_string();
    const size_t len = str.length() + 1;
    char* buffer = (char*) palloc(len);
    strncpy(buffer, str.c_str(), len);
    return buffer;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

ptr_with_size quantiles_double_sketch_serialize(const void* sketchptr, unsigned header_size) {
  try {
    ptr_with_size p;
    auto bytes = new (palloc(sizeof(quantiles_double_sketch::vector_bytes))) quantiles_double_sketch::vector_bytes(
      static_cast<const quantiles_double_sketch*>(sketchptr)->serialize(header_size)
    );
    p.ptr = bytes->data();
    p.size = bytes->size();
    return p;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* quantiles_double_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    return new (palloc(sizeof(quantiles_double_sketch))) quantiles_double_sketch(quantiles_double_sketch::deserialize(buffer, length));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

unsigned quantiles_double_sketch_get_serialized_size_bytes(const void* sketchptr) {
  try {
    return static_cast<const quantiles_double_sketch*>(sketchptr)->get_serialized_size_bytes();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* quantiles_double_sketch_get_pmf_or_cdf(const void* sketchptr, const double* split_points, unsigned num_split_points, bool is_cdf, bool scale) {
  try {
    auto array = is_cdf ?
      static_cast<const quantiles_double_sketch*>(sketchptr)->get_CDF(split_points, num_split_points) :
      static_cast<const quantiles_double_sketch*>(sketchptr)->get_PMF(split_points, num_split_points);
    Datum* pmf = (Datum*) palloc(sizeof(Datum) * (num_split_points + 1));
    const uint64_t n = static_cast<const quantiles_double_sketch*>(sketchptr)->get_n();
    for (unsigned i = 0; i < num_split_points + 1; i++) {
      if (scale) {
        pmf[i] = pg_float8_get_datum(array[i] * n);
      } else {
        pmf[i] = pg_float8_get_datum(array[i]);
      }
    }
    return pmf;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* quantiles_double_sketch_get_quantiles(const void* sketchptr, const double* fractions, unsigned num_fractions) {
  try {
    Datum* quantiles = (Datum*) palloc(sizeof(Datum) * num_fractions);
    for (unsigned i = 0; i < num_fractions; i++) {
      quantiles[i] = pg_float8_get_datum(static_cast<const quantiles_double_sketch*>(sketchptr)->get_quantile(fractions[i]));
    }
    return quantiles;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
