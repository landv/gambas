dnl ---- configure.ac for gb.sge

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.sge)
AC_PROG_LIBTOOL


GB_CHECK_XWINDOW
GB_COMPONENT(
  sge,
  sge,
  gb.sge,
  [src],
  [GB_FIND(gl.h glu.h glew.h, /usr/local/lib /usr/local /usr/lib /usr, sge/include include sge*/include sge)],
  [GB_FIND(libGL.$SHLIBEXT libGLEW.$SHLIBEXT libGLU.$SHLIBEXT, /usr/local /usr, lib)],
  [$C_LIB $THREAD_LIB -ljpeg -lpng -lGL -lGLEW -lGLU],
  [$THREAD_INC -ljpeg -lpng -lGL -lGLEW -lGLU])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
