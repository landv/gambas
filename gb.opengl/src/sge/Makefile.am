COMPONENT = gb.opengl.sge
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.opengl.sge.la

gb_opengl_sge_la_LIBADD = @SGE_LIB@
gb_opengl_sge_la_LDFLAGS = -module @LD_FLAGS@ @SGE_LDFLAGS@
gb_opengl_sge_la_CPPFLAGS = @SGE_INC@

gb_opengl_sge_la_SOURCES = \
	main.c main.h \
	cmd2model.c cmd2model.h \
	cmd2object.c cmd2object.h
