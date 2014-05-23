AM_CFLAGS += -I$(top_srcdir)/share @INTL_INC@ @CONV_INC@ @GBX_THREAD_INC@ @FFI_INC@
AM_CFLAGS_OPT += -I$(top_srcdir)/share

bin_PROGRAMS = gbx3
noinst_LIBRARIES = libgbx.a
gblib_LTLIBRARIES = gb.la

libgbx_a_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS_OPT)

gbx3_LDADD = @C_LIB@ libgbx.a @MATH_LIB@ @INTL_LIB@ @CONV_LIB@ @GETTEXT_LIB@ @DL_LIB@ @GBX_THREAD_LIB@ @FFI_LIB@
gbx3_LDFLAGS = @LD_FLAGS@ @INTL_LDFLAGS@ @CONV_LDFLAGS@ @GETTEXT_LDFLAGS@ @FFI_LDFLAGS@
gbx3_CFLAGS = -DGAMBAS_PATH="\"$(bindir)\"" $(AM_CFLAGS)

gb_la_LIBADD = @C_LIB@ @MATH_LIB@ @INTL_LIB@ @CONV_LIB@ @GETTEXT_LIB@ @DL_LIB@ @GBX_THREAD_LIB@ @FFI_LIB@
gb_la_LDFLAGS = -module @LD_FLAGS@ @INTL_LDFLAGS@ @CONV_LDFLAGS@ @GETTEXT_LDFLAGS@ @FFI_LDFLAGS@
gb_la_CFLAGS = -DGBX_INFO $(AM_CFLAGS) -O0

libgbx_a_SOURCES = \
 gb_error.h gb_error.c \
 gbx_split.h gbx_split.c \
 gbx_exec_loop.c

gbx3_SOURCES = \
 gb_common_check.h gb_common.c \
 gbx_debug.h gbx_debug.c \
 gbx_jit.h gbx_jit.c \
 gb_alloc.c gb_array.c \
 gbx_stack.h gbx_stack.c \
 gb_buffer.c gbx_replace.c \
 gbx_value.h gbx_value.c \
 gb_hash.c \
 gb_table.c \
 gb_list.c \
 gbx_type.h gbx_type.c \
 gbx_exec.h gbx_exec.c gbx_exec_push.c gbx_exec_enum.c gbx_exec_pop.c gbx_exec_operator.c \
 gbx_class_desc.h gbx_class.h gbx_class_init.c gbx_class.c gbx_class_native.c \
 gbx_class_load.c gbx_class_load.h \
 gbx_event.h gbx_event.c \
 gb_file.h gb_file.c \
 gbx_stream.h gbx_stream.c gbx_stream_direct.c gbx_stream_buffer.c gbx_stream_memory.c \
 gbx_stream_arch.c gbx_stream_process.c gbx_stream_pipe.c gbx_stream_string.c \
 gbx_project.h gbx_project.c \
 gbx_library.h gbx_library.c \
 gbx_subr.h gbx_subr.c \
 gbx_subr_file.c gbx_subr_string.c gbx_subr_conv.c gbx_subr_time.c gbx_subr_extern.c gbx_subr_misc.c \
 gbx_math.h gbx_math.c \
 gbx_subr_math_temp.h gbx_subr_math.c \
 gbx_subr_test_temp.h gbx_subr_test.c \
 gbx_api.h gbx_api.c \
 gbx_local.h gbx_local.c \
 gbx_regexp.h gbx_regexp.c \
 gbx_archive.h gbx_archive.c \
 gbx_watch.h gbx_watch.c \
 gbx_expression.h gbx_eval.h gbx_eval.c \
 gbx_compare.h gbx_compare.c \
 gbx.c \
 gbx_number.h gbx_number.c \
 gbx_object.h gbx_object.c \
 gbx_string.h gbx_string.c \
 gbx_variant.h \
 gbx_date.h gbx_date.c \
 gbx_c_class.h gbx_c_class.c \
 gbx_c_collection.h gbx_c_collection.c \
 gbx_c_error.h gbx_c_error.c \
 gbx_c_gambas.h gbx_c_gambas.c \
 gbx_c_file.h gbx_c_file.c \
 gbx_c_application.h gbx_c_application.c \
 gbx_c_system.h gbx_c_system.c \
 gbx_c_array.h gbx_c_array.c \
 gbx_c_process.h gbx_c_process.c \
 gbx_c_string.h gbx_c_string.c \
 gbx_c_observer.h gbx_c_observer.c \
 gbx_c_task.h gbx_c_task.c \
 gbx_component.h gbx_component.c \
 gbx_extern.h gbx_extern.c \
 gbx_c_enum.h gbx_c_enum.c \
 gbx_c_timer.h gbx_c_timer.c \
 gbx_struct.h gbx_struct.c \
 gbx_signal.h gbx_signal.c

gb_la_SOURCES = \
 gbx_info.h \
 gbx_local.h gbx_compare.h gbx_date.h \
 gbx_c_class.h gbx_c_class.c \
 gbx_c_collection.h gbx_c_collection.c \
 gbx_c_error.h gbx_c_error.c \
 gbx_c_gambas.h gbx_c_gambas.c \
 gbx_c_file.h gbx_c_file.c \
 gbx_c_application.h gbx_c_application.c \
 gbx_c_system.h gbx_c_system.c \
 gbx_c_array.h gbx_c_array.c \
 gbx_c_process.h gbx_c_process.c \
 gbx_c_string.h gbx_c_string.c \
 gbx_c_observer.h gbx_c_observer.c \
 gbx_c_enum.h gbx_c_enum.c \
 gbx_c_timer.h gbx_c_timer.c \
 gbx_c_task.h gbx_c_task.c \
 gbx_class_info.c
