dnl ---- configure.ac for gb.net.curl

AC_INIT(configure.ac)
GB_INIT(gb.net.curl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  curl,
  CURL,
  [Advanced networking component],
  [src],
  libcurl ">= 7.13"
  )

AC_OUTPUT( \
Makefile \
src/Makefile \
)
