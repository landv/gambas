COMPONENT = gb.compress.zlib
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.compress.zlib.la

gb_compress_zlib_la_LIBADD = @ZLIB_LIB@
gb_compress_zlib_la_LDFLAGS = -module @LD_FLAGS@ @ZLIB_LDFLAGS@
gb_compress_zlib_la_CFLAGS = $(AM_CFLAGS) @ZLIB_INC@

gb_compress_zlib_la_SOURCES = main.h main.c
