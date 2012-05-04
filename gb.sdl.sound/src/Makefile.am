COMPONENT = gb.sdl.sound
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sdl.sound.la

gb_sdl_sound_la_LIBADD = @SDLSOUND_LIB@
gb_sdl_sound_la_LDFLAGS = -module @LD_FLAGS@ @SDLSOUND_LDFLAGS@
gb_sdl_sound_la_CPPFLAGS = @SDLSOUND_INC@

gb_sdl_sound_la_SOURCES = \
 main.h main.c \
 sound.h sound.c \
 cdrom.h cdrom.c
