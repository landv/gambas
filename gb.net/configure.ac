dnl ---- configure.ac for gb.net

AC_INIT(configure.ac)
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
