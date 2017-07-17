dnl ---- configure.ac for gb.db.postgresql driver

dnl ---- Initialization

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-postgresql, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.postgresql)
AC_PROG_LIBTOOL

dnl ---- PostgreSQL driver

GB_COMPONENT(
  postgresql, POSTGRESQL, gb.db.postgresql, [src],
  [GB_FIND(libpq-fe.h postgres.h pg_type.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include/pgsql* pgsql*/include include/postgresql* postgresql*/include include/postgresql/*/server/catalog include/postgresql/*/server include)],
  [GB_FIND(libpq.$SHLIBEXT, /usr/local /opt /usr, lib pgsql*/lib postgresql*/lib)],
  [$C_LIB -lpq])
 
dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
