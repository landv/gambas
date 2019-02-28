dnl ---- configure.ac for gb.input
m4_include([../version.m4])
AC_INIT(gambas3-gb-input, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.input)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  input,
  INPUT,
  gb.input,
  [src],
  libevdev)
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
