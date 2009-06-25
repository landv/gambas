COMPONENT = gb.db.postgresql
include $(top_srcdir)/component.am

INCLUDES = @POSTGRESQL_INC@

gblib_LTLIBRARIES = gb.db.postgresql.la

gb_db_postgresql_la_LIBADD = @POSTGRESQL_LIB@
gb_db_postgresql_la_LDFLAGS = -module @LD_FLAGS@

gb_db_postgresql_la_SOURCES = \
  main.h main.c
