COMPONENT = gb.inotify
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.inotify.la

gb_inotify_la_LIBADD = @INOTIFY_LIB@
gb_inotify_la_LDFLAGS = -module @LD_FLAGS@ @INOTIFY_LDFLAGS@
gb_inotify_la_CPPFLAGS = @INOTIFY_INC@ -I$(top_srcdir)/share

gb_inotify_la_SOURCES = \
 main.h main.c \
 c_watch.h c_watch.c
