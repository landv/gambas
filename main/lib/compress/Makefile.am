COMPONENT = gb.compress
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.compress.la

gb_compress_la_LIBADD = 
gb_compress_la_LDFLAGS = -module @LD_FLAGS@
gb_compress_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_compress_la_SOURCES = gb.compress.h main.h main.c CCompress.h CCompress.c CUncompress.h CUncompress.c


