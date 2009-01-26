dnl ---- configure.ac for gb.cairo

AC_INIT(configure.ac)
GB_INIT(gb.cairo)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  cairo,
  CAIRO,
  [Cairo component],
  [src],
  cairo ">= 1.8.0"
)

GB_COMPONENT_PKG_CONFIG(
  cairo_ft,
  CAIRO_FT,
  [FreeType backend for Cairo],
  [src],
  cairo-ft ">= 1.8.0"
)

AC_OUTPUT( Makefile src/Makefile )
