COMPONENT = gb.image.io
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.image.io.la

gb_image_io_la_LIBADD = @IMAGE_IO_LIB@
gb_image_io_la_LDFLAGS = -module @LD_FLAGS@ @IMAGE_IO_LDFLAGS@
gb_image_io_la_CPPFLAGS = @IMAGE_IO_INC@

gb_image_io_la_SOURCES = main.c main.h c_image.c c_image.h
