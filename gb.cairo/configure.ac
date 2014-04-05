dnl ---- configure.ac for gb.cairo

m4_include([../version.m4])
AC_INIT(gambas3-gb-cairo, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.cairo)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  cairo, CAIRO, gb.cairo, [src],
  'cairo >= 1.6.0' 'cairo-ft >= 1.6.0'
)

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
