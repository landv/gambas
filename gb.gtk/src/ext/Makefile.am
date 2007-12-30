COMPONENT = gb.gtk.ext
include $(top_srcdir)/component.am

INCLUDES = @GTK_INC@ -I$(top_srcdir)/share

gblib_LTLIBRARIES = gb.gtk.ext.la

gb_gtk_ext_la_LIBADD = @GTK_LIB@
gb_gtk_ext_la_LDFLAGS = -module @LD_FLAGS@

gb_gtk_ext_la_SOURCES = \
 main.h main.cpp \
 CGridView.h CGridView.cpp