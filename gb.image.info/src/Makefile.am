COMPONENT = gb.image.info
include $(top_srcdir)/component.am

INCLUDES = @IMAGE_INFO_INC@

gblib_LTLIBRARIES = gb.image.info.la

gb_image_info_la_LIBADD = @IMAGE_INFO_LIB@
gb_image_info_la_LDFLAGS = -module @LD_FLAGS@

gb_image_info_la_SOURCES = main.h main.c image.h image.c CImageStat.h CImageStat.c
