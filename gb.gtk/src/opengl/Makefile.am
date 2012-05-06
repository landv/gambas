COMPONENT = gb.gtk.opengl
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.gtk.opengl.la

gb_gtk_opengl_la_LIBADD = @GTK_LIB@ @GTKOPENGL_LIB@
gb_gtk_opengl_la_LDFLAGS = -module @LD_FLAGS@ @GTK_LDFLAGS@ @GTKOPENGL_LDFLAGS@
gb_gtk_opengl_la_CPPFLAGS = @GTK_INC@ @GTKOPENGL_INC@

gb_gtk_opengl_la_SOURCES = \
 main.h main.c \
 c_glarea.h c_glarea.c

