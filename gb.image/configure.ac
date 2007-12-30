dnl ---- configure.ac for gb.image

AC_INIT(configure.ac)
GB_INIT(gb.image)
AC_PROG_LIBTOOL
GB_COMPONENT(
  image,
  IMAGE,
  [Image processing component],
  [src],
  [],
  [],
  [$THREAD_LIB -lm],
  [$THREAD_INC])
AC_OUTPUT( Makefile src/Makefile )
