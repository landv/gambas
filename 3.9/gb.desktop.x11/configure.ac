dnl ---- configure.ac for gb.desktop.x11

m4_include([../version.m4])
AC_INIT(gambas3-gb-desktop-x11, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.desktop.x11)

AC_PROG_LIBTOOL
GB_CHECK_XWINDOW
AC_CHECK_LIB(Xtst, XTestQueryExtension, [X_LIBS="$X_LIBS -lXtst"], [touch DISABLED], [$X_LIBS])

GB_COMPONENT(
  desktop_x11,
  DESKTOP_X11,
  gb.desktop.x11,
  [src],
  [],
  [],
  [$C_LIB $THREAD_LIB $X_LIBS],
  [$THREAD_INC])

AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
