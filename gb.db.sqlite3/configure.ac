dnl ---- configure.ac for gb.db.sqlite

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.db.sqlite3)
AC_PROG_LIBTOOL

GB_COMPONENT_SEARCH(
  sqlite3,
  SQLITE3,
  [SQLite 3 driver],
  [src],
  sqlite3 ">= 3.0",
  [GB_FIND(sqlite3.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include sqlite*/include)],
  [GB_FIND(libsqlite3.$SHLIBEXT, /usr/local /opt /usr, lib sqlite*/lib)],
  [$C_LIB $CXX_LIB -lsqlite3])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)
