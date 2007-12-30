COMPONENT = gb.opengl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.opengl.la
INCLUDES = @OPENGL_INC@

gb_opengl_la_LIBADD = @OPENGL_LIB@
gb_opengl_la_LDFLAGS = -module @LD_FLAGS@

gb_opengl_la_SOURCES = \
 main.h main.c \
 GLinfo.h GLinfo.c \
 GL.h GL.c \
 GLclipping.c GLclipping.h \
 GLcolorLighting.h GLcolorLighting.c \
 GLconstants.h GLconstants.c \
 GLcoordTransf.h  GLcoordTransf.c \
 GLdisplayList.h GLdisplayList.c \
 GLfog.h GLfog.c \
 GLframeBufferOps.h GLframeBufferOps.c \
 GLmodesExec.h GLmodesExec.c \
 GLprimitives.h GLprimitives.c \
 GLpixelOperations.h GLpixelOperations.c \
 GLrasterization.h GLrasterization.c \
 GLtextureMapping.h GLtextureMapping.c \
 GLselectPixmap.h GLselectPixmap.c \
 GLU.h GLU.c \
 GLUcoordTransf.h GLUcoordTransf.c \
 GLUtextureImage.h GLUtextureImage.c
