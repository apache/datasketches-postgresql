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

#include "aod_sketch_c_adapter.h"
#include "allocator.h"
#include "postgres_h_substitute.h"
#include "kll_float_sketch_c_adapter.h"

#include <array_of_doubles_sketch.hpp>
#include <array_of_doubles_union.hpp>
#include <array_of_doubles_intersection.hpp>
#include <array_of_doubles_a_not_b.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/math/distributions/students_t.hpp>

using vector_double = std::vector<double, palloc_allocator<double>>;

using update_aod_sketch_pg = datasketches::update_array_of_doubles_sketch_alloc<palloc_allocator<double>>;
using compact_aod_sketch_pg = datasketches::compact_array_of_doubles_sketch_alloc<palloc_allocator<double>>;
using aod_union_pg = datasketches::array_of_doubles_union_alloc<palloc_allocator<double>>;
// using the union policy in the intersection since this is how it is done in Druid
using aod_intersection_pg = datasketches::array_of_doubles_intersection<datasketches::array_of_doubles_union_policy_alloc<palloc_allocator<double>>, palloc_allocator<double>>;
using aod_a_not_b_pg = datasketches::array_of_doubles_a_not_b_alloc<palloc_allocator<double>>;

std::ostream& operator<<(std::ostream& os, const vector_double& v) {
  os << "(";
  for (size_t i = 0; i < v.size(); ++i) {
    if (i != 0) os << ", ";
    os << v[i];
  }
  os << ")";
  return os;
}

