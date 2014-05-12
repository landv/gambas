COMPONENT = gb.desktop.x11
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.desktop.x11.la

gb_desktop_x11_la_LIBADD = @DESKTOP_X11_LIB@
gb_desktop_x11_la_LDFLAGS = -module @LD_FLAGS@ @DESKTOP_X11_LDFLAGS@
gb_desktop_x11_la_CPPFLAGS = @DESKTOP_X11_INC@

gb_desktop_x11_la_SOURCES = \
 main.c main.h \
 desktop.c desktop.h \
 x11.c x11.h \
 gb_list.c
