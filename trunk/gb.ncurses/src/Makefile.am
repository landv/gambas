COMPONENT = gb.ncurses
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.ncurses.la

gb_ncurses_la_LIBADD = @NCURSES_LIB@
gb_ncurses_la_LDFLAGS = -module @LD_FLAGS@ @NCURSES_LDFLAGS@
gb_ncurses_la_CPPFLAGS = @NCURSES_INC@

gb_ncurses_la_SOURCES = \
 main.h main.c \
 c_window.h c_window.c \
 c_key.h c_key.c \
 c_color.h c_color.c \
 c_screen.h c_screen.c \
 c_input.h c_input.c

