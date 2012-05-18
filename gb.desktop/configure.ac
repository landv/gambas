dnl ---- configure.ac for gb.desktop

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.desktop)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

AC_CHECK_LIB(Xtst, XTestQueryExtension, [X_LIBS="$X_LIBS -lXtst"], [touch DISABLED], [$X_LIBS])

GB_COMPONENT(
  desktop, DESKTOP, gb.desktop, [src],
  [],
  [],
  [$C_LIB $THREAD_LIB $X_LIBS],
  [$THREAD_INC ])

GB_COMPONENT_PKG_CONFIG(
  gnome, GNOME, gb.desktop.gnome, [gnome],
  gnome-keyring-1)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/gnome/Makefile \
)

GB_PRINT_MESSAGES
