COMPONENT = gb.image.effect
include $(top_srcdir)/component.am

noinst_LTLIBRARIES = libimageeffect.la
gblib_LTLIBRARIES = gb.image.effect.la

libimageeffect_la_LIBADD =
libimageeffect_la_LDFLAGS = -module @LD_FLAGS@
libimageeffect_la_CXXFLAGS = -I$(top_srcdir)/share $(AM_CXXFLAGS_OPT)

libimageeffect_la_SOURCES = \
 kimageeffect.h kimageeffect.cpp \
 effect.h effect.cpp

gb_image_effect_la_LIBADD = libimageeffect.la
gb_image_effect_la_LDFLAGS = -module @LD_FLAGS@
gb_image_effect_la_CXXFLAGS = -I$(top_srcdir)/share $(AM_CXXFLAGS)

gb_image_effect_la_SOURCES = \
main.cpp main.h qt.h \
kcpuinfo.h kcpuinfo.cpp \
qcolor.h qcolor.cpp \
qpoint.h qpoint.cpp \
qsize.h qsize.cpp \
qrect.h qrect.cpp \
qimage.h qimage.cpp \
CImage.h CImage.cpp


