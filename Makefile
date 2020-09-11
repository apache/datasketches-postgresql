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
  sql/datasketches_theta_sketch.sql \
  sql/datasketches_frequent_strings_sketch.sql \
  sql/datasketches_hll_sketch.sql \
  sql/datasketches_aod_sketch.sql
SQL_INSTALL = sql/$(EXTENSION)--$(EXTVERSION).sql
DATA = $(SQL_INSTALL)

EXTRA_CLEAN = $(SQL_INSTALL)

OBJS = src/global_hooks.o src/base64.o src/common.o \
  src/kll_float_sketch_pg_functions.o src/kll_float_sketch_c_adapter.o \
  src/cpc_sketch_pg_functions.o src/cpc_sketch_c_adapter.o \
  src/theta_sketch_pg_functions.o src/theta_sketch_c_adapter.o \
  src/frequent_strings_sketch_pg_functions.o src/frequent_strings_sketch_c_adapter.o \
  src/hll_sketch_pg_functions.o src/hll_sketch_c_adapter.o \
  src/aod_sketch_pg_functions.o src/aod_sketch_c_adapter.o

# assume a dir or link named "datasketches-cpp" in the current dir
CORE = datasketches-cpp

# assume a dir or link named "boost" in the current dir
BOOST = boost

PG_CPPFLAGS = -fPIC -I/usr/local/include -I$(CORE)/kll/include -I$(CORE)/common/include -I$(CORE)/cpc/include -I$(CORE)/theta/include -I$(CORE)/fi/include -I$(CORE)/hll/include -I$(CORE)/tuple/include -I$(BOOST)
PG_CXXFLAGS = -std=c++11
SHLIB_LINK = -lstdc++ -L/usr/local/lib

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# generate combined sql
$(SQL_INSTALL): $(sort $(SQL_MODULES))
	cat $^ > $@

install: $(SQL_INSTALL)
