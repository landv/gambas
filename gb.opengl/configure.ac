dnl ---- configure.ac for gb.opengl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.opengl)
AC_PROG_LIBTOOL

GB_COMPONENT(
  opengl,
  OPENGL,
  [OpenGL support component],
  [src],
  [GB_FIND(gl.h glu.h glew.h, /opt/local /usr /usr/X11R6 /usr/local, include/GL include)],
  [GB_FIND(libGL.$SHLIBEXT libGLU.$SHLIBEXT libGLEW.$SHLIBEXT, /opt/local /usr /usr/X11R6 /usr/local, lib)],
  [-lGL -lGLU -lGLEW])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/glsl/Makefile \
)
