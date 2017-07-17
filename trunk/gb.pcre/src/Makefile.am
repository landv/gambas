COMPONENT = gb.pcre
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.pcre.la

gb_pcre_la_LIBADD = @PCRE_LIB@
gb_pcre_la_LDFLAGS = -module @LD_FLAGS@ @PCRE_LDFLAGS@
gb_pcre_la_CPPFLAGS = @PCRE_INC@

gb_pcre_la_SOURCES = \
 main.h main.c \
 regexp.h regexp.c \
 gb.pcre.h


