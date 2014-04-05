dnl ---- configure.ac for gb.db.odbc driver

dnl ---- Initialization

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-odbc, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.odbc)
AC_PROG_LIBTOOL

dnl ---- ODBC driver

GB_COMPONENT(
  odbc, ODBC, gb.db.odbc, [src],
  [GB_FIND(sql.h sqlext.h sqltypes.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include odbc*/include)],
  [GB_FIND(libodbc.$SHLIBEXT, /usr/local /opt /usr /opt/local, lib odbc*/lib )],
  [$C_LIB -lodbc])
 
dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
