dnl ---- configure.ac for gb.ncurses

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.ncurses)
AC_PROG_LIBTOOL

GB_COMPONENT(
  ncurses, NCURSES, gb.ncurses, [src],
  [GB_FIND(ncurses.h panel.h, /usr/lib /usr, include)],
  [GB_FIND(libncurses.$SHLIBEXT libpanel.$SHLIBEXT, /usr/local /usr, lib)],
  [-lpanel -lncurses])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
