dnl ---- configure.ac for gb.sdl

AC_INIT(configure.ac)
GB_INIT(gb.sdl.sound)
AC_PROG_LIBTOOL

dnl TODO: add a version test in GB_COMPONENT!
AM_PATH_SDL(1.2.8, , [touch DISABLED])

dnl check for SDL libs
GB_COMPONENT(
  sdl_sound,
  SDL_SOUND,
  [SDL sound],
  [src],
  [GB_FIND(SDL_mixer.h SDL_cdrom.h, `sdl-config --prefix`, include/SDL)],
  [GB_FIND(libSDL_mixer.$SHLIBEXT, /usr /usr/local `sdl-config --prefix`, lib)],
  [$SDL_LIBS -lSDL_mixer],
  [$SDL_CFLAGS])

AC_OUTPUT( \
Makefile \
src/Makefile \
)
