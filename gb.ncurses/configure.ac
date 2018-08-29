dnl ---- configure.ac for gb.ncurses

m4_include([../version.m4])
AC_INIT(gambas3-gb-ncurses, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.ncurses)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  ncurses, NCURSES, gb.ncurses, [src],
  ncurses panel)

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
