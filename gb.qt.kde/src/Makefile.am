COMPONENT = gb.qt.kde
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

SUBDIRS = html

INCLUDES = @QT_INC@ @KDE_INC@ @TRINITY_INC@

gblib_LTLIBRARIES = gb.qt.kde.la

gb_qt_kde_la_LIBADD = @QT_LIB@ @KDE_LIB@ @TRINITY_LIB@
gb_qt_kde_la_LDFLAGS = -module @LD_FLAGS@

gb_qt_kde_la_SOURCES = \
 main.h main_moc.cpp main.cpp \
 CDialog.h CDialog.cpp \
 CApplication.h CApplication.cpp \
 CURLLabel.h CURLLabel_moc.cpp CURLLabel.cpp \
 CArrowButton.h CArrowButton_moc.cpp CArrowButton.cpp \
 CColorBox.h CColorBox_moc.cpp CColorBox.cpp
