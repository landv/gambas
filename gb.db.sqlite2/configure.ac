dnl ****************************************************************************
dnl   configure.ac for gb.db.sqlite driver
dnl   (c) Benoît Minisini 2000-2005
dnl ****************************************************************************

dnl ---- Initialization

AC_INIT(configure.ac)
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
