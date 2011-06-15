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
  [GB_FIND(gl.h glew.h, /opt/local /usr /usr/X11R6 /usr/local, include/GL include)],
  [GB_FIND(libGL.$SHLIBEXT libGLEW.$SHLIBEXT, /opt/local /usr /usr/X11R6 /usr/local, lib)],
  [-lGL -lGLEW])

GB_COMPONENT(
  glu,
  GLU,
  [OpenGL Utility support component],
  [glu],
  [GB_FIND(gl.h glu.h, /opt/local /usr /usr/X11R6 /usr/local, include/GL include)],
  [GB_FIND(libGL.$SHLIBEXT libGLU.$SHLIBEXT, /opt/local /usr /usr/X11R6 /usr/local, lib)],
  [-lGLU])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/glu/Makefile \
src/glsl/Makefile \
)

GB_PRINT_MESSAGES
