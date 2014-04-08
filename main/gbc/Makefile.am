AM_CFLAGS += -I$(top_srcdir)/share
AM_CFLAGS_OPT += -I$(top_srcdir)/share

bin_PROGRAMS = gbc3 gba3 gbi3
noinst_PROGRAMS = gbcm3
##noinst_LTLIBRARIES = libgbcopt.la

##libgbcopt_la_CFLAGS= -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS_OPT)

gbc3_LDADD = @C_LIB@ @MATH_LIB@
gbc3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gbi3_LDADD = @C_LIB@ @DL_LIB@
gbi3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gba3_LDADD = @C_LIB@
gba3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gbcm3_LDADD = @C_LIB@

gbc3_SOURCES = \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gbc_class.h gbc_class.c \
 gbc_read.h gbc_read.c \
 gbc_preprocess.h gbc_preprocess.c \
 gbc_type.h gbc_type.c \
 gbc_compile.h gbc_compile.c \
 gbc_header.h gbc_header.c \
 gbc_help.h gbc_help.c \
 gbc_output.h gbc_output.c \
 gbc_trans.h gbc_trans_expr.c gbc_trans_tree.c gbc_trans_ctrl.c gbc_trans_subr.c \
 gbc_reserved.c \
 gb_buffer.c \
 gbc_dump.c gbc_code.c gbc_trans.c gbc_trans_code.c \
 gbc_pcode.c \
 gb_file.h gb_file.c \
 gbc_form.h gbc_form.c gbc_form_webpage.c \
 gb_str.h gb_str.c \
 gbc_chown.h gbc_chown.c \
 gb_common.c \
 gbc_arch.c \
 gb_table.c \
 gbc.c 

gba3_SOURCES = \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gb_table.c \
 gb_str.h gb_str.c \
 gb_file.h gb_file.c \
 gbc_archive.h gbc_archive.c \
 gbc_chown.h gbc_chown.c \
 gb_common.c \
 gbc_arch.c \
 gba.c

gbi3_SOURCES= \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gb_str.h gb_str.c \
 gb_file.h gb_file.c \
 gb_table.c \
 gb_common.c \
 gbc_arch.c \
 gbi.c

##libgbcopt_la_SOURCES = \
## gb_table.c

gbcm3_SOURCES = \
 gbc_reserved_make.c
