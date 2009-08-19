COMPONENT = gb.sdl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sdl.la
INCLUDES = @SDL_INC@

gb_sdl_la_LIBADD = @SDL_LIB@
gb_sdl_la_LDFLAGS = -module @LD_FLAGS@ @SDL_LDFLAGS@

gb_sdl_la_SOURCES = \
 SDL_h.h \
 SDLapp.h SDLapp.cpp \
 SDLcore.h SDLcore.cpp \
 SDLdebug.h SDLdebug.cpp \
 SDLerror.h SDLerror.cpp \
 SDLfont.h SDLfont.cpp \
 SDLgfx.h SDLgfx.cpp \
 SDLgl.h SDLgl.cpp \
 SDLcursor.h SDLcursor.cpp \
 SDLosrender.h SDLosrender.cpp \
 SDLsurface.h SDLsurface.cpp \
 SDLtexture.h SDLtexture.cpp \
 SDLwindow.h SDLwindow.cpp \
 Cconst.h Cconst.cpp \
 Cdesktop.h Cdesktop.cpp \
 Cdraw.h Cdraw.cpp \
 Cfont.h Cfont.cpp \
 Cimage.h Cimage.cpp \
 Ckey.h Ckey.cpp \
 Cmouse.h Cmouse.cpp \
 Cwindow.h Cwindow.cpp \
 main.h main.cpp
