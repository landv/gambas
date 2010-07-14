dnl ---- Initialization

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.compress.zlib)
AC_PROG_LIBTOOL

dnl ---- zlib compression driver

GB_COMPONENT(
  zlib,
  ZLIB,
  [zlib compression],
  [src],
  [GB_FIND(zlib.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include)],
  [GB_FIND(libz.$SHLIBEXT, /usr/local /opt /usr, lib)],
  [$C_LIB -lz])

dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
