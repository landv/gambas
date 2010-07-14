dnl ---- Initialization

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.db.postgresql)
AC_PROG_LIBTOOL

dnl ---- PostgreSQL driver

GB_COMPONENT(
  postgresql,
  POSTGRESQL,
  [PostgreSQL driver],
  [src],
  [GB_FIND(libpq-fe.h postgres.h pg_type.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include/pgsql* pgsql*/include include/postgresql* postgresql*/include include)],
  [GB_FIND(libpq.$SHLIBEXT, /usr/local /opt /usr, lib pgsql*/lib postgresql*/lib)],
  [$C_LIB -lpq])
 
dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
