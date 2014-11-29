COMPONENT = gb.openssl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.openssl.la

gb_openssl_la_LIBADD = @OPENSSL_LIB@
gb_openssl_la_LDFLAGS = -module @LD_FLAGS@ @OPENSSL_LDFLAGS@
gb_openssl_la_CPPFLAGS = @OPENSSL_INC@ -I../../main/gbx -I../../main/share

gb_openssl_la_SOURCES = \
 main.h main.c \
 c_digest.h c_digest.c \
 c_cipher.h c_cipher.c \
 c_hmac.h c_hmac.c

 # c_signature.h c_signature.c
