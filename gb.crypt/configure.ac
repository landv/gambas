dnl ---- configure.ac for gb.crypt

AC_INIT(configure.ac)
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
