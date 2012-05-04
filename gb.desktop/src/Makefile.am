COMPONENT = gb.desktop
include $(top_srcdir)/component.am

SUBDIRS = . @GNOME_DIR@

gblib_LTLIBRARIES = gb.desktop.la

gb_desktop_la_LIBADD = @DESKTOP_LIB@
gb_desktop_la_LDFLAGS = -module @LD_FLAGS@ @DESKTOP_LDFLAGS@
gb_desktop_la_CPPFLAGS = @DESKTOP_INC@

gb_desktop_la_SOURCES = \
 main.c main.h \
 desktop.c desktop.h \
 x11.c x11.h \
 gb_list.c
