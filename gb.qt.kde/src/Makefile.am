COMPONENT = gb.qt.kde
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

SUBDIRS = html

INCLUDES = @QT_INC@ @KDE_INC@

gblib_LTLIBRARIES = gb.qt.kde.la

gb_qt_kde_la_LIBADD = @QT_LIB@ @KDE_LIB@
gb_qt_kde_la_LDFLAGS = -module @LD_FLAGS@

gb_qt_kde_la_SOURCES = \
 main.h main.cpp moc_main.cpp \
 CDialog.h CDialog.cpp \
 CApplication.h CApplication.cpp \
 CURLLabel.h moc_CURLLabel.cpp CURLLabel.cpp \
 CArrowButton.h moc_CArrowButton.cpp CArrowButton.cpp \
 CColorBox.h moc_CColorBox.cpp CColorBox.cpp
