COMPONENT = gb.qt4.webkit
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

gblib_LTLIBRARIES = gb.qt4.webkit.la

gb_qt4_webkit_la_LIBADD = @QTWEBKIT_LIB@
gb_qt4_webkit_la_LDFLAGS = -module @LD_FLAGS@ @QTWEBKIT_LDFLAGS@
gb_qt4_webkit_la_CPPFLAGS = @QTWEBKIT_INC@-I$(top_srcdir)/share -I$(top_srcdir)/src/share

gb_qt4_webkit_la_SOURCES = \
 main.h main.cpp \
 cwebsettings.h cwebsettings.cpp \
 cwebframe.h cwebframe.cpp cwebframe_moc.cpp \
 cwebelement.h cwebelement.cpp \
 cwebview.h cwebview.cpp cwebview_moc.cpp \
 ccookiejar.h ccookiejar.cpp ccookiejar_moc.cpp \
 cwebhittest.h cwebhittest.cpp \
 cwebdownload.h cwebdownload.cpp cwebdownload_moc.cpp
