dnl ---- configure.ac for gb.net.smtp

m4_include([../version.m4])
AC_INIT(gambas3-gb-net-smtp, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.net.smtp)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  smtp, SMTP, gb.net.smtp, [src],
  glib-2.0)

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
