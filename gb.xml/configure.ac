dnl ---- configure.ac for gb.xml

m4_include([../version.m4])
AC_INIT(gambas3-gb-xml, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.xml)
AC_PROG_LIBTOOL

GB_COMPONENT(
  xml, XML, gb.xml, [src],
  [],
  [],
  []
)

GB_COMPONENT(
  xmlhtml, XMLHTML, gb.xml.html, [html],
  [],
  [],
  []
)
  
GB_COMPONENT_PKG_CONFIG(
  xmlxslt, XMLXSLT, gb.xml.xslt, [xslt],
  libxml-2.0 libxslt
)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/html/Makefile \
src/xslt/Makefile \
src/rpc/Makefile \
)

GB_PRINT_MESSAGES