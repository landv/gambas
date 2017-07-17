dnl ---- configure.ac for gb.image.imlib

m4_include([../version.m4])
AC_INIT(gambas3-gb-image-imlib, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.image.imlib)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  image_imlib, IMAGE_IMLIB, gb.image.imlib, [src],
  'imlib2 >= 1.4.0'
  )

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
