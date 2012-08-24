COMPONENT = gb.task
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.task.la

gb_task_la_LDFLAGS = -module @LD_FLAGS@
gb_task_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_task_la_SOURCES = \
 c_task.h c_task.c \
 main.h main.c


