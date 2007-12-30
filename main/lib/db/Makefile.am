COMPONENT = gb.db
include $(top_srcdir)/component.am

INCLUDES = -I$(top_srcdir)/share

gblib_LTLIBRARIES = gb.db.la

gb_db_la_LIBADD = @C_LIB@
gb_db_la_LDFLAGS = -module @LD_FLAGS@

gb_db_la_SOURCES = \
 gb.db.h gb.db.proto.h gb_barray.h \
 main.h main.c \
 deletemap.h deletemap.c \
 CConnection.h CConnection.c \
 CDatabase.h CDatabase.c \
 CUser.h CUser.c \
 CTable.h CTable.c \
 CField.h CField.c \
 CIndex.h CIndex.c \
 CResult.h CResult.c \
 CResultField.h CResultField.c \
 sqlite.h sqlite.c


