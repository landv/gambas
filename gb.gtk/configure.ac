dnl ---- configure.ac for gb.gtk component

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.gtk)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

GB_COMPONENT_PKG_CONFIG(
  gtk,
  GTK,
  [GTK+ toolkit],
  [src],
  gtk+-2.0 ">= 2.10" librsvg-2.0 ">= 2.14.3" cairo ">= 1.6.0" cairo-ft ">= 1.6.0" gtk+-unix-print-2.0 ">= 2.10"
)

GB_COMPONENT_PKG_CONFIG(
  gtkopengl,
  GTKOPENGL,
  [GTKglext toolkit],
  [opengl],
  gtkglext-1.0
)
  
AC_OUTPUT( \
Makefile \
src/Makefile \
src/opengl/Makefile
)

GB_PRINT_MESSAGES
