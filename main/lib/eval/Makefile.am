COMPONENT = gb.eval
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.eval.la

gb_eval_la_LIBADD = 
gb_eval_la_LDFLAGS = -module @LD_FLAGS@
gb_eval_la_CFLAGS = -I$(top_srcdir)/share @INCLTDL@ $(AM_CFLAGS)

gb_eval_la_SOURCES = \
 gb_alloc_override.h \
 gb_error.h gb_error.c \
 gb_array.c \
 gb_table.c \
 eval_code.h eval_code.c \
 eval_read.h eval_read.c \
 eval_reserved.c \
 eval_trans.h eval_trans.c \
 eval_trans_expr.c \
 eval_trans_tree.c \
 eval.h eval.c \
 eval_analyze.h eval_analyze.c \
 CExpression.h CExpression.c \
 CSystem.h CSystem.c \
 CHighlight.h CHighlight.c \
 gb.eval.h \
 main.h main.c


