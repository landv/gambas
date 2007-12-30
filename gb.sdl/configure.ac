dnl ---- configure.ac for gb.sdl

AC_INIT(configure.ac)
GB_INIT(gb.sdl)
AC_PROG_LIBTOOL
AC_PATH_X
AC_PATH_XTRA

dnl TODO: add a version test in GB_COMPONENT!
AM_PATH_SDL(1.2.8, [rm -f DISABLED], [touch DISABLED])

if test -e DISABLED; then
  AC_MSG_WARN([SDL component is disabled])
  SDL_DIR=""
  AC_SUBST(SDL_DIR)
else
  dnl check Xcursor support
  AC_CHECK_LIB(Xcursor, XcursorLibraryLoadCursor,
    [echo -n
    LDFLAGS="$LDFLAGS -lXcursor"], 
    [AC_MSG_WARN([libXcursor not found. Check 'config.log' for more details.])
    touch DISABLED],
    $X_LIBS)

  if test -e DISABLED; then
    AC_MSG_WARN([SDL component is disabled])
    SDL_DIR=""
    AC_SUBST(SDL_DIR)
  else
    GB_COMPONENT(
      sdl,
      SDL,
      [SDL],
      [src],
      [GB_FIND(SDL_opengl.h SDL.h SDL_image.h, `sdl-config --prefix`, include/SDL)],
      [GB_FIND(libSDL_image.$SHLIBEXT , /usr /usr/X11R6 /usr/local `sdl-config --prefix`, lib)],
      [$SDL_LIBS -lSDL_image -lGL -lGLU],
      [$SDL_CFLAGS])
  fi
fi

AC_OUTPUT( \
Makefile \
src/Makefile \
)
