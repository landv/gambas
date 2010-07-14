dnl ---- configure.ac for gb.db.firebird driver

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.db.firebird)
AC_PROG_LIBTOOL

GB_COMPONENT(
  firebird,
  FIREBIRD,
  [firebird driver],
  [src],
  [],
  [GB_FIND(libfbclient.$SHLIBEXT, /usr/local /opt /usr, lib firebird2*/lib firebird*/lib lib/firebird2* lib/firebird*)],
  [-lfbclient])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
