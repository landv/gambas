dnl ---- configure.ac for gb.openssl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.openssl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  openssl, OPENSSL, gb.openssl, [src],
  libcrypto)

AC_OUTPUT(Makefile src/Makefile)
GB_PRINT_MESSAGES
