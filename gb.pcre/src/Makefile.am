COMPONENT = gb.pcre
include $(top_srcdir)/component.am

INCLUDES = @PCRE_INC@

gblib_LTLIBRARIES = gb.pcre.la

gb_pcre_la_LIBADD = @PCRE_LIB@
gb_pcre_la_LDFLAGS = -module @LD_FLAGS@

gb_pcre_la_SOURCES = \
 main.h main.c \
 regexp.h regexp.c


