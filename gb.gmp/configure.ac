dnl ---- configure.ac for gb.gmp

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.gmp)
AC_PROG_LIBTOOL
GB_COMPONENT(
  gmp,
  GMP,
  gb.gmp,
  [src],
  [GB_FIND(gmp.h, /usr/local /usr, include)],
  [GB_FIND(libgmp.$SHLIBEXT, /usr/local /usr, lib)],
  [$C_LIB -lgmp],
  [ ])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
