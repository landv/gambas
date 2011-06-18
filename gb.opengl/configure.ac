dnl ---- configure.ac for gb.opengl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.opengl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  opengl,
  OPENGL,
  [OpenGL support component],
  [src],
  [gl glew])

GB_COMPONENT_PKG_CONFIG(
  glu,
  GLU,
  [OpenGL Utility support component],
  [glu],
  [glu])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/glu/Makefile \
src/glsl/Makefile \
)

GB_PRINT_MESSAGES
