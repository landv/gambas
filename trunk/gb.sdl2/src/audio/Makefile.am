COMPONENT = gb.sdl2.audio
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sdl2.audio.la

gb_sdl2_audio_la_LIBADD = @SDL2AUDIO_LIB@
gb_sdl2_audio_la_LDFLAGS = -module @LD_FLAGS@ @SDL2AUDIO_LDFLAGS@
gb_sdl2_audio_la_CFLAGS = $(AM_CFLAGS)
gb_sdl2_audio_la_CPPFLAGS = @SDL2AUDIO_INC@

gb_sdl2_audio_la_SOURCES = \
 c_sound.h c_sound.c \
 c_music.h c_music.c \
 c_channel.h c_channel.c \
 main.h main.c
