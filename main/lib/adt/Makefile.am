COMPONENT = gb.adt
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.adt.la

gb_adt_la_LDFLAGS = -module @LD_FLAGS@
gb_adt_la_CFLAGS  = -I$(top_srcdir)/share

gb_adt_la_SOURCES = \
 main.h main.c \
 c_list.h c_list.c \
 c_deque.h c_deque.c \
 c_circular.h c_circular.c

