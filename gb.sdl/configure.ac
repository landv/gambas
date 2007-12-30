dnl ---- configure.ac for gb.sdl

AC_INIT(configure.ac)
GB_INIT(gb.sdl)
AC_PROG_LIBTOOL

dnl TODO: add a version test in GB_COMPONENT!
dnl SDL_VERSION="1.2.8"
dnl AM_PATH_SDL($SDL_VERSION,,AC_MSG_ERROR([*** SDL >= $SDL_VERSION not found!]))

dnl check Xcursor support
GB_COMPONENT(
  Xcursor,
  XCURSOR,
  [Xcursor],
  [],
  [GB_FIND(Xcursor.h, /usr /usr/local /usr/X11R6, include/X11/Xcursor)],
  [GB_FIND(libXcursor.$SHLIBEXT, /usr /usr/local /usr/X11R6, lib lib/X11R6/)],
  [-lXcursor],
  [],
  ["Support for colorized cursors"])

GB_COMPONENT(
  sdl,
  SDL,
  [SDL],
  [src],
  [GB_FIND(SDL_opengl.h SDL.h SDL_image.h, `sdl-config --prefix`, include/SDL)],
  [GB_FIND(libSDL_image.$SHLIBEXT , /usr /usr/X11R6 /usr/local `sdl-config --prefix`, lib)],
  [`sdl-config --libs` -lSDL_image -lGL -lGLU],
  [`sdl-config --cflags`])

GB_COMPONENT(
  sdl_sound,
  SDL_SOUND,
  [SDL sound],
  [sound],
  [GB_FIND(SDL_mixer.h SDL_cdrom.h, `sdl-config --prefix`, include/SDL)],
  [GB_FIND(libSDL_mixer.$SHLIBEXT, /usr /usr/local `sdl-config --prefix`, lib)],
  [`sdl-config --libs` -lSDL_mixer],
  [`sdl-config --cflags`])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/sound/Makefile \
)
