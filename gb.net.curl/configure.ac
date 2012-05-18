dnl ---- configure.ac for gb.net.curl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.net.curl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  curl, CURL, gb.net.curl, [src],
  libcurl ">= 7.13"
  )

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
