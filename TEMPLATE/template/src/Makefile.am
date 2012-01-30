COMPONENT = gb.gsl
include $(top_srcdir)/component.am

INCLUDES = @GSL_INC@

gblib_LTLIBRARIES = gb.gsl.la

gb_gsl_la_LIBADD = @GSL_LIB@
gb_gsl_la_LDFLAGS = -module @LD_FLAGS@ @GSL_LDFLAGS@

gb_gsl_la_SOURCES = main.c main.h c_gsl.c c_gsl.h
