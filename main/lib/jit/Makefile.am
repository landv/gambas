COMPONENT = gb.jit
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.jit.la

gb_jit_la_LIBADD = @C_LIB@
gb_jit_la_LDFLAGS = -module @LD_FLAGS@
gb_jit_la_CFLAGS = -I$(top_srcdir)/share -I$(top_srcdir)/gbx $(AM_CFLAGS)

gb_jit_la_SOURCES = \
 main.h main.c \
 gb_str.h gb_str.c \
 gbc_reserved.c \
 jit.h jit.c jit_body.c


