COMPONENT = gb.debug
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.debug.la

gb_debug_la_LIBADD = @GBX_THREAD_LIB@
gb_debug_la_LDFLAGS = -module @LD_FLAGS@
gb_debug_la_CFLAGS = -I$(top_srcdir)/share -I$(top_srcdir)/gbx @GBX_THREAD_INC@ @INCLTDL@ $(AM_CFLAGS)

gb_debug_la_SOURCES = \
 CDebug.h CDebug.c \
 print.h print.c \
 debug.h debug.c \
 profile.h profile.c \
 main.h main.c \
 gb.debug.h


