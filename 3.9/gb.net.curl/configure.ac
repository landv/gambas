dnl ---- configure.ac for gb.net.curl

m4_include([../version.m4])
AC_INIT(gambas3-gb-net-curl, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.net.curl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  curl, CURL, gb.net.curl, [src],
  'libcurl >= 7.13'
  )

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
