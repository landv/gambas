dnl ---- configure.ac for gb.gsl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.gsl)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  gsl,
  GSL,
  [GNU Scientific Library component],
  [src],
  gsl)
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
