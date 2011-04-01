COMPONENT = gb.opengl.glu
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.opengl.glu.la
INCLUDES = @GLU_INC@

gb_opengl_glu_la_LIBADD = @GLU_LIB@
gb_opengl_glu_la_LDFLAGS = -module @LD_FLAGS@ @GLU_LDFLAGS@

gb_opengl_glu_la_SOURCES = \
 main.h main.c \
 GLU.h GLU.c \
 GLUcoordTransf.h GLUcoordTransf.c \
 GLUtextureImage.h GLUtextureImage.c

