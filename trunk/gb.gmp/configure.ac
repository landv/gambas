dnl ---- configure.ac for gb.gmp

m4_include([../version.m4])
AC_INIT(gambas3-gb-gmp, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.gmp)
AC_PROG_LIBTOOL

GB_COMPONENT(
  gmp,
  GMP,
  gb.gmp,
  [src],
  [GB_FIND(gmp.h, /usr/local /usr /opt/local, include)],
  [GB_FIND(libgmp.$SHLIBEXT, /usr/local /usr /opt/local, lib)],
  [$C_LIB -lgmp],
  [ ])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
