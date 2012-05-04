COMPONENT = gb.db.sqlite3
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.db.sqlite3.la

gb_db_sqlite3_la_LIBADD = @SQLITE3_LIB@
gb_db_sqlite3_la_LDFLAGS = -module @LD_FLAGS@ @SQLITE3_LDFLAGS@
gb_db_sqlite3_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions
gb_db_sqlite3_la_CPPFLAGS = @SQLITE3_INC@

gb_db_sqlite3_la_SOURCES = \
 main.h main.cpp dataset.h dataset.cpp qry_dat.cpp qry_dat.h sqlitedataset.h \
 sqlitedataset.cpp stringhelper.h stringhelper.cpp
