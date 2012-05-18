dnl ---- configure.ac for gb.pcre

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.pcre)
AC_PROG_LIBTOOL

GB_COMPONENT(
  pcre, PCRE, gb.pcre, [src],
  [GB_FIND(pcre.h, /usr/local/lib /usr/local /usr/lib /usr, include)],
  [GB_FIND(libpcre.$SHLIBEXT, /usr/local /usr /, lib)],
  [-lpcre])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
