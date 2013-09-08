dnl ---- configure.ac for gb.opengl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.opengl)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  opengl, OPENGL, gb.opengl, [src],
  [gl glew])

GB_COMPONENT_PKG_CONFIG(
  glsl, GLSL, gb.opengl.glsl, [glsl],
  [gl glew])

GB_COMPONENT_PKG_CONFIG(
  glu, GLU, gb.opengl.glu, [glu],
  [glu])

GB_COMPONENT_PKG_CONFIG(
  sge, SGE, gb.opengl.sge, [sge],
	[gl glew])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/glu/Makefile \
src/glsl/Makefile \
src/sge/Makefile
)

GB_PRINT_MESSAGES
