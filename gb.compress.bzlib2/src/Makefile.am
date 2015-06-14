COMPONENT = gb.compress.bzlib2
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.compress.bzlib2.la

gb_compress_bzlib2_la_LIBADD = @BZLIB2_LIB@
gb_compress_bzlib2_la_LDFLAGS = -module @LD_FLAGS@ @BZLIB2_LDFLAGS@
gb_compress_bzlib2_la_CFLAGS = $(AM_CFLAGS) @BZLIB2_INC@

gb_compress_bzlib2_la_SOURCES = main.h main.c
