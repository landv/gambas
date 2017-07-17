COMPONENT = gb.opengl.glsl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.opengl.glsl.la

gb_opengl_glsl_la_LIBADD = @OPENGL_LIB@
gb_opengl_glsl_la_LDFLAGS = -module @LD_FLAGS@ @OPENGL_LDFLAGS@
gb_opengl_glsl_la_CPPFLAGS = @OPENGL_INC@

gb_opengl_glsl_la_SOURCES = \
 main.h main.c \
 GLprogram.h GLprogram.c \
 GLshader.h GLshader.c \
 GLuniform.h GLuniform.c \
 GLattributes.h GLattributes.c \
 GL.h GL.c

