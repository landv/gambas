COMPONENT = gb.gsl
include $(top_srcdir)/component.am

INCLUDES = @GSL_INC@

gblib_LTLIBRARIES = gb.gsl.la

gb_gsl_la_LIBADD = @GSL_LIB@
gb_gsl_la_LDFLAGS = -module @LD_FLAGS@ @GSL_LDFLAGS@

gb_gsl_la_SOURCES = main.c main.h c_gsl.c c_gsl.h c_complex.c c_complex.h c_polynomial.c c_polynomial.h c_complexpolynomial.c c_complexpolynomial.h
