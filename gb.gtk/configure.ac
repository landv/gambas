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
  gtk+-2.0 ">= 2.16" librsvg-2.0 ">= 2.14.3" cairo ">= 1.6.0" cairo-ft ">= 1.6.0" gtk+-unix-print-2.0 ">= 2.10"
)

dnl GB_COMPONENT_PKG_CONFIG(
dnl   gtkopengl,
dnl   GTKOPENGL,
dnl   [GTKglext toolkit],
dnl   [opengl],
dnl   gtkglext-1.0
dnl )
  
AC_OUTPUT( \
Makefile \
src/Makefile \
)

dnl src/opengl/Makefile

GB_PRINT_MESSAGES
