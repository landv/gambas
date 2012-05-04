COMPONENT = gb.crypt
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.crypt.la

gb_crypt_la_LIBADD = @CRYPT_LIB@
gb_crypt_la_LDFLAGS = -module @LD_FLAGS@ @CRYPT_LDFLAGS@
gb_crypt_la_CPPFLAGS = @CRYPT_INC@

gb_crypt_la_SOURCES = \
 main.h main.c \
 c_crypt.h c_crypt.c


