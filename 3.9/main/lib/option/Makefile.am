COMPONENT = gb.option
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.option.la

gb_option_la_LIBADD = 
gb_option_la_LDFLAGS = -module @LD_FLAGS@
gb_option_la_CFLAGS = -I$(top_srcdir)/share @INCLTDL@ $(AM_CFLAGS)

gb_option_la_SOURCES = \
 main.h main.c \
 getoptions.h getoptions.c


