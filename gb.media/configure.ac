dnl ---- configure.ac for gb.media

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.media)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  media, MEDIA, gb.media, [src],
  gstreamer-0.10 gstreamer-interfaces-0.10
)
AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES