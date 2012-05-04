COMPONENT = gb.v4l
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.v4l.la

gb_v4l_la_LIBADD = @V4L_LIB@ @V4LCONVERT_LIB@
gb_v4l_la_LDFLAGS = -module @LD_FLAGS@ @V4L_LDFLAGS@ @V4LCONVERT_LDFLAGS@
gb_v4l_la_CPPFLAGS = @V4L_INC@ @V4LCONVERT_INC@

gb_v4l_la_SOURCES = main.h main.c CWebcam.h CWebcam.c gv4l2.c CConverters.c videodev.h



 
