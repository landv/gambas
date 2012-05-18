dnl ---- configure.ac for gb.dbus

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.dbus)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  dbus, DBUS, gb.dbus, [src],
  dbus-1)
AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
