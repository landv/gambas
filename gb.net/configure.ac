dnl ---- configure.ac for gb.net

m4_include([../version.m4])
AC_INIT(gambas3-gb-net, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.net)
AC_PROG_LIBTOOL

GB_COMPONENT(
  net, NET, gb.net, [src],
  [],
  [],
  [$C_LIB $THREAD_LIB],
  [$THREAD_INC])

AC_OUTPUT(Makefile src/Makefile)

GB_PRINT_MESSAGES
