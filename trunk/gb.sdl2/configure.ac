dnl ---- configure.ac for gb.sdl

m4_include([../version.m4])
AC_INIT(gambas3-gb-sdl2, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.sdl2)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
   sdl2, SDL2, gb.sdl2, [src],
   sdl2 ">= 2.0.2" SDL2_image ">= 2.0.0" SDL2_ttf ">= 2.0.12"
)

GB_COMPONENT_PKG_CONFIG(
   sdl2audio, SDL2AUDIO, gb.sdl2.audio, [audio],
   sdl2 ">= 2.0.2" SDL2_mixer ">= 2.0.0"
)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/audio/Makefile \
)

GB_PRINT_MESSAGES
