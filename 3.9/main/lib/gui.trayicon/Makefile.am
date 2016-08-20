COMPONENT = gb.gui.trayicon
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.gui.trayicon.la

gb_gui_trayicon_la_LIBADD = 
gb_gui_trayicon_la_LDFLAGS = -module @LD_FLAGS@
gb_gui_trayicon_la_CFLAGS = -I$(top_srcdir)/share @INCLTDL@ $(AM_CFLAGS)

gb_gui_trayicon_la_SOURCES = \
 cfaketrayicon.h cfaketrayicon.c \
 main.h main.c

