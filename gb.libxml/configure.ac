dnl ---- configure.ac for gb.xml

m4_include([../version.m4])
AC_INIT(gambas3-gb-libxml, GB_VERSION, GB_MAIL, [], GB_URL)

AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.libxml)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  xml, XML, gb.libxml, [src],
  libxml-2.0)

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
