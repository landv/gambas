COMPONENT = gb.sdl2
include $(top_srcdir)/component.am

SUBDIRS = . @SDL2AUDIO_DIR@

gblib_LTLIBRARIES = gb.sdl2.la

gb_sdl2_la_LIBADD = @SDL2_LIB@ libfont.la
gb_sdl2_la_LDFLAGS = -module @LD_FLAGS@ @SDL2_LDFLAGS@
gb_sdl2_la_CFLAGS = $(AM_CFLAGS)
gb_sdl2_la_CPPFLAGS = @SDL2_INC@

gb_sdl2_la_SOURCES = \
 c_image.h c_image.c \
 c_window.h c_window.c \
 c_draw.h c_draw.c \
 c_mouse.h c_mouse.c \
 c_key.h c_key.c \
 c_font.h c_font.c \
 main.h main.c

noinst_LTLIBRARIES = libfont.la

libfont_la_CPPFLAGS = @SDL2_INC@
libfont_la_CFLAGS = $(AM_CFLAGS_OPT)

libfont_la_SOURCES = \
 default_font_data.h default_font.h default_font.c

