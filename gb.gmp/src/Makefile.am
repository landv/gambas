COMPONENT = gb.gmp
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.gmp.la

gb_gmp_la_LIBADD = @GMP_LIB@
gb_gmp_la_LDFLAGS = -module @LD_FLAGS@ @GMP_LDFLAGS@
gb_gmp_la_CPPFLAGS = @GMP_INC@

gb_gmp_la_SOURCES = \
 main.c main.h \
 c_bigint.c c_bigint.h \
 c_rational.c c_rational.h
