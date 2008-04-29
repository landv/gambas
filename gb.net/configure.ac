dnl ---- configure.ac for gb.net

AC_INIT(configure.ac)
GB_INIT(gb.net)
AC_PROG_LIBTOOL

GB_COMPONENT(
  net,
  NET,
  [Networking component],
  [src],
  [],
  [],
  [$C_LIB $THREAD_LIB],
  [$THREAD_INC])

AC_OUTPUT(Makefile src/Makefile)

