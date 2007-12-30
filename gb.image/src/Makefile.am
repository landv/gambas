COMPONENT = gb.image
include $(top_srcdir)/component.am

INCLUDES = @IMAGE_INC@

gblib_LTLIBRARIES = gb.image.la

gb_image_la_LIBADD = @IMAGE_LIB@
gb_image_la_LDFLAGS = -module @LD_FLAGS@
gb_image_la_CXXFLAGS = $(AM_CXXFLAGS) -O2

gb_image_la_SOURCES = main.cpp main.h qt.h \
kcpuinfo.h kcpuinfo.cpp \
qcolor.h qcolor.cpp \
qpoint.h qpoint.cpp \
qsize.h qsize.cpp \
qrect.h qrect.cpp \
qimage.h qimage.cpp \
kimageeffect.h kimageeffect.cpp \
effect.h effect.cpp \
CImage.h CImage.cpp
