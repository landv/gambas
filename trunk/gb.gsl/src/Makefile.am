COMPONENT = gb.gsl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.gsl.la

gb_gsl_la_LIBADD = @GSL_LIB@
gb_gsl_la_LDFLAGS = -module @LD_FLAGS@ @GSL_LDFLAGS@
gb_gsl_la_CPPFLAGS = @GSL_INC@

gb_gsl_la_SOURCES = \
 main.c main.h \
 c_gsl.c c_gsl.h \
 c_complex.c c_complex.h \
 c_vector.c c_vector.h \
 c_matrix.c c_matrix.h \
 c_polynomial.c c_polynomial.h
