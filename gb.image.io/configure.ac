dnl ---- configure.ac for gb.image.io

m4_include([../version.m4])
AC_INIT(gambas3-gb-image-io, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.image.io)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  image_io, IMAGE_IO, gb.image.io, [src],
  'gdk-pixbuf-2.0 >= 2.4.13'
  )

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
