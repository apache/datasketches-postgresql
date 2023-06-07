# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

EXTENSION = datasketches
EXTVERSION = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
MODULE_big = datasketches

SQL_MODULES = sql/datasketches_cpc_sketch.sql \
  sql/datasketches_kll_float_sketch.sql \
  sql/datasketches_kll_double_sketch.sql \
  sql/datasketches_theta_sketch.sql \
  sql/datasketches_frequent_strings_sketch.sql \
  sql/datasketches_hll_sketch.sql \
  sql/datasketches_aod_sketch.sql \
  sql/datasketches_req_float_sketch.sql \
  sql/datasketches_quantiles_double_sketch.sql
SQL_INSTALL = sql/$(EXTENSION)--$(EXTVERSION).sql
DATA = $(SQL_INSTALL) \
  sql/datasketches--1.3.0--1.4.0.sql \
  sql/datasketches--1.4.0--1.5.0.sql \
  sql/datasketches--1.5.0--1.6.0.sql \
  sql/datasketches--1.3.0--1.6.0.sql

EXTRA_CLEAN = $(SQL_INSTALL)

OBJS = src/global_hooks.o src/base64.o src/common.o \
  src/kll_float_sketch_pg_functions.o src/kll_float_sketch_c_adapter.o \
  src/kll_double_sketch_pg_functions.o src/kll_double_sketch_c_adapter.o \
  src/cpc_sketch_pg_functions.o src/cpc_sketch_c_adapter.o \
  src/theta_sketch_pg_functions.o src/theta_sketch_c_adapter.o \
  src/frequent_strings_sketch_pg_functions.o src/frequent_strings_sketch_c_adapter.o \
  src/hll_sketch_pg_functions.o src/hll_sketch_c_adapter.o \
  src/aod_sketch_pg_functions.o src/aod_sketch_c_adapter.o \
  src/req_float_sketch_pg_functions.o src/req_float_sketch_c_adapter.o \
  src/quantiles_double_sketch_pg_functions.o src/quantiles_double_sketch_c_adapter.o

# assume a dir or link named "datasketches-cpp" in the current dir
CORE = datasketches-cpp

# assume a dir or link named "boost" in the current dir
BOOST = boost
BOOST_VER = 1.75.0
BOOST_FILE = boost_$(subst .,_,$(BOOST_VER))
BOOST_URL = https://boostorg.jfrog.io/artifactory/main/release/$(BOOST_VER)/source/$(BOOST_FILE).zip

PG_CPPFLAGS = -fPIC -I/usr/local/include -I$(BOOST) -I$(CORE)/common/include \
  -I$(CORE)/kll/include \
  -I$(CORE)/cpc/include \
  -I$(CORE)/theta/include \
  -I$(CORE)/fi/include \
  -I$(CORE)/hll/include \
  -I$(CORE)/tuple/include \
  -I$(CORE)/req/include \
  -I$(CORE)/quantiles/include
PG_CXXFLAGS = -std=c++11
SHLIB_LINK = -lstdc++ -L/usr/local/lib

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# fix LLVM JIT compilation error
ifeq ($(with_llvm), yes)
	COMPILE.cxx.bc = $(CLANG) -xc++ -Wno-ignored-attributes $(BITCODE_CXXFLAGS) $(CPPFLAGS) -emit-llvm -c
endif

%.bc : %.cpp
	$(COMPILE.cxx.bc) -o $@ $<
	if [ "$(with_llvm)" = "yes" ]; then $(LLVM_BINPATH)/opt -module-summary -f $@ -o $@; fi

# generate combined sql
$(SQL_INSTALL): $(SQL_MODULES)
	cat $^ > $@

install: $(SQL_INSTALL)

boost:
	wget $(BOOST_URL)
	unzip $(BOOST_FILE).zip
	ln -s $(BOOST_FILE) boost

tests:
	@for t in test/*.sql; do psql test -f $$t; done
