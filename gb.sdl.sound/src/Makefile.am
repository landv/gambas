COMPONENT = gb.sdl.sound
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sdl.sound.la
INCLUDES = @SDL_SOUND_INC@

gb_sdl_sound_la_LIBADD = @SDL_SOUND_LIB@
gb_sdl_sound_la_LDFLAGS = -module @LD_FLAGS@

gb_sdl_sound_la_SOURCES = \
 main.h main.c \
 sound.h sound.c \
 cdrom.h cdrom.c
