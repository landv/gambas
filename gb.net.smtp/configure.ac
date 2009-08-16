dnl ---- configure.ac for gb.net.smtp

AC_INIT(configure.ac)

AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.net.smtp)

AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  smtp,
  SMTP,
  [STMP client],
  [src],
  glib-2.0)

AC_OUTPUT( Makefile src/Makefile )
