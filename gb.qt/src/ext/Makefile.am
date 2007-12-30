COMPONENT = gb.qt.ext
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

INCLUDES = @QT_INC@ -I$(top_srcdir)/share -I$(top_srcdir)/src/share

gblib_LTLIBRARIES = gb.qt.ext.la

gb_qt_ext_la_LIBADD = @QT_LIB@
gb_qt_ext_la_LDFLAGS = -module @LD_FLAGS@

gb_qt_ext_la_SOURCES = \
 main.h main.cpp \
 CLCDNumber.h moc_CLCDNumber.cpp CLCDNumber.cpp \
 CDial.h moc_CDial.cpp CDial.cpp \
 garray.h garray.cpp \
 gstring.h gstring.cpp \
 gdocument.h gdocument.cpp \
 gview.h moc_gview.cpp gview.cpp \
 CEditor.h moc_CEditor.cpp CEditor.cpp \
 CDraw.h CDraw.cpp \
 CDrawing.h moc_CDrawing.cpp CDrawing.cpp \
 CPrinter.h moc_CPrinter.cpp CPrinter.cpp \
 CTextEdit.h moc_CTextEdit.cpp CTextEdit.cpp
