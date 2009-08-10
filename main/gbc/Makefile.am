AM_CFLAGS += -I$(top_srcdir)/share @INCLTDL@

bin_PROGRAMS = gbc3 gba3 gbi3

gbc3_LDADD = @MATH_LIB@
gbc3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gbi3_LDADD = @LIBLTDL@
gbi3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gba3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gbc3_SOURCES = \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gbc_class.h \
 gbc_read.h gbc_read.c \
 gbc_type.h gbc_compile.h gbc_header.h \
 gbc_output.h gbc_trans.h \
 gbc_reserved.c \
 gbc_compile.c \
 gb_table.c gbc_type.c gb_buffer.c \
 gbc_dump.c gbc_class.c gbc_code.c gbc_trans.c gbc_header.c gbc_trans_code.c \
 gbc_trans_expr.c gbc_trans_tree.c gbc_trans_ctrl.c gbc_trans_subr.c \
 gbc_output.c gbc_pcode.c \
 gb_file.h gb_file.c \
 gbc_form.h gbc_form.c \
 gb_str.h gb_str.c \
 gbc_chown.h gbc_chown.c \
 gb_common.c \
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
 gba.c

gbi3_SOURCES= \
 gb_error.h gb_error.c \
 gb_alloc.c gb_array.c \
 gb_str.h gb_str.c \
 gb_file.h gb_file.c \
 gb_table.c \
 gb_common.c \
 gbi.c

gbc3-gb_table.o: gb_table.c
	if $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(gbc3_CFLAGS) -O3 $(CFLAGS) -MT gbc3-gb_table.o -MD -MP -MF "$(DEPDIR)/gbc3-gb_table.Tpo" -c -o gbc3-gb_table.o `test -f 'gb_table.c' || echo '$(srcdir)/'`gb_table.c; then mv -f "$(DEPDIR)/gbc3-gb_table.Tpo" "$(DEPDIR)/gbc3-gb_table.Po"; else rm -f "$(DEPDIR)/gbc3-gb_table.Tpo"; exit 1; fi

