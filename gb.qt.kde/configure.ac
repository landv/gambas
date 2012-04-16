dnl ---- configure.ac for gb.qt.kde

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.qt.kde)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

GB_FIND_QT_MOC

GB_COMPONENT(
  qt,
  QT,
  [QT component],
  [src],
  [GB_FIND(qptrlist.h, /usr/local/lib /usr/local /opt /usr/lib /usr/X11 /usr/X11R6 /usr/pkg /usr, include qt*/include qt/*/include)],
  [GB_FIND(libqt-mt.$SHLIBEXT, /usr/local /opt /usr/X11 /usr/X11R6 /usr/pkg /usr, lib qt*/lib qt/*/lib, libqt-mt.so.3)],
  [$CXX_LIB $THREAD_LIB $X_LIBS -lqt-mt],
  [$THREAD_INC])

if test "$have_qt" != "no"; then

  GB_COMPONENT(
    kde,
    KDE,
    [KDE 3.x component],
    [src],
    [GB_FIND(kapplication.h, `kde-config --prefix 2> /dev/null`, include)],
    [GB_FIND(libkdecore.$SHLIBEXT, `kde-config --prefix 2> /dev/null`, lib)],
    [-lkdecore -lkdeui -lDCOP -lkio])

  if test "$have_kde" == "no"; then

    PATH=$PATH:/opt/trinity/bin
    rm -f DISABLED
  
    GB_COMPONENT(
      trinity,
      TRINITY,
      [Trinity],
      [src],
      [GB_FIND(tqstring.h, /usr/local /usr /opt, include include/tqt tqt/include)],
      [GB_FIND(libtqt.$SHLIBEXT, /usr/local /usr, lib lib/tqt tqt/lib)],
      [-ltqt])

  fi

fi
  
AC_OUTPUT( \
Makefile \
src/Makefile \
src/html/Makefile \
)
