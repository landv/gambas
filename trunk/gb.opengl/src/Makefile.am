COMPONENT = gb.opengl
include $(top_srcdir)/component.am

SUBDIRS = . @GLSL_DIR@ @GLU_DIR@ @SGE_DIR@

gblib_LTLIBRARIES = gb.opengl.la

gb_opengl_la_LIBADD = @OPENGL_LIB@
gb_opengl_la_LDFLAGS = -module @LD_FLAGS@ @OPENGL_LDFLAGS@
gb_opengl_la_CPPFLAGS = @OPENGL_INC@

gb_opengl_la_SOURCES = \
 main.h main.c gb.gl.h \
 GL.h GL.c \
 GLinfo.h GLinfo.c \
 GLclipping.c GLclipping.h \
 GLcolorLighting.h GLcolorLighting.c \
 GLcoordTransf.h  GLcoordTransf.c \
 GLdisplayList.h GLdisplayList.c \
 GLeval.h GLeval.c \
 GLfog.h GLfog.c \
 GLframeBufferOps.h GLframeBufferOps.c \
 GLmodesExec.h GLmodesExec.c \
 GLprimitives.h GLprimitives.c \
 GLpixelOperations.h GLpixelOperations.c \
 GLrasterization.h GLrasterization.c \
 GLtextureMapping.h GLtextureMapping.c \
 GLselectFeedback.h GLselectFeedback.c \
 framebufferobject.h framebufferobject.c
