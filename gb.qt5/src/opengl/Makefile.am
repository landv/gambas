COMPONENT = gb.qt5.opengl
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

gblib_LTLIBRARIES = gb.qt5.opengl.la

gb_qt5_opengl_la_LIBADD = @QT5OPENGL_LIB@ @QT5OPENGL_LIB@
gb_qt5_opengl_la_LDFLAGS = -module @LD_FLAGS@ @QT5OPENGL_LDFLAGS@
gb_qt5_opengl_la_CXXFLAGS = @THREAD_INC@ $(AM_CXXFLAGS) -std=c++11
gb_qt5_opengl_la_CPPFLAGS = @QT5OPENGL_INC@

gb_qt5_opengl_la_SOURCES = main.h main.cpp
 
if QT_OLD_OPENGL
gb_qt5_opengl_la_SOURCES += COldGLarea_moc.cpp COldGLarea.h COldGLarea.cpp
else
gb_qt5_opengl_la_SOURCES += CGLarea_moc.cpp CGLarea.h CGLarea.cpp
endif
 
 

 

