COMPONENT = gb.signal
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.signal.la

gb_signal_la_LIBADD = @GBX_THREAD_LIB@
gb_signal_la_LDFLAGS = -module @LD_FLAGS@
gb_signal_la_CFLAGS = -I$(top_srcdir)/share -I$(top_srcdir)/gbx @GBX_THREAD_INC@ @INCLTDL@ $(AM_CFLAGS)

gb_signal_la_SOURCES = \
 csignal.h csignal.c \
 main.h main.c


