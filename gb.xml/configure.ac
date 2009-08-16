dnl ---- configure.ac for gb.xml

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.xml)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  xml,
  XML,
  [XML parser component],
  [src],
  libxml-2.0)

GB_COMPONENT_PKG_CONFIG(
  xslt,
  XSLT,
  [XSLT/XML parser component],
  [xslt],
  libxslt)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/xslt/Makefile \
src/rpc/Makefile \
)
