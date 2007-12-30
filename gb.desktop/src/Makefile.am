COMPONENT = gb.desktop
include $(top_srcdir)/component.am

INCLUDES = @DESKTOP_INC@

gblib_LTLIBRARIES = gb.desktop.la

gb_desktop_la_LIBADD = @DESKTOP_LIB@
gb_desktop_la_LDFLAGS = -module @LD_FLAGS@

gb_desktop_la_SOURCES = \
 main.c main.h \
 desktop.c desktop.h \
 x11.c x11.h
