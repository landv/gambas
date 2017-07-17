COMPONENT = gb.vb
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.vb.la

gb_vb_la_LIBADD = @C_LIB@ @MATH_LIB@
gb_vb_la_LDFLAGS = -module @LD_FLAGS@
gb_vb_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_vb_la_SOURCES = main.h main.c vb.h vb.c vbdate.h vbdate.c



 
