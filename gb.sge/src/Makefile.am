COMPONENT = gb.sge
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.sge.la

gb_sge_la_LIBADD = @sge_LIB@
gb_sge_la_LDFLAGS = -module @LD_FLAGS@ @sge_LDFLAGS@
gb_sge_la_CPPFLAGS = @sge_INC@

gb_sge_la_SOURCES = \
	main.c main.h \
	cmd2model.c cmd2model.h
