EXTRA_DIST = gb.image.h gb.geom.h

gblib_LTLIBRARIES = gb.draw.la

gb_draw_la_LIBADD = @MATH_LIB@
gb_draw_la_LDFLAGS = -module @LD_FLAGS@ 
gb_draw_la_CFLAGS = -I$(top_srcdir)/share @INCLTDL@ $(AM_CFLAGS)

gb_draw_la_SOURCES = \
 gb.draw.h gb.paint.h \
 gb_list.c \
 cpaint.h cpaint.c \
 main.h main.c


