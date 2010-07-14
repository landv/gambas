dnl ---- configure.ac for gb.image.imlib

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.image.imlib)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  image_imlib,
  IMAGE_IMLIB,
  [Image routines based on the Imlib2 library],
  [src],
  imlib2 ">= 1.4.0"
  )

AC_OUTPUT( Makefile src/Makefile )

GB_PRINT_MESSAGES
