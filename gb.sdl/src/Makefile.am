COMPONENT = gb.sdl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sdl.la
INCLUDES = @SDL_INC@

gb_sdl_la_LIBADD = @SDL_LIB@
gb_sdl_la_LDFLAGS = -module @LD_FLAGS@ @SDL_LDFLAGS@

gb_sdl_la_SOURCES = \
 SDL_h.h \
 SDLcore.h SDLcore.cpp \
 SDLerror.h SDLerror.cpp \
 SDLsurface.h SDLsurface.cpp \
 SDLwindow.h SDLwindow.cpp \
 SDLgfx.h SDLgfx.cpp \
 SDLapp.h SDLapp.cpp \
 SDLcursor.h SDLcursor.cpp \
 SDLfont.h SDLfont.cpp \
 main.h main.cpp \
 timer.h timer.cpp \
 Cconst.h Cconst.cpp \
 Ccolor.h Ccolor.cpp \
 Cwindow.h Cwindow.cpp \
 Cdesktop.h Cdesktop.cpp \
 Cimage.h Cimage.cpp \
 Cdraw.h Cdraw.cpp \
 Ckey.h Ckey.cpp \
 Cmouse.h Cmouse.cpp \
 Cfont.h Cfont.cpp


