/***************************************************************************

  gb.jit.h

  gb.jit component

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __GB_JIT_H
#define __GB_JIT_H

#include "gbx_extern.h"

typedef
	struct {
		//void (*GetExec)(void);
		
		void (*F_EXEC_release)(TYPE type, VALUE *value);
		void (*F_RELEASE_many)(VALUE* val, int n);
		
		void (*F_EXEC_push_unknown)(ushort code);
		void (*F_EXEC_push_array)(ushort code);
		
		void (*F_EXEC_pop_unknown)(void);
		void (*F_EXEC_pop_array)(ushort code);
		
		void *(*F_CENUM_create)(void *object);
		void (*F_EXEC_enum_first)(PCODE code);
		char (*F_EXEC_enum_next)(PCODE code);
		
		void (*F_EXEC_new)(void);
		
		void (*F_EXEC_enter_quick)(void);
		void (*F_EXEC_enter)(void);
		//void (*F_EXEC_jit_execute_function)(void);
		void (*F_EXEC_native_quick)(void);
		void (*F_EXEC_native)(void);
		char (*F_EXEC_call_native)(void (*exec)(), void *object, TYPE type, VALUE *param);
		void (*F_EXEC_function_real)(void);
		void (*F_EXEC_leave_keep)(void);
		void (*F_EXEC_leave_drop)(void);
		void (*F_EXEC_function_loop)(void);
		
		char (*F_EXEC_special)(int special, CLASS *klass, void *object, int nparam, bool drop);
		CLASS *(*F_EXEC_object_variant)(VALUE *val, OBJECT **pobject);
		char (*F_EXEC_object_other)(VALUE *val, CLASS **pclass, OBJECT **pobject);
		
		EXTERN_FUNC_INFO (*F_EXTERN_get_function_info)(CLASS_EXTERN *ext);
		void (*F_EXTERN_call)(void);
		void *(*F_EXTERN_make_callback)(VALUE_FUNCTION* value);
		
		char *(*F_STRING_new)(const char *src, int len);
		void (*F_STRING_new_temp_value)(VALUE *value, const char* src, int len);
		void (*F_STRING_free_real)(char *ptr);
		char *(*F_STRING_free_later)(char *ptr);
		int (*F_STRING_compare)(const char *ptr1, int len1, const char *ptr2, int len2);
		char (*F_STRING_equal_ignore_case_same)(const char *str1, const char *str2, int len);
		int (*F_STRING_conv)(char **result, const char *str, int len, const char *src, const char *dst, char _throw);
		
		int (*F_OBJECT_comp_value)(VALUE *object_1, VALUE *object_2);
		int (*F_COMPARE_object)(void *object_1, void *object_2);
		char (*F_CLASS_inherits)(CLASS *klass, CLASS *parent);
		void *(*F_OBJECT_create)(CLASS *klass, const char *name, void *parent, int nparam);
		void (*F_CLASS_free)(void *object);
		int (*F_SYMBOL_find)(void *symbol, ushort *sort, int n_symbol, size_t s_symbol, int flag, const char *name, int len, const char *prefix);
		
		CLASS *(*F_CARRAY_get_array_class)(CLASS *klass, CTYPE ctype);
		void *(*F_CARRAY_get_data_multi)(void *_object, GB_INTEGER *arg, int nparam);
		void *(*F_CARRAY_create_static)(CLASS *klass, void *ref, CLASS_ARRAY *desc, void *data);
		void *(*F_CSTRUCT_create_static)(void *ref, CLASS *klass, char *addr);
		
		void (*F_REGEXP_scan)(void *array, const char *pattern, int len_pattern, const char *string, int len_string);
		
		void (*F_VALUE_convert)(VALUE *value, TYPE type);
		void (*F_VALUE_to_string)(VALUE *value, char **addr, int *len);
		void (*F_VALUE_convert_float)(VALUE *value);
		void (*F_VALUE_convert_variant)(VALUE *value);
		void (*F_VALUE_convert_string)(VALUE *value);
		char (*F_VALUE_is_null)(VALUE *val);
		void (*F_VALUE_undo_variant)(VALUE *value);
		
		char (*F_NUMBER_from_string)(int option, const char *str, int len, VALUE *value);
		void (*F_NUMBER_int_to_string)(uint64_t nbr, int prec, int base, VALUE *value);
		
		char (*F_LOCAL_format_number)(double number, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str, char local);
		
		int (*F_DATE_to_string)(char *buffer, VALUE *value);
		char (*F_DATE_from_string)(const char *str, int len, VALUE *val, char local);
		int (*F_DATE_comp)(DATE *date1, DATE *date2);
		void (*F_DATE_timer)(double *result, int from_start);
		void (*F_DATE_now)(VALUE *val);
		void (*F_DATE_add)(VALUE *date, int period, int val);
		void (*F_DATE_diff)(VALUE *date1, VALUE *date2, int period);
		
		void (*F_randomize)(char set, uint seed);
		double (*F_rnd)(void);
		
		void (*F_THROW)(int code, ...) NORETURN;
		void (*F_ERROR_panic)(const char *error, ...) NORETURN;
		void (*F_ERROR_propagate)(void) NORETURN;
		void (*F_ERROR_reset)(ERROR_INFO *info);
		void (*F_ERROR_set_last)(char bt);
		void (*F_ERROR_lock)(void);
		void (*F_ERROR_unlock)(void);
		
		const char *(*F_TYPE_get_name)(TYPE type);
		
		void (*F_SUBR_not)(ushort code);
		void (*F_SUBR_and_)(ushort code);
		void (*F_SUBR_cat)(ushort code);
		void (*F_SUBR_file)(ushort code);
		void (*F_SUBR_like)(ushort code);
		void (*F_SUBR_string)(void);
		void (*F_SUBR_upper)(ushort code);
		void (*F_SUBR_instr)(ushort code);
		void (*F_SUBR_subst)(ushort code);
		void (*F_SUBR_replace)(ushort code);
		void (*F_SUBR_split)(ushort code);
		void (*F_SUBR_strcomp)(ushort code);
		void (*F_SUBR_sconv)(ushort code);
		void (*F_SUBR_abs)(ushort code);
		void (*F_SUBR_int)(ushort code);
		void (*F_SUBR_fix)(ushort code);
		void (*F_SUBR_sgn)(ushort code);
		void (*F_SUBR_min_max)(ushort code);
		void (*F_SUBR_choose)(ushort code);
		void (*F_SUBR_bit)(ushort code);
		void (*F_SUBR_is_type)(ushort code);
		void (*F_SUBR_hex_bin)(ushort code);
		void (*F_SUBR_val)(ushort code);
		void (*F_SUBR_format)(ushort code);
		void (*F_SUBR_year)(ushort code);
		void (*F_SUBR_week)(ushort code);
		void (*F_SUBR_date)(ushort code);
		void (*F_SUBR_time)(ushort code);
		void (*F_SUBR_eval)(ushort code);
		void (*F_SUBR_debug)(void);
		void (*F_SUBR_wait)(ushort code);
		void (*F_SUBR_open)(ushort code);
		void (*F_SUBR_close)(ushort code);
		void (*F_SUBR_input)(ushort code);
		void (*F_SUBR_linput)(void);
		void (*F_SUBR_print)(ushort code);
		void (*F_SUBR_read)(ushort code);
		void (*F_SUBR_write)(ushort code);
		void (*F_SUBR_flush)(void);
		void (*F_SUBR_lock)(void);
		void (*F_SUBR_inp_out)(ushort code);
		void (*F_SUBR_eof)(ushort code);
		void (*F_SUBR_lof)(ushort code);
		void (*F_SUBR_seek)(ushort code);
		void (*F_SUBR_kill)(ushort code);
		void (*F_SUBR_move)(ushort code);
		void (*F_SUBR_exist)(ushort code);
		void (*F_SUBR_access)(ushort code);
		void (*F_SUBR_stat)(ushort code);
		void (*F_SUBR_dfree)(void);
		void (*F_SUBR_temp)(ushort code);
		void (*F_SUBR_isdir)(ushort code);
		void (*F_SUBR_dir)(ushort code);
		void (*F_SUBR_rdir)(ushort code);
		void (*F_SUBR_exec)(ushort code);
		void (*F_SUBR_alloc)(ushort code);
		void (*F_SUBR_free)(void);
		void (*F_SUBR_realloc)(ushort code);
		void (*F_SUBR_strptr)(ushort code);
		void (*F_SUBR_collection)(ushort code);
		void (*F_SUBR_tr)(ushort code);
		void (*F_SUBR_quote)(ushort code);
		void (*F_SUBR_unquote)(ushort code);
		void (*F_SUBR_ptr)(ushort code);
		
		void (*F_CLASS_load_from_jit)(CLASS *klass);
		void (*F_CLASS_run_inits)(CLASS *klass);
		
		const char *(*F_DEBUG_get_current_position)(void);
		void (*F_DEBUG_Profile_Add)(void *cp, void *fp, void *pc);
		
		void (*F_EXEC_quit)(void);
		
		
		//In GB: LOCAL_gettext, CLASS_auto_create, GB_Raise, GB_CollectionGet, GB_CollectionSet
		
		}
	GB_JIT_INTERFACE;

typedef
	struct {
		intptr_t version;
		void (*Init)(GB_JIT_INTERFACE *jif, char **STACK_limit, STACK_CONTEXT *EXEC_current, VALUE **SP, VALUE *TEMP,
			VALUE *RET, char *GAMBAS_StopEvent, char **EXEC_enum, EXEC_GLOBAL *EXEC,
			const char **EXEC_unknown_name, char *__EXEC_profile, char *__EXEC_profile_instr, unsigned char *__EXEC_quit_value,
			void **EVENT_Last, ERROR_CONTEXT **ERROR_current, ERROR_HANDLER **ERROR_handler, const char *STRING_char_string);
		void (*CompileAndExecute)(void);
		void (*LoadClass)(CLASS *klass);
		}
	JIT_INTERFACE;

#define JIT_INTERFACE_VERSION 1

#endif /* __GB_JIT_H */
