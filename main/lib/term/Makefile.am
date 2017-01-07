COMPONENT = gb.term
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.term.la

gb_term_la_LIBADD = @GBX_THREAD_LIB@
gb_term_la_LDFLAGS = -module @LD_FLAGS@
gb_term_la_CFLAGS = -I$(top_srcdir)/share -I$(top_srcdir)/gbx @GBX_THREAD_INC@ @INCLTDL@ $(AM_CFLAGS)

gb_term_la_SOURCES = \
 cterm.h cterm.c \
 main.h main.c


