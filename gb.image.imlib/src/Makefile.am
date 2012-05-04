COMPONENT = gb.image.imlib
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.image.imlib.la

gb_image_imlib_la_LIBADD = @IMAGE_IMLIB_LIB@
gb_image_imlib_la_LDFLAGS = -module @LD_FLAGS@ @IMAGE_IMLIB_LDFLAGS@
gb_image_imlib_la_CPPFLAGS = @IMAGE_IMLIB_INC@

gb_image_imlib_la_SOURCES = main.c main.h c_image.c c_image.h c_imlib.c c_imlib.h
