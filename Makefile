EXTENSION = datasketches
MODULE_big = datasketches

OBJS = src/base64.o src/common.o \
  src/kll_float_sketch_pg_functions.o src/kll_float_sketch_c_adapter.o \
  src/cpc_sketch_pg_functions.o src/cpc_sketch_c_adapter.o \
  src/frequent_strings_sketch_pg_functions.o src/frequent_strings_sketch_c_adapter.o

# assume a copy or link sketches-core-cpp in the current dir
CORE = sketches-core-cpp
CPC = $(CORE)/cpc/src
OBJS += $(CPC)/cpc_sketch.o $(CPC)/fm85.o $(CPC)/fm85Compression.o $(CPC)/fm85Confidence.o $(CPC)/fm85Merging.o $(CPC)/fm85Util.o $(CPC)/iconEstimator.o $(CPC)/u32Table.o

DATA = sql/datasketches_cpc_sketch.sql sql/datasketches_kll_float_sketch.sql

CXX = g++-8
PG_CPPFLAGS = -I/usr/local/include -I$(CORE)/kll/include -I$(CORE)/common/include -I$(CORE)/cpc/include -I$(CORE)/fi/include
SHLIB_LINK = -lstdc++ -L/usr/local/lib

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
