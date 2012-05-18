dnl ---- configure.ac for gb.sdl

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.sdl.sound)
AC_PROG_LIBTOOL

dnl TODO: add a version test in GB_COMPONENT!
GB_PATH_SDL(1.2.8, [] , [touch DISABLED])

dnl check for SDL libs
GB_COMPONENT(
  sdlsound, SDLSOUND, gb.sdl.sound, [src],
  [GB_FIND(SDL_mixer.h SDL_cdrom.h, `sdl-config --prefix`, include/SDL)],
  [GB_FIND(libSDL_mixer.$SHLIBEXT, /usr /usr/local `sdl-config --prefix`, lib)],
  [-lSDL_mixer],
  [$SDL_CFLAGS])

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
