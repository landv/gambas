dnl ---- configure.ac for gb.sdl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.sdl)

AC_PROG_LIBTOOL
GB_CHECK_XWINDOW

dnl TODO: add a version test in GB_COMPONENT!
GB_PATH_SDL([1.2.8], [], [touch DISABLED])

dnl check Xcursor support
AC_CHECK_LIB(Xcursor, XcursorLibraryLoadCursor,
  [echo -n
  LDFLAGS="$LDFLAGS -lXcursor"], 
  [AC_MSG_WARN([libXcursor not found. Check 'config.log' for more details.])
  touch DISABLED],
  $X_LIBS)

dnl check Xft support
AC_CHECK_LIB(Xft, XftListFonts,
  [echo -n
  LDFLAGS="$LDFLAGS -lXft" SDL_CFLAGS="$SDL_CFLAGS `freetype-config --cflags`"], 
  [AC_MSG_WARN([libXft not found. Check 'config.log' for more details.])
  touch DISABLED],
  $X_LIBS)

dnl check for SDL libs
GB_COMPONENT(
  sdl,
  SDL,
  [SDL],
  [src],
  [GB_FIND(SDL_opengl.h SDL.h SDL_image.h SDL_ttf.h, `sdl-config --prefix`, include/SDL)],
  [GB_FIND(libSDL_ttf.$SHLIBEXT libGLEW.$SHLIBEXT, /usr /usr/X11R6 /usr/local `sdl-config --prefix`, lib)],
  [$X_LIBS -lSDL_ttf -lGLU -lGLEW],
  [$SDL_CFLAGS])

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
