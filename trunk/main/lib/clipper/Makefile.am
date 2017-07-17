COMPONENT = gb.clipper
include $(top_srcdir)/component.am

noinst_LTLIBRARIES = libclipper.la
gblib_LTLIBRARIES = gb.clipper.la

libclipper_la_LIBADD =
libclipper_la_LDFLAGS = -module @LD_FLAGS@
libclipper_la_CXXFLAGS = -I$(top_srcdir)/share $(AM_CXXFLAGS_OPT) -fexceptions

libclipper_la_SOURCES = \
 clipper.hpp clipper.cpp

gb_clipper_la_LIBADD = libclipper.la
gb_clipper_la_LDFLAGS = -module @LD_FLAGS@
gb_clipper_la_CXXFLAGS = -I$(top_srcdir)/share $(AM_CXXFLAGS)

gb_clipper_la_SOURCES = \
 main.h main.cpp gb.geom.h c_clipper.cpp c_clipper.h
