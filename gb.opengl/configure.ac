dnl ---- configure.ac for gb.opengl

AC_INIT(configure.ac)
GB_INIT(gb.opengl)
AC_PROG_LIBTOOL

GB_COMPONENT(
  opengl,
  OPENGL,
  [openGL],
  [src],
  [GB_FIND(gl.h glu.h, /usr /usr/X11R6 /usr/local, include include/GL)],
  [GB_FIND(libGL.$SHLIBEXT libGLU.$SHLIBEXT, /usr /usr/X11R6 /usr/local, lib)],
  [-lGL -lGLU])

AC_OUTPUT( \
Makefile \
src/Makefile \
)
