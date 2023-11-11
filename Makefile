MODULE_big = pg_hook_minimal
OBJS = pg_hook_minimal.o
ifdef USE_PGXS
#PG_CONFIG = pg_config # TODO try this with multipass setup? explain how i fixed if not
PG_CONFIG = /usr/local/pgsql/bin/pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_hook_minimal
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif