COMPONENT = gb.httpd
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.httpd.la

gb_httpd_la_LIBADD = @HTTPD_LIB@
gb_httpd_la_LDFLAGS = -module @LD_FLAGS@ @HTTPD_LDFLAGS@
gb_httpd_la_CPPFLAGS = @HTTPD_INC@

gb_httpd_la_SOURCES = main.c main.h \
 fdwatch.h fdwatch.c \
 libhttpd.h libhttpd.c \
 match.h match.c \
 mime_encodings.h \
 mime_types.h \
 tdate_parse.h tdate_parse.c \
 thttpd.h thttpd.c \
 timers.h timers.c \
 version.h

