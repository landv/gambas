dnl ---- configure.ac for gb.qte

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.qte)
AC_PROG_LIBTOOL

AC_ARG_ENABLE(
  qte-translation,
  [  --enable-qt-translation             use qt/embedded translation files (default: yes)],
  gb_use_qt_translation=$enableval,
  gb_use_qt_translation=yes
)

if test "$gb_use_qt_translation" = "yes"; then
  AC_DEFINE(USE_QT_TRANSLATION, 1, [Use QT/Embedded translation])
fi

GB_FIND_QT_MOC

AC_DEFINE(NO_X_WINDOW, 1, [No X-Window system])

GB_COMPONENT(
  qte,
  QTE,
  [QT/Embedded component],
  [src],
  [GB_FIND(qptrlist.h, /usr/local/lib /usr/local /opt /usr/lib /usr/X11 /usr/X11R6 /usr/pkg /usr, include qt*/include qt/*/include)],
  [GB_FIND(libqte-mt.$SHLIBEXT, /usr/local /opt /usr/X11 /usr/X11R6 /usr/pkg /usr, lib qt*/lib qt/*/lib lib/qt*/lib, libqte-mt.so.3)],
  [$CXX_LIB $THREAD_LIB -lqte-mt],
  [$THREAD_INC])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/share/Makefile \
src/ext/Makefile \
)
