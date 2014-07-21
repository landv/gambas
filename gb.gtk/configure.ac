dnl ---- configure.ac for gb.gtk component

m4_include([../version.m4])
AC_INIT(gambas3-gb-gtk, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.gtk)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW()

GB_COMPONENT_PKG_CONFIG(
  gtk, GTK, gb.gtk, [src],
  'gtk+-2.0 >= 2.16' 'librsvg-2.0 >= 2.14.3' 'cairo >= 1.6.0' 'cairo-ft >= 1.6.0' 'gtk+-unix-print-2.0 >= 2.10' sm ice
)

GB_COMPONENT_PKG_CONFIG(
  gtkopengl, GTKOPENGL, gb.gtk.opengl, [opengl],
  gtkglext-1.0
)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/opengl/Makefile \
)

GB_PRINT_MESSAGES
