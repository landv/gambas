dnl ---- configure.ac for gb.gsl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.gsl)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  gsl, GSL, gb.gsl, [src],
  gsl)

GB_COMPONENT_SEARCH(
  gslcblas, GSLCBLAS, gb.gsl, [src],
  libgsl libgslcblas,
  [GB_FIND(gsl/gsl_cblas.h gsl/gsl_math.h, /usr /usr/local `gsl-config --prefix`, include include/gsl)],
  [GB_FIND(libgslcblas.$SHLIBEXT libgsl.$SHLIBEXT, /usr/lib /usr/local /usr/local/lib `gsl-config --prefix`, lib)],
  [$X_LIBS -llibgsl -llibgslcblas])

AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
