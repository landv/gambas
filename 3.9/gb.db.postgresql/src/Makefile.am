COMPONENT = gb.db.postgresql
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.db.postgresql.la

gb_db_postgresql_la_LIBADD = @POSTGRESQL_LIB@
gb_db_postgresql_la_LDFLAGS = -module @LD_FLAGS@ @POSTGRESQL_LDFLAGS@
gb_db_postgresql_la_CPPFLAGS = @POSTGRESQL_INC@

gb_db_postgresql_la_SOURCES = \
  main.h main.c
