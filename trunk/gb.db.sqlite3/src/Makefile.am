COMPONENT = gb.db.sqlite3
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.db.sqlite3.la

gb_db_sqlite3_la_LIBADD = @SQLITE3_LIB@
gb_db_sqlite3_la_LDFLAGS = -module @LD_FLAGS@ @SQLITE3_LDFLAGS@
gb_db_sqlite3_la_CFLAGS = $(AM_CFLAGS)
gb_db_sqlite3_la_CPPFLAGS = @SQLITE3_INC@

gb_db_sqlite3_la_SOURCES = \
 main.h main.c \
 helper.h helper.c \
 gb_buffer.h gb_buffer.c
