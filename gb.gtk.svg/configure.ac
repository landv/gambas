dnl ---- configure.ac for gb.gtk.svg

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.gtk.svg)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

GB_COMPONENT_PKG_CONFIG(
  gtk,
  GTK,
  [GTK+ component],
  [src],
  gtk+-2.0 ">= 2.8.0"
)

GB_COMPONENT_PKG_CONFIG(
  gtk_svg,
  GTK_SVG,
  [SVG loader for GTK+],
  [src],
  librsvg-2.0 ">= 2.14.3"
)

AC_OUTPUT( \
Makefile \
src/Makefile \
)
