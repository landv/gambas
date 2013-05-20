dnl ---- configure.ac for gb.crypt.openssl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.crypt.openssl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  crypt_openssl, CRYPT_OPENSSL, gb.crypt.openssl, [src],
  libcrypto)

AC_OUTPUT(Makefile src/Makefile)
GB_PRINT_MESSAGES
