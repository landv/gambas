COMPONENT = gb.qt5.opengl
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

gblib_LTLIBRARIES = gb.qt5.opengl.la

gb_qt5_opengl_la_LIBADD = @QT5_LIB@ @QT5OPENGL_LIB@
gb_qt5_opengl_la_LDFLAGS = -module @LD_FLAGS@ @QT5_LDFLAGS@ @QT5OPENGL_LDFLAGS@
gb_qt5_opengl_la_CPPFLAGS = @QT5_INC@ @QT5OPENGL_INC@

gb_qt5_opengl_la_SOURCES = \
 main.h main.cpp \
 CGLarea.h CGLarea_moc.cpp CGLarea.cpp

