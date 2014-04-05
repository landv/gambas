dnl ---- configure.ac for gb.qt

m4_include([../version.m4])
AC_INIT(gambas3-gb-qt4, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

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
AM_CONDITIONAL(XWINDOW, test x"$have_x" = xyes)

GB_COMPONENT_PKG_CONFIG(
  qt, QT, gb.qt4, [src],
  'QtCore >= 4.5.0' QtGui Qt3Support QtSvg
)

GB_COMPONENT_PKG_CONFIG(
  qtext, QTEXT, gb.qt4.ext, [ext],
  'QtCore >= 4.5.0' QtGui Qt3Support
)

GB_COMPONENT_PKG_CONFIG(
  qtwebkit, QTWEBKIT, gb.qt4.webkit, [webkit],
  'QtCore >= 4.5.0' QtGui QtNetwork QtDBus QtXml 'QtWebKit >= 4.5.0'
)

GB_COMPONENT_PKG_CONFIG(
  qtopengl, QTOPENGL, gb.qt4.opengl, [opengl],
  'QtCore >= 4.5.0' QtGui QtOpenGL
)

MOC=`pkg-config --variable=moc_location QtCore`
AC_SUBST(MOC)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/ext/Makefile \
src/webkit/Makefile \
src/opengl/Makefile \
)

GB_PRINT_MESSAGES
