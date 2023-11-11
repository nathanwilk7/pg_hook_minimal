MODULE_big = pg_hook_minimal
OBJS = pg_hook_minimal.o
ifdef USE_PGXS
# note that is hardcoded for the default multipass VM dir structure and install location
PG_CONFIG = /usr/local/pgsql/bin/pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_hook_minimal
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif