dnl ---- configure.ac for gb.xml

AC_INIT(configure.ac)
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
