COMPONENT = gb.opengl
include $(top_srcdir)/component.am

SUBDIRS = glsl

gblib_LTLIBRARIES = gb.opengl.la
INCLUDES = @OPENGL_INC@

gb_opengl_la_LIBADD = @OPENGL_LIB@
gb_opengl_la_LDFLAGS = -module @LD_FLAGS@ @OPENGL_LDFLAGS@

gb_opengl_la_SOURCES = \
 main.h main.c gb.gl.h gb.gl.c \
 GL.h GL.c \
 GLinit.c GLinit.h \
 GLinfo.h GLinfo.c \
 GLclipping.c GLclipping.h \
 GLcolorLighting.h GLcolorLighting.c \
 GLcoordTransf.h  GLcoordTransf.c \
 GLdisplayList.h GLdisplayList.c \
 GLfog.h GLfog.c \
 GLframeBufferOps.h GLframeBufferOps.c \
 GLmodesExec.h GLmodesExec.c \
 GLprimitives.h GLprimitives.c \
 GLpixelOperations.h GLpixelOperations.c \
 GLrasterization.h GLrasterization.c \
 GLtextureMapping.h GLtextureMapping.c \
 GLselectFeedback.h GLselectFeedback.c \
 GLU.h GLU.c \
 GLUcoordTransf.h GLUcoordTransf.c \
 GLUtextureImage.h GLUtextureImage.c

