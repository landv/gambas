dnl ---- Initialization

m4_include([../version.m4])
AC_INIT(gambas3-gb-compress-bzlib2, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.compress.bzlib2)
AC_PROG_LIBTOOL

dnl ---- bzlib2 compression driver

GB_COMPONENT(
  bzlib2, BZLIB2, gb.compress.bzlib2, [src],
  [GB_FIND(bzlib.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include)],
  [GB_FIND(libbz2.$SHLIBEXT, /usr/local /opt /usr, lib)],
  [$C_LIB -lbz2])
 
dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
