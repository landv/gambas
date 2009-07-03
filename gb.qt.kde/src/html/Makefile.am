COMPONENT = gb.qt.kde.html
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

INCLUDES = @QT_INC@ @KDE_INC@

gblib_LTLIBRARIES = gb.qt.kde.html.la

gb_qt_kde_html_la_LIBADD = @QT_LIB@ @KDE_LIB@ -lkhtml -lkjs
gb_qt_kde_html_la_LDFLAGS = -module @LD_FLAGS@ @KDE_LDFLAGS@

gb_qt_kde_html_la_SOURCES = \
 main.h main_moc.cpp main.cpp \
 CWebBrowser.h CWebBrowser_moc.cpp CWebBrowser.cpp
