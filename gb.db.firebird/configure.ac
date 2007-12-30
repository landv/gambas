dnl ---- configure.ac for gb.db.firebird driver

AC_INIT(configure.ac)
GB_INIT(gb.db.firebird)
AC_PROG_LIBTOOL

dnl -- Only the presence of libfbclient.so.2 is tested. Apparently, no link is needed?

GB_COMPONENT(
  firebird,
  FIREBIRD,
  [firebird driver],
  [src],
  [],
  [GB_FIND(libfbclient.$SHLIBEXT.2, /usr/local /opt /usr, lib firebird2*/lib firebird*/lib lib/firebird2* lib/firebird*)],
  [])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)
