dnl ---- configure.ac for gb.openssl

m4_include([../version.m4])
AC_INIT(gambas3-gb-openssl, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.openssl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  openssl, OPENSSL, gb.openssl, [src],
  libcrypto)

AC_OUTPUT(Makefile src/Makefile)
GB_PRINT_MESSAGES
