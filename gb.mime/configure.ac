dnl ---- configure.ac for gb.mime

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.mime)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  mime,
  MIME,
  gb.mime,
  [src],
  gmime-2.6)
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
