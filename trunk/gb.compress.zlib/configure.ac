dnl ---- Initialization

m4_include([../version.m4])
AC_INIT(gambas3-gb-compress-zlib, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.compress.zlib)
AC_PROG_LIBTOOL

dnl ---- zlib compression driver

GB_COMPONENT(
  zlib, ZLIB, gb.compress.zlib, [src],
  [GB_FIND(zlib.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include)],
  [GB_FIND(libz.$SHLIBEXT, /usr/local /opt /usr, lib)],
  [$C_LIB -lz])

dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
