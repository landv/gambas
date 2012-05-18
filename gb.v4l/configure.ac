dnl ---- configure.ac for gb.v4l

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.v4l)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  v4lconvert, V4LCONVERT, gb.v4l, [src],
  libv4lconvert)

GB_COMPONENT(
  v4l, V4L, gb.v4l, [src],
  [GB_FIND(png.h jpeglib.h linux/videodev2.h, /usr/local/lib /usr/local /usr/lib /usr, include)],
  [GB_FIND(libpng.$SHLIBEXT libjpeg.$SHLIBEXT, /usr/local /usr, lib)],
  [$C_LIB -ljpeg -lpng])

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
