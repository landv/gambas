COMPONENT = gb.qt4.opengl
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

INCLUDES = -DQT3_SUPPORT @QT_INC@ @QTOPENGL_INC@

gblib_LTLIBRARIES = gb.qt4.opengl.la

gb_qt4_opengl_la_LIBADD = @QT_LIB@ @QTOPENGL_LIB@
gb_qt4_opengl_la_LDFLAGS = -module @LD_FLAGS@ @QT_LDFLAGS@ @QTOPENGL_LDFLAGS@

gb_qt4_opengl_la_SOURCES = \
 main.h main.cpp \
 CGLarea.h CGLarea_moc.cpp CGLarea.cpp

