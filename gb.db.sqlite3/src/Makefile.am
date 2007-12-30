INCLUDES = @SQLITE3_INC@

gblib_LTLIBRARIES = gb.db.sqlite3.la

gb_db_sqlite3_la_LIBADD = @SQLITE3_LIB@
gb_db_sqlite3_la_LDFLAGS = -module @LD_FLAGS@

gb_db_sqlite3_la_SOURCES = \
 main.h main.cpp dataset.h dataset.cpp qry_dat.cpp qry_dat.h sqlitedataset.h \
 sqlitedataset.cpp stringhelper.h stringhelper.cpp
