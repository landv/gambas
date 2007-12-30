COMPONENT = gb.qt.opengl
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

INCLUDES = @QT_INC@ @QTOPENGL_INC@

gblib_LTLIBRARIES = gb.qt.opengl.la

gb_qt_opengl_la_LIBADD = @QT_LIB@ @QTOPENGL_LIB@
gb_qt_opengl_la_LDFLAGS = -module @LD_FLAGS@

gb_qt_opengl_la_SOURCES = \
 main.h main.cpp \
 CGLarea.h moc_CGLarea.cpp CGLarea.cpp

