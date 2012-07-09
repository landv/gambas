COMPONENT = gb.complex
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.complex.la

gb_complex_la_LIBADD = @MATH_LIB@
gb_complex_la_LDFLAGS = -module @LD_FLAGS@
gb_complex_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_complex_la_SOURCES = \
 ccomplex.h ccomplex.c \
 main.h main.c


