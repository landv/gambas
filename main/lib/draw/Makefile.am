EXTRA_DIST = gb.image.h

gblib_LTLIBRARIES = gb.draw.la

gb_draw_la_LIBADD = @MATH_LIB@
gb_draw_la_LDFLAGS = -module @LD_FLAGS@ 
gb_draw_la_CFLAGS = -I$(top_srcdir)/share @INCLTDL@ $(AM_CFLAGS)

gb_draw_la_SOURCES = \
 gb.draw.h \
 gb_list.c \
 matrix.h matrix.c \
 CDraw.h CDraw.c \
 main.h main.c


