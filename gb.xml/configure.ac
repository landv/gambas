dnl ---- configure.ac for gb.xml

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.xml)
AC_PROG_LIBTOOL
GB_CXXFLAGS_GCC_OPTION(-std=c++0x)

GB_COMPONENT(
  xml,
  XML,
  [XML document manipulation],
  [src],
  [],
  [],
  [])

GB_COMPONENT(
  xmlhtml,
  XMLHTML,
  [HTML document manipulation],
  [html],
  [],
  [],
  [])
  
GB_COMPONENT_PKG_CONFIG(
  xmlxslt,
  XMLXSLT,
  [XSLT/XML parser component],
  [xslt],
  [libxml-2.0, libxslt])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/html/Makefile \
src/xslt/Makefile \
src/rpc/Makefile \
)

GB_PRINT_MESSAGES