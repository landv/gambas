COMPONENT = gb.sdl2
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sdl2.la

gb_sdl2_la_LIBADD = @SDL2_LIB@
gb_sdl2_la_LDFLAGS = -module @LD_FLAGS@ @SDL2_LDFLAGS@
gb_sdl2_la_CXXFLAGS = $(AM_CXXFLAGS) -DDATA_DIR=\"$(DESTDIR)$(gbdatadir)/$(COMPONENT)\"
gb_sdl2_la_CPPFLAGS = @SDL2_INC@

gb_sdl2_la_SOURCES = \
 c_image.h c_image.c \
 c_window.h c_window.c \
 c_draw.h c_draw.c \
 main.h main.c
