COMPONENT = gb.db.firebird
include $(top_srcdir)/component.am

##INCLUDES = -I./ibpp -I./ibpp/fbheaders
INCLUDES = -I./ibpp/core
EXTRA_DIST = ibpp

gblib_LTLIBRARIES = gb.db.firebird.la

gb_db_firebird_la_LIBADD = @FIREBIRD_LIB@
gb_db_firebird_la_LDFLAGS = -module @LD_FLAGS@

gb_db_firebird_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_db_firebird_la_SOURCES = \
  main.h main.cpp all_in_one.cpp
