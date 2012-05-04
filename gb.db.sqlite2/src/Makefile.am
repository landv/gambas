COMPONENT = gb.db.sqlite2
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.db.sqlite2.la

gb_db_sqlite2_la_LIBADD = @SQLITE2_LIB@
gb_db_sqlite2_la_LDFLAGS = -module @LD_FLAGS@ @SQLITE2_LDFLAGS@
gb_db_sqlite2_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions
gb_db_sqlite2_la_CPPFLAGS = @SQLITE2_INC@

gb_db_sqlite2_la_SOURCES = \
 main.h main.cpp dataset.h dataset.cpp qry_dat.cpp qry_dat.h sqlitedataset.h \
 sqlitedataset.cpp stringhelper.h stringhelper.cpp
