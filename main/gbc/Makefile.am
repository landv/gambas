INCLUDES = -I$(top_srcdir)/share @INCLTDL@

bin_PROGRAMS = gbc2 gba2 gbi2
noinst_LTLIBRARIES = libgbcopt.la

gbc2_LDADD = libgbcopt.la @MATH_LIB@
gbc2_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

libgbcopt_la_CFLAGS= -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS_OPT)

gbi2_LDADD = @LIBLTDL@
gbi2_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gba2_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gbc2_SOURCES = \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gbc_class.h \
 gbc_read.h gbc_read.c \
 gbc_type.h gbc_compile.h gbc_header.h \
 gbc_output.h gbc_trans.h \
 gbc_reserved.c \
 gbc_compile.c \
 gbc_type.c gb_buffer.c \
 gbc_dump.c gbc_class.c gbc_code.c gbc_trans.c gbc_header.c gbc_trans_code.c \
 gbc_trans_expr.c gbc_trans_tree.c gbc_trans_ctrl.c gbc_trans_subr.c \
 gbc_output.c gbc_pcode.c \
 gb_file.h gb_file.c \
 gbc_form.h gbc_form.c \
 gb_str.h gb_str.c \
 gbc_chown.h gbc_chown.c \
 gb_common.c \
 gbc.c 

gba2_SOURCES = \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gb_table.c \
 gb_str.h gb_str.c \
 gb_file.h gb_file.c \
 gbc_archive.h gbc_archive.c \
 gbc_chown.h gbc_chown.c \
 gb_common.c \
 gba.c

gbi2_SOURCES= \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gb_str.h gb_str.c \
 gb_file.h gb_file.c \
 gb_table.c \
 gb_common.c \
 gbi.c

libgbcopt_la_SOURCES = gb_table.c
