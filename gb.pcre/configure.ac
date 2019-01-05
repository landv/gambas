dnl ---- configure.ac for gb.pcre

m4_include([../version.m4])
AC_INIT(gambas3-gb-pcre, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.pcre)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  pcre, PCRE, gb.pcre, [src],
  libpcre)

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
