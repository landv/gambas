dnl ---- configure.ac for gb.db.sqlite

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-sqlite3, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.sqlite3)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  sqlite3, SQLITE3, gb.db.sqlite3, [src],
  'sqlite3 >= 3.0')
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
