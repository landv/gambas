dnl ---- configure.ac for gb.net.curl

AC_INIT(configure.ac)
GB_INIT(gb.net.curl)
AC_PROG_LIBTOOL

GB_COMPONENT(
  curl,
  CURL,
  [Advanced networking component],
  [src],
  [GB_FIND(curl.h, `curl-config --prefix 2> /dev/null`, include)],
  [GB_FIND(libcurl.$SHLIBEXT, `curl-config --prefix 2> /dev/null`, lib)],
  [$C_LIB `curl-config --libs 2> /dev/null`])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)
