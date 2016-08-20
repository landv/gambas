COMPONENT = gb.db.odbc
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.db.odbc.la

gb_db_odbc_la_LIBADD = @ODBC_LIB@
gb_db_odbc_la_LDFLAGS = -module @LD_FLAGS@ @ODBC_LDFLAGS@
gb_db_odbc_la_CPPFLAGS = @ODBC_INC@

gb_db_odbc_la_SOURCES = \
  main.h main.c
