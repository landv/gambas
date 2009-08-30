dnl ---- configure.ac for gb.db.mysql driver

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.db.mysql)
AC_PROG_LIBTOOL

GB_COMPONENT(
  mysql,
  MYSQL,
  [MySQL driver],
  [src],
  [GB_FIND(mysql.h my_config.h, /usr/local/lib /usr/local /opt /usr/lib /usr, include mysql*/include)],
  [GB_FIND(libmysqlclient.$SHLIBEXT, /usr/local /opt /usr, lib mysql*/lib)],
  [$C_LIB -lmysqlclient -lz])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)
