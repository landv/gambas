COMPONENT = gb.qt.ext
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

INCLUDES = @QT_INC@ -I$(top_srcdir)/share -I$(top_srcdir)/src/share

gblib_LTLIBRARIES = gb.qt.ext.la

gb_qt_ext_la_LIBADD = @QT_LIB@
gb_qt_ext_la_LDFLAGS = -module @LD_FLAGS@ @QT_LDFLAGS@

gb_qt_ext_la_SOURCES = \
 main.h main.cpp \
 CLCDNumber.h CLCDNumber_moc.cpp CLCDNumber.cpp \
 CDial.h CDial_moc.cpp CDial.cpp \
 garray.h garray.cpp \
 gstring.h gstring.cpp \
 gdocument.h gdocument.cpp \
 gview.h gview_moc.cpp gview.cpp \
 CEditor.h CEditor_moc.cpp CEditor.cpp \
 CDraw.h CDraw.cpp \
 CDrawing.h CDrawing_moc.cpp CDrawing.cpp \
 CPrinter.h CPrinter_moc.cpp CPrinter.cpp \
 CTextEdit.h CTextEdit_moc.cpp CTextEdit.cpp
