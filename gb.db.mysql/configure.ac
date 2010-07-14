dnl ---- configure.ac for gb.db.mysql driver

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.db.mysql)
LT_INIT

GB_COMPONENT(
  mysql,
  MYSQL,
  [MySQL driver],
  [src],
  [GB_FIND(mysql.h my_config.h, /opt/local /opt /usr/local/lib /usr/local /usr/lib /usr, include mysql*/include)],
  [GB_FIND(libmysqlclient.$SHLIBEXT, /opt/local /opt /usr/local /usr, lib mysql*/lib)],
  [$C_LIB -lmysqlclient -lz])
 
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
