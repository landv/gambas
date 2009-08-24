COMPONENT = gb.gui
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.gui.la

gb_gui_la_LIBADD = 
gb_gui_la_LDFLAGS = -module @LD_FLAGS@
gb_gui_la_CFLAGS = -I$(top_srcdir)/share -I$(top_srcdir)/gbx @INCLTDL@ $(AM_CFLAGS)

gb_gui_la_SOURCES = \
 main.h main.c

