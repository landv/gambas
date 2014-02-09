COMPONENT = gb.image
include $(top_srcdir)/component.am

noinst_LTLIBRARIES = libimage.la
gblib_LTLIBRARIES = gb.image.la

libimage_la_LIBADD =
libimage_la_LDFLAGS = -module @LD_FLAGS@
libimage_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS_OPT)

libimage_la_SOURCES = \
 image.h image.c

gb_image_la_LIBADD = libimage.la @MATH_LIB@
gb_image_la_LDFLAGS = -module @LD_FLAGS@
gb_image_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_image_la_SOURCES = \
 image_stat.h image_stat.c \
 CImage.h CImage.c \
 CImageStat.h CImageStat.c \
 c_color.h c_color.c \
 gb.image.h \
 main.h main.c
