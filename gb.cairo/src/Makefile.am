COMPONENT = gb.cairo
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.cairo.la

gb_cairo_la_LIBADD = @CAIRO_LIB@
gb_cairo_la_LDFLAGS = -module @LD_FLAGS@
gb_cairo_la_CPPFLAGS = @CAIRO_INC@

gb_cairo_la_SOURCES = main.c main.h c_cairo.c c_cairo.h c_surface.c c_surface.h

