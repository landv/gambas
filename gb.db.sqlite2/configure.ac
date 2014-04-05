dnl ---- configure.ac for gb.db.sqlite2 driver

dnl ---- Initialization

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-sqlite2, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.sqlite2)
AC_PROG_LIBTOOL

dnl ---- SQLite driver

GB_COMPONENT(
  sqlite2, SQLITE2, gb.db.sqlite2, [src],
  [GB_FIND(sqlite.h, /usr/local/lib /usr/local /opt /usr/lib /usr /opt/local, include sqlite*/include)],
  [GB_FIND(libsqlite.$SHLIBEXT, /usr/local /opt /usr /opt/local, lib sqlite*/lib)],
  [$C_LIB $CXX_LIB -lsqlite])

dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
