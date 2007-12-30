COMPONENT = gb.debug
include $(top_srcdir)/component.am

INCLUDES = -I$(top_srcdir)/share -I$(top_srcdir)/gbx @INCLTDL@

gblib_LTLIBRARIES = gb.debug.la

gb_debug_la_LIBADD = 
gb_debug_la_LDFLAGS = -module @LD_FLAGS@

gb_debug_la_SOURCES = \
 CDebug.h CDebug.c \
 print.h print.c \
 debug.h debug.c \
 main.h main.c \
 gb.debug.h


