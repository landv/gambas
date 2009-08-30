dnl ---- configure.ac for gb.opengl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.opengl)
AC_PROG_LIBTOOL

dnl x/glx is currently needed, will be optional when moved to other platform
AC_PATH_X
AC_PATH_XTRA
AC_CHECK_HEADER(GL/glx.h,
    AC_DEFINE(HAVE_X11_GLX, 1, [Target system is X11/glx]),
    [ AC_MSG_WARN([glx.h not found. Check 'config.log' for more details.])
    touch DISABLED])

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
