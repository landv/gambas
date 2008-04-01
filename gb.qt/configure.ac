dnl ---- configure.ac for gb.qt

AC_INIT(configure.ac)
GB_INIT(gb.qt)
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

GB_FIND_QT_MOC

GB_COMPONENT(
  qt,
  QT,
  [QT component],
  [src],
  [GB_FIND(qptrlist.h X11/Xlib.h, /usr/local/lib /usr/local /opt /usr/lib /usr/X11 /usr/X11R6 /usr/pkg /usr, include qt*/include qt/*/include)],
  [GB_FIND(libqt-mt.$SHLIBEXT, /usr/local /opt /usr/X11 /usr/X11R6 /usr/pkg /usr, lib qt*/lib qt/*/lib, libqt-mt.so.3)],
  [$CXX_LIB $THREAD_LIB $X_LIBS -lqt-mt],
  [$THREAD_INC])

GB_COMPONENT(
  qtopengl,
  QTOPENGL,
  [QT OpenGL component],
  [opengl],
  [GB_FIND(gl.h glu.h, /usr /usr/X11R6 /usr/local, include/GL)],
  [GB_FIND(libGL.$SHLIBEXT libGLU.$SHLIBEXT, /usr /usr/X11R6 /usr/local, lib)],
  [-lGL -lGLU])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/share/Makefile \
src/ext/Makefile \
src/opengl/Makefile \
)
