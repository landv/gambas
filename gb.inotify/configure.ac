dnl ---- configure.ac for gb.inotify
AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.inotify)
AC_PROG_LIBTOOL
GB_COMPONENT(
  inotify,
  INOTIFY,
  gb.inotify,
  [src],
  [GB_FIND(sys/inotify.h, /usr/local/lib /usr/local /usr/lib /usr, include)],
  [GB_FIND(, /usr/local /usr, lib)],
  [$C_LIB ],
  [ ])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
