COMPONENT = gb.corba
include $(top_srcdir)/component.am

INCLUDES = @CORBA_INC@

gblib_LTLIBRARIES = gb.corba.la

gb_corba_la_LIBADD = @CORBA_LIB@
gb_corba_la_LDFLAGS = -module @LD_FLAGS@
gb_corba_la_CXXFLAGS = $(AM_CXXFLAGS) -fexceptions

gb_corba_la_SOURCES = main.cpp main.h CCorbaApplication.cpp CCorbaApplication.h
