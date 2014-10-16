dnl ---- configure.ac for gb.gtk3 component

m4_include([../version.m4])
AC_INIT(gambas3-gb-gtk3, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.gtk3)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW()

GB_COMPONENT_PKG_CONFIG(
  gtk3, GTK3, gb.gtk3, [src],
  'gtk+-3.0 >= 3.4' 'librsvg-2.0 >= 2.14.3' 'cairo >= 1.6.0' 'cairo-ft >= 1.6.0' 'gtk+-unix-print-3.0 >= 3.4' sm ice
)

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
