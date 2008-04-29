dnl ---- configure.ac for gb.qt.kde

AC_INIT(configure.ac)
GB_INIT(gb.qt.kde)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

GB_FIND_QT_MOC

GB_COMPONENT_SEARCH(
  qt,
  QT,
  [QT component],
  [src],
  qt-mt ">= 3.3",
  [GB_FIND(qptrlist.h, /usr/local/lib /usr/local /opt /usr/lib /usr/X11 /usr/X11R6 /usr/pkg /usr, include qt*/include qt/*/include)],
  [GB_FIND(libqt-mt.$SHLIBEXT, /usr/local /opt /usr/X11 /usr/X11R6 /usr/pkg /usr, lib qt*/lib qt/*/lib, libqt-mt.$SHLIBEXT.3)],
  [$CXX_LIB $THREAD_LIB $X_LIBS -lqt-mt],
  [$THREAD_INC])

if test x"$have_qt" != "x"; then
  
GB_COMPONENT(
  kde,
  KDE,
  [KDE 3.x component],
  [src],
  [GB_FIND(kapplication.h, `kde-config --prefix 2> /dev/null`, include)],
  [GB_FIND(libkdecore.$SHLIBEXT, `kde-config --prefix 2> /dev/null`, lib)],
  [-lkdecore -lkdeui -lDCOP -lkio])

fi
  
AC_OUTPUT( \
Makefile \
src/Makefile \
src/html/Makefile \
)
