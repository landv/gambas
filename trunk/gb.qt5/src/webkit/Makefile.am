COMPONENT = gb.qt5.webkit
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

gblib_LTLIBRARIES = gb.qt5.webkit.la

gb_qt5_webkit_la_LIBADD = @QT5WEBKIT_LIB@
gb_qt5_webkit_la_LDFLAGS = -module @LD_FLAGS@ @QT5WEBKIT_LDFLAGS@
gb_qt5_webkit_la_CXXFLAGS = @THREAD_INC@ $(AM_CXXFLAGS) -std=c++11
gb_qt5_webkit_la_CPPFLAGS = @QT5WEBKIT_INC@ -I$(top_srcdir)/share -I$(top_srcdir)/src/share

gb_qt5_webkit_la_SOURCES = \
 main.h main.cpp \
 cwebsettings.h cwebsettings.cpp \
 cwebframe.h cwebframe.cpp cwebframe_moc.cpp \
 cwebelement.h cwebelement.cpp \
 cwebview.h cwebview.cpp cwebview_moc.cpp \
 ccookiejar.h ccookiejar.cpp ccookiejar_moc.cpp \
 cwebhittest.h cwebhittest.cpp \
 cwebdownload.h cwebdownload.cpp cwebdownload_moc.cpp
