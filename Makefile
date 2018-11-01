EXTENSION = datasketches
MODULE_big = datasketches

OBJS = src/base64.o src/common.o \
  src/kll_float_sketch_pg_functions.o src/kll_float_sketch_c_adapter.o \
  src/cpc_sketch_pg_functions.o src/cpc_sketch_c_adapter.o

DATA = sql/datasketches_cpc_sketch.sql sql/datasketches_kll_float_sketch.sql

CXX = g++-8
PG_CPPFLAGS = -I/usr/local/include -I../sketches-core-cpp/kll/include -I../sketches-core-cpp/common/include -I../sketches-core-cpp/cpc/include
SHLIB_LINK = -L../sketches-core-cpp/lib -ldatasketches -lstdc++ -L/usr/local/lib -lnettle

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
