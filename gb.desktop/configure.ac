dnl ---- configure.ac for gb.desktop

AC_INIT(configure.ac)
GB_INIT(gb.desktop)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

AC_CHECK_LIB(Xtst, XTestQueryExtension, [X_LIBS="$X_LIBS -lXtst"], [touch DISABLED], [-lX11])

GB_COMPONENT(
  desktop,
  DESKTOP,
  [Desktop-neutral routines],
  [src],
  [],
  [],
  [$C_LIB $THREAD_LIB $X_LIBS],
  [$THREAD_INC ])

AC_OUTPUT( Makefile src/Makefile )
