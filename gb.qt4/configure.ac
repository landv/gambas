dnl ---- configure.ac for gb.qt

AC_INIT(configure.ac)
GB_INIT(gb.qt4)
AC_PROG_LIBTOOL

AC_ARG_ENABLE(
  qt-translation,
  [  --enable-qt-translation             use qt translation files (default: yes)],
  gb_use_qt_translation=$enableval,
  gb_use_qt_translation=yes
)

if test "$gb_use_qt_translation" = "yes"; then
  AC_DEFINE(USE_QT_TRANSLATION, 1, [Use QT translation])
fi

GB_CHECK_XWINDOW

GB_COMPONENT_PKG_CONFIG(
  qt,
  QT,
  [QT4 toolkit],
  [src],
  QtCore ">= 4.3.4" QtGui Qt3Support
)

GB_COMPONENT_PKG_CONFIG(
  qtext,
  QTEXT,
  [QT4 toolkit extension],
  [ext],
  QtCore ">= 4.3.4" QtGui Qt3Support
)

GB_COMPONENT(
  opengl,
  OPENGL,
  [OpenGL libraries],
  [],
  [GB_FIND(gl.h glu.h, /usr /usr/X11R6 /usr/local, include/GL)],
  [GB_FIND(libGL.$SHLIBEXT libGLU.$SHLIBEXT, /usr /usr/X11R6 /usr/local, lib)],
  [-lGL -lGLU])

GB_COMPONENT_PKG_CONFIG(
  qtopengl,
  QTOPENGL,
  [QT4 OpenGL],
  [opengl],
  QtOpenGL ">= 4.3.4"
)

MOC=`pkg-config --variable=moc_location QtCore`
AC_SUBST(MOC)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/ext/Makefile \
src/opengl/Makefile \
)
