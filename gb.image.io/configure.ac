dnl ---- configure.ac for gb.image.io

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.image.io)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  image_io,
  IMAGE_IO,
  [Image loading and saving],
  [src],
  gdk-pixbuf-2.0 ">= 2.4.13"
  )

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
