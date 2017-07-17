COMPONENT = gb.openal
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.openal.la

gb_openal_la_LIBADD = @OPENAL_LIB@
gb_openal_la_LDFLAGS = -module @LD_FLAGS@ @OPENAL_LDFLAGS@
gb_openal_la_CPPFLAGS = @OPENAL_INC@

gb_openal_la_SOURCES = main.c main.h c_al.h c_al.c c_alc.h c_alc.c c_alure.h c_alure.c
