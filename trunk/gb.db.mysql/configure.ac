dnl ---- configure.ac for gb.db.mysql driver

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-mysql, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.mysql)
AC_PROG_LIBTOOL
dnl LT_INIT

GB_COMPONENT(
  mysql, MYSQL, gb.db.mysql, [src],
  [GB_FIND(mysql.h my_config.h, /opt/local /opt /usr/local/lib /usr/local /usr/lib /usr, include mysql*/include)],
  [GB_FIND(libmysqlclient.$SHLIBEXT, /opt/local /opt /usr/local /usr, lib mysql*/lib)],
  [$C_LIB -lmysqlclient -lz])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
