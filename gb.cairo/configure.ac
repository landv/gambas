dnl ---- configure.ac for gb.cairo

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.cairo)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  cairo, CAIRO, gb.cairo, [src],
  'cairo >= 1.6.0' 'cairo-ft >= 1.6.0'
)

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