void* aod_sketch_new(unsigned num_values) {
  try {
    return new (palloc(sizeof(update_aod_sketch_pg))) update_aod_sketch_pg(update_aod_sketch_pg::builder(num_values).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_sketch_new_lgk(unsigned num_values, unsigned lg_k) {
  try {
    return new (palloc(sizeof(update_aod_sketch_pg))) update_aod_sketch_pg(update_aod_sketch_pg::builder(num_values).set_lg_k(lg_k).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_sketch_new_lgk_p(unsigned num_values, unsigned lg_k, float p) {
  try {
    return new (palloc(sizeof(update_aod_sketch_pg))) update_aod_sketch_pg(update_aod_sketch_pg::builder(num_values).set_lg_k(lg_k).set_p(p).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void update_aod_sketch_delete(void* sketchptr) {
  try {
    static_cast<update_aod_sketch_pg*>(sketchptr)->~update_aod_sketch_pg();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void compact_aod_sketch_delete(void* sketchptr) {
  try {
    static_cast<compact_aod_sketch_pg*>(sketchptr)->~compact_aod_sketch_pg();
    pfree(sketchptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void aod_sketch_update(void* sketchptr, const void* data, unsigned length, const double* values) {
  try {
    static_cast<update_aod_sketch_pg*>(sketchptr)->update(data, length, values);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* aod_sketch_compact(void* sketchptr) {
  try {
    auto newptr = new (palloc(sizeof(compact_aod_sketch_pg))) compact_aod_sketch_pg(static_cast<update_aod_sketch_pg*>(sketchptr)->compact());
    static_cast<update_aod_sketch_pg*>(sketchptr)->~update_aod_sketch_pg();
    pfree(sketchptr);
    return newptr;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

double update_aod_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const update_aod_sketch_pg*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

double compact_aod_sketch_get_estimate(const void* sketchptr) {
  try {
    return static_cast<const compact_aod_sketch_pg*>(sketchptr)->get_estimate();
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* aod_sketch_get_estimate_and_bounds(const void* sketchptr, unsigned num_std_devs) {
  try {
    Datum* est_and_bounds = (Datum*) palloc(sizeof(Datum) * 3);
    est_and_bounds[0] = pg_float8_get_datum(static_cast<const compact_aod_sketch_pg*>(sketchptr)->get_estimate());
    est_and_bounds[1] = pg_float8_get_datum(static_cast<const compact_aod_sketch_pg*>(sketchptr)->get_lower_bound(num_std_devs));
    est_and_bounds[2] = pg_float8_get_datum(static_cast<const compact_aod_sketch_pg*>(sketchptr)->get_upper_bound(num_std_devs));
    return est_and_bounds;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

char* aod_sketch_to_string(const void* sketchptr, bool print_entries) {
  try {
    auto str = static_cast<const compact_aod_sketch_pg*>(sketchptr)->to_string(print_entries);
    const size_t len = str.length() + 1;
    char* buffer = (char*) palloc(len);
    strncpy(buffer, str.c_str(), len);
    return buffer;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

ptr_with_size aod_sketch_serialize(const void* sketchptr, unsigned header_size) {
  try {
    ptr_with_size p;
    auto bytes = new (palloc(sizeof(compact_aod_sketch_pg::vector_bytes))) compact_aod_sketch_pg::vector_bytes(
      static_cast<const compact_aod_sketch_pg*>(sketchptr)->serialize(header_size)
    );
    p.ptr = bytes->data();
    p.size = bytes->size();
    return p;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_sketch_deserialize(const char* buffer, unsigned length) {
  try {
    return new (palloc(sizeof(compact_aod_sketch_pg))) compact_aod_sketch_pg(compact_aod_sketch_pg::deserialize(buffer, length));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_union_new(unsigned num_values) {
  try {
    return new (palloc(sizeof(aod_union_pg))) aod_union_pg(aod_union_pg::builder(num_values).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_union_new_lgk(unsigned num_values, unsigned lg_k) {
  try {
    return new (palloc(sizeof(aod_union_pg))) aod_union_pg(aod_union_pg::builder(num_values).set_lg_k(lg_k).build());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void aod_union_delete(void* unionptr) {
  try {
    static_cast<aod_union_pg*>(unionptr)->~aod_union_pg();
    pfree(unionptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void aod_union_update(void* unionptr, const void* sketchptr) {
  try {
    static_cast<aod_union_pg*>(unionptr)->update(std::move(*static_cast<const compact_aod_sketch_pg*>(sketchptr)));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* aod_union_get_result(const void* unionptr) {
  try {
    return new (palloc(sizeof(compact_aod_sketch_pg))) compact_aod_sketch_pg(static_cast<const aod_union_pg*>(unionptr)->get_result());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_intersection_new(unsigned num_values) {
  try {
    return new (palloc(sizeof(aod_intersection_pg))) aod_intersection_pg(datasketches::DEFAULT_SEED, num_values);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void aod_intersection_delete(void* interptr) {
  try {
    static_cast<aod_intersection_pg*>(interptr)->~aod_intersection_pg();
    pfree(interptr);
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void aod_intersection_update(void* interptr, const void* sketchptr) {
  try {
    static_cast<aod_intersection_pg*>(interptr)->update(*static_cast<const compact_aod_sketch_pg*>(sketchptr));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
}

void* aod_intersection_get_result(const void* interptr) {
  try {
    return new (palloc(sizeof(compact_aod_sketch_pg))) compact_aod_sketch_pg(static_cast<const aod_intersection_pg*>(interptr)->get_result());
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_a_not_b(const void* sketchptr1, const void* sketchptr2) {
  try {
    aod_a_not_b_pg a_not_b;
    return new (palloc(sizeof(compact_aod_sketch_pg))) compact_aod_sketch_pg(a_not_b.compute(
      *static_cast<const compact_aod_sketch_pg*>(sketchptr1),
      *static_cast<const compact_aod_sketch_pg*>(sketchptr2)
    ));
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

void* aod_sketch_to_kll_float_sketch(const void* sketchptr, unsigned column_index, unsigned k) {
  try {
    auto kllptr = kll_float_sketch_new(k);
    for (const auto& entry: *static_cast<const compact_aod_sketch_pg*>(sketchptr)) {
      kll_float_sketch_update(kllptr, entry.second[column_index]);
    }
    return kllptr;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

double t_test_unequal_sd(double m1, double v1, uint64_t n1, double m2, double v2, uint64_t n2) {
  double degrees_of_freedom = v1 / n1 + v2 / n2;
  degrees_of_freedom *= degrees_of_freedom;
  double t1 = v1 / n1;
  t1 *= t1;
  t1 /= (n1 - 1);
  double t2 = v2 / n2;
  t2 *= t2;
  t2 /= (n2 - 1);
  degrees_of_freedom /= (t1 + t2);
  double t_stat = (m1 - m2) / sqrt(v1 / n1 + v2 / n2);
  using boost::math::students_t;
  students_t distribution(degrees_of_freedom);
  return 2 * cdf(complement(distribution, fabs(t_stat))); // double to match 2-sided test in Java (commons-math3)
}

Datum* aod_sketch_students_t_test(const void* sketchptr1, const void* sketchptr2, unsigned* arr_len_out) {
  try {
    const auto& sketch1 = *static_cast<const compact_aod_sketch_pg*>(sketchptr1);
    const auto& sketch2 = *static_cast<const compact_aod_sketch_pg*>(sketchptr2);
    if (sketch1.get_num_values() != sketch2.get_num_values()) pg_error("aod_sketch_students_t_test: number of values mismatch");
    unsigned num_values = sketch1.get_num_values();
    Datum* p_values = (Datum*) palloc(sizeof(Datum) * num_values);
    *arr_len_out = num_values;

    using namespace boost::accumulators;
    using Accum = accumulator_set<double, stats<tag::mean, tag::variance>>;

    std::vector<Accum, palloc_allocator<Accum>> stats1(num_values);
    for (const auto& entry: sketch1) {
      for (unsigned i = 0; i < num_values; ++i) stats1[i](entry.second[i]);
    }

    std::vector<Accum, palloc_allocator<Accum>> stats2(num_values);
    for (const auto& entry: sketch2) {
      for (unsigned i = 0; i < num_values; ++i) stats2[i](entry.second[i]);
    }

    for (unsigned i = 0; i < num_values; ++i) {
      p_values[i] = pg_float8_get_datum(t_test_unequal_sd(
        mean(stats1[i]), variance(stats1[i]), sketch1.get_num_retained(),
        mean(stats2[i]), variance(stats2[i]), sketch2.get_num_retained()
      ));
    }

    return p_values;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* aod_sketch_to_means(const void* sketchptr, unsigned* arr_len_out) {
  try {
    const auto& sketch = *static_cast<const compact_aod_sketch_pg*>(sketchptr);
    unsigned num_values = sketch.get_num_values();
    Datum* means = (Datum*) palloc(sizeof(Datum) * num_values);
    *arr_len_out = num_values;

    using namespace boost::accumulators;
    using Accum = accumulator_set<double, stats<tag::mean>>;

    std::vector<Accum, palloc_allocator<Accum>> stats(num_values);
    for (const auto& entry: sketch) {
      for (unsigned i = 0; i < num_values; ++i) stats[i](entry.second[i]);
    }

    for (unsigned i = 0; i < num_values; ++i) {
      means[i] = pg_float8_get_datum(mean(stats[i]));
    }

    return means;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}

Datum* aod_sketch_to_variances(const void* sketchptr, unsigned* arr_len_out) {
  try {
    const auto& sketch = *static_cast<const compact_aod_sketch_pg*>(sketchptr);
    unsigned num_values = sketch.get_num_values();
    Datum* variances = (Datum*) palloc(sizeof(Datum) * num_values);
    *arr_len_out = num_values;

    using namespace boost::accumulators;
    using Accum = accumulator_set<double, stats<tag::variance>>;

    std::vector<Accum, palloc_allocator<Accum>> stats(num_values);
    for (const auto& entry: sketch) {
      for (unsigned i = 0; i < num_values; ++i) stats[i](entry.second[i]);
    }

    for (unsigned i = 0; i < num_values; ++i) {
      variances[i] = pg_float8_get_datum(variance(stats[i]));
    }

    return variances;
  } catch (std::exception& e) {
    pg_error(e.what());
  }
  pg_unreachable();
}
