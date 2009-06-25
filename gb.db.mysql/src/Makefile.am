COMPONENT = gb.db.mysql
include $(top_srcdir)/component.am

INCLUDES = @MYSQL_INC@

gblib_LTLIBRARIES = gb.db.mysql.la

gb_db_mysql_la_LIBADD = @MYSQL_LIB@
gb_db_mysql_la_LDFLAGS = -module @LD_FLAGS@

gb_db_mysql_la_SOURCES = \
 main.h main.c
