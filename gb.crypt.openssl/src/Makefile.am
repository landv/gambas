COMPONENT = gb.crypt.openssl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.crypt.openssl.la

gb_crypt_openssl_la_LIBADD = @CRYPT_OPENSSL_LIB@
gb_crypt_openssl_la_LDFLAGS = -module @LD_FLAGS@ @CRYPT_OPENSSL_LDFLAGS@
gb_crypt_openssl_la_CPPFLAGS = @CRYPT_OPENSSL_INC@

gb_crypt_openssl_la_SOURCES = \
 main.h main.c \
 c_digest.h c_digest.c \
 c_cipher.h c_cipher.c \
 c_hmac.h c_hmac.c
