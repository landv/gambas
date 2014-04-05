dnl ---- configure.ac for gb.crypt

m4_include([../version.m4])
AC_INIT(gambas3-gb-crypt, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.crypt)
AC_PROG_LIBTOOL

GB_COMPONENT(
  crypt, CRYPT, gb.crypt, [src],
  [GB_FIND(crypt.h, /usr/lib /usr, include)],
  [GB_FIND(libcrypt.$SHLIBEXT, /usr/local /usr, lib)],
  [-lcrypt])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
