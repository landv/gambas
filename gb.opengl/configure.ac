dnl ---- configure.ac for gb.opengl

AC_INIT(configure.ac)
GB_INIT(gb.opengl)
AC_PROG_LIBTOOL

dnl x/glx is currently needed, will be optional when moved to other platform
AC_PATH_X
AC_PATH_XTRA
AC_CHECK_HEADER(GL/glx.h,
    AC_DEFINE(HAVE_X11_GLX, 1, [Target system is X11/glx]),
    [ AC_MSG_WARN([glx.h not found. Check 'config.log' for more details.])
    touch DISABLED])

GB_COMPONENT_PKG_CONFIG(
  opengl,
  OPENGL,
  [OpenGL support component],
  [src],
  gl glu
  )

AC_OUTPUT( \
Makefile \
src/Makefile \
)
