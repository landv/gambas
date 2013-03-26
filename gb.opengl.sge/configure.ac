dnl ---- configure.ac for gb.opengl.sge

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.opengl.sge)
AC_PROG_LIBTOOL


GB_CHECK_XWINDOW
GB_COMPONENT(
  opengl_sge,
  OPENGL_SGE,
  gb.opengl.sge,
  [src],
  [GB_FIND(gl.h glu.h glew.h, /usr/local/lib /usr/local /usr/lib /usr, opengl.sge/include include opengl.sge*/include opengl.sge)],
  [GB_FIND(libGL.$SHLIBEXT libGLEW.$SHLIBEXT libGLU.$SHLIBEXT, /usr/local /usr, lib)],
  [$C_LIB $THREAD_LIB -ljpeg -lpng -lGL -lGLEW -lGLU],
  [$THREAD_INC -ljpeg -lpng -lGL -lGLEW -lGLU])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
