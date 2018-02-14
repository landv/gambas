COMPONENT = gb.qt5.ext
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

gblib_LTLIBRARIES = gb.qt5.ext.la

gb_qt5_ext_la_LIBADD = @QT5EXT_LIB@
gb_qt5_ext_la_LDFLAGS = -module @LD_FLAGS@ @QT5EXT_LDFLAGS@
gb_qt5_ext_la_CXXFLAGS = @THREAD_INC@ $(AM_CXXFLAGS) -std=c++11
gb_qt5_ext_la_CPPFLAGS = @QT5EXT_INC@ -I$(top_srcdir)/share -I$(top_srcdir)/src/share

gb_qt5_ext_la_SOURCES = \
 main.h main.cpp \
 CLCDNumber.h CLCDNumber_moc.cpp CLCDNumber.cpp \
 CDial.h CDial_moc.cpp CDial.cpp \
 CTextEdit.h CTextEdit_moc.cpp CTextEdit.cpp
