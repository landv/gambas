dnl ---- configure.ac for gb.desktop

AC_INIT(configure.ac)
GB_INIT(gb.desktop)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

AC_CHECK_LIB(Xtst, XTestQueryExtension, [X_LIBS="$X_LIBS -lXtst"], [touch DISABLED], [$X_LIBS])

GB_COMPONENT(
  desktop,
  DESKTOP,
  [Desktop-neutral routines],
  [src],
  [],
  [],
  [$C_LIB $THREAD_LIB $X_LIBS],
  [$THREAD_INC ])

GB_COMPONENT_PKG_CONFIG(
  gnome,
  GNOME,
  [Gnome desktop routines],
  [gnome],
  gnome-keyring-1)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/gnome/Makefile \
)
