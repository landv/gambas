dnl ---- configure.ac for gb.db.sqlite

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-sqlite3, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.sqlite3)
AC_PROG_LIBTOOL

GB_COMPONENT_SEARCH(
  sqlite3, SQLITE3, gb.db.sqlite3, [src],
  sqlite3 ">= 3.0",
  [GB_FIND(sqlite3.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include sqlite*/include)],
  [GB_FIND(libsqlite3.$SHLIBEXT, /usr/local /opt /usr, lib sqlite*/lib)],
  [$C_LIB $CXX_LIB -lsqlite3])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
