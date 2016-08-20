dnl ---- configure.ac for gb.sdl

m4_include([../version.m4])
AC_INIT(gambas3-gb-sdl, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.sdl)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW

dnl TODO: add a version test in GB_COMPONENT!
dnl GB_PATH_SDL([1.2.8], [], [touch DISABLED])

dnl check Xcursor support
dnl AC_CHECK_LIB(Xcursor, XcursorLibraryLoadCursor,
dnl   [echo -n
dnl   LDFLAGS="$LDFLAGS -lXcursor"], 
dnl   [AC_MSG_WARN([libXcursor not found. Check 'config.log' for more details.])
dnl   touch DISABLED],
dnl   $X_LIBS)

dnl check for SDL libs
dnl GB_COMPONENT(
dnl   sdl,
dnl   SDL,
dnl   [SDL],
dnl   [src],
dnl   [GB_FIND(SDL_opengl.h SDL.h SDL_ttf.h, `sdl-config --prefix`, include/SDL)],
dnl   [GB_FIND(libSDL_ttf.$SHLIBEXT libGLEW.$SHLIBEXT, /usr /usr/X11R6 /usr/local `sdl-config --prefix`, lib)],
dnl   [$X_LIBS -lSDL_ttf -lGLEW -lXcursor],
dnl   [$SDL_CFLAGS])

GB_COMPONENT_SEARCH(
   sdl, SDL, gb.sdl, [src],
   sdl "> 1.2.8" SDL_ttf glew xcursor,
   [GB_FIND(GL/glew.h SDL_ttf.h X11/Xcursor/Xcursor.h, /usr /usr/local `sdl-config --prefix`, include include/SDL)],
   [GB_FIND(libSDL_ttf.$SHLIBEXT libGLEW.$SHLIBEXT libXcursor.$SHLIBEXT, /usr /usr/local `sdl-config --prefix`, lib)],
   [$X_LIBS -lSDL_ttf -lGLEW -lXcursor],
   [$SDL_CFLAGS]
)

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
