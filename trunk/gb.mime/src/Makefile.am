COMPONENT = gb.mime
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.mime.la

gb_mime_la_LIBADD = @MIME_LIB@
gb_mime_la_LDFLAGS = -module @LD_FLAGS@ @MIME_LDFLAGS@
gb_mime_la_CPPFLAGS = @MIME_INC@

gb_mime_la_SOURCES = main.c main.h \
 c_mime.h c_mime.c \
 c_mimemessage.h c_mimemessage.c \
 c_mimepart.h c_mimepart.c
