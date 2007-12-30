COMPONENT = gb.qte.ext
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qte.am

INCLUDES = @QTE_INC@

gblib_LTLIBRARIES = gb.qte.ext.la

gb_qte_ext_la_LIBADD = @QTE_LIB@
gb_qte_ext_la_LDFLAGS = -module @LD_FLAGS@
gb_qte_ext_la_CXXFLAGS = -fno-rtti -DQWS -DGB_QT_COMPONENT $(AM_CXXFLAGS)

gb_qte_ext_la_SOURCES = \
 main.h main.cpp \
 CLCDNumber.h moc_CLCDNumber.cpp CLCDNumber.cpp \
 CDial.h moc_CDial.cpp CDial.cpp \
 CTextView.h moc_CTextView.cpp CTextView.cpp \
 garray.h garray.cpp \
 gstring.h gstring.cpp \
 gdocument.h gdocument.cpp \
 gview.h moc_gview.cpp gview.cpp \
 CEditor.h moc_CEditor.cpp CEditor.cpp
