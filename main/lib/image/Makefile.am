COMPONENT = gb.image
include $(top_srcdir)/component.am

INCLUDES = -I$(top_srcdir)/share @INCLTDL@

gblib_LTLIBRARIES = gb.image.la

gb_image_la_LIBADD = 
gb_image_la_LDFLAGS = -module @LD_FLAGS@

gb_image_la_SOURCES = \
 image_stat.h image_stat.c \
 CImageStat.h CImageStat.c \
 gb.image.h \
 main.h main.c


