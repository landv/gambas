COMPONENT = gb.db
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.db.la

gb_db_la_LIBADD = @C_LIB@
gb_db_la_LDFLAGS = -module @LD_FLAGS@
gb_db_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_db_la_SOURCES = \
 gb.db.h gb.db.proto.h gb_barray.h \
 c_subcollection.h c_subcollection.c \
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


