COMPONENT = gb.qt4.ext
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

gblib_LTLIBRARIES = gb.qt4.ext.la

gb_qt4_ext_la_LIBADD = @QTEXT_LIB@
gb_qt4_ext_la_LDFLAGS = -module @LD_FLAGS@ @QTEXT_LDFLAGS@
gb_qt4_ext_la_CPPFLAGS = -DQT3_SUPPORT @QTEXT_INC@ -I$(top_srcdir)/share -I$(top_srcdir)/src/share

gb_qt4_ext_la_SOURCES = \
 main.h main.cpp \
 CLCDNumber.h CLCDNumber_moc.cpp CLCDNumber.cpp \
 CDial.h CDial_moc.cpp CDial.cpp \
 garray.h garray.cpp \
 gstring.h gstring.cpp \
 gdocument.h gdocument.cpp \
 gview.h gview_moc.cpp gview.cpp \
 CEditor.h CEditor_moc.cpp CEditor.cpp \
 CTextEdit.h CTextEdit_moc.cpp CTextEdit.cpp
