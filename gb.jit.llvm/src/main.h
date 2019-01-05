/***************************************************************************

  main.h

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gb_common.h"
#include "gambas.h"

#ifdef __cplusplus
#define class klass
#endif

#include "gbx_value.h"
#include "gbx_stack.h"
#include "gbx_class.h"
#include "gbx_object.h"

#ifdef __cplusplus
#undef class
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MAIN_CPP
extern GB_INTERFACE GB;
extern bool MAIN_debug;
#endif

typedef
	struct {
#ifdef __cplusplus
		CLASS *klass;
#else
		CLASS *class;
#endif
		OBJECT *object;
		int index;
		CLASS_DESC_METHOD *desc;
		char nparam;
		bool native;
		bool use_stack;
		}
	EXEC_GLOBAL;
	
/*typedef
	struct {
		char code;          // Error code
		char native;        // A native method has raised an error
		char free;          // If 'msg' sould be freed
		char _reserved;
		void *cp;
		void *fp;
		void *pc;
		char *msg;
		}
	ERROR_INFO;

typedef
	struct _ERROR_CONTEXT {
		struct _ERROR_CONTEXT *prev;
		struct _ERROR_HANDLER *handler;
		ERROR_INFO info;
		jmp_buf env;
		char ret;
		}
	ERROR_CONTEXT;

typedef
	struct _ERROR_HANDLER {
		struct _ERROR_HANDLER *prev;
		ERROR_CONTEXT *context;
		void (*handler)();
		}
	ERROR_HANDLER;
	
#define ERROR_LEAVE_DONE ((ERROR_CONTEXT *)-1)*/

#include "gb.jit.h"

#ifndef __JIT_API_CPP
extern GB_JIT_INTERFACE JIF;
extern char **_STACK_limit;
extern STACK_CONTEXT *_EXEC_current;
extern VALUE **_SP;
extern VALUE *_TEMP;
extern VALUE *_RET;

extern char *_GAMBAS_StopEvent;
extern char **_EXEC_enum;
extern EXEC_GLOBAL *_EXEC;
extern const char **_EXEC_unknown_name;
extern char *_EXEC_profile;
extern char *_EXEC_profile_instr;
extern unsigned char *_EXEC_quit_value;

extern void **_EVENT_Last;

extern ERROR_CONTEXT **_ERROR_current;
extern ERROR_HANDLER **_ERROR_handler;

extern const char *_STRING_char_string;
#endif

#ifdef __cplusplus
}
#endif

#define STACK_limit (*_STACK_limit)
#define EXEC_current (*_EXEC_current)
#define SP (*_SP)
#define TEMP (*_TEMP)
#define RET (*_RET)

#define GAMBAS_StopEvent (*_GAMBAS_StopEvent)
#define EXEC_enum (*_EXEC_enum)
#define EXEC (*_EXEC)
#define EXEC_unknown_name (*_EXEC_unknown_name)
#define EXEC_profile (*_EXEC_profile)
#define EXEC_profile_instr (*_EXEC_profile_instr)
#define EXEC_quit_value (*_EXEC_quit_value)

#define EVENT_Last (*_EVENT_Last)

#define ERROR_current (*_ERROR_current)
#define ERROR_handler (*_ERROR_handler)

#define STRING_char_string (_STRING_char_string)

// Local variables base pointer
#define BP EXEC_current.bp
// Arguments base pointer
#define PP EXEC_current.pp
// Current class
#define CP EXEC_current.cp
// Current object
#define OP EXEC_current.op
// Save stack pointer for a TRY
#define EP EXEC_current.ep
// Current function
#define FP EXEC_current.fp
// Program counter
#define PC EXEC_current.pc
// Where to go if there is an error
#define EC EXEC_current.ec
// Save register for TRY
#define ET EXEC_current.et
// Last break in the function
#define TC EXEC_current.tc
// Stack at the last break in the function
#define TP EXEC_current.tp
// GoSub stack
#define GP EXEC_current.gp

// Function return value pointer
#define RP (&RET)

#define THROW JIF.F_THROW
#define THROW_ILLEGAL() THROW(E_ILLEGAL)
#define SYMBOL_find JIF.F_SYMBOL_find

#undef TRY
#undef CATCH
#undef END_TRY
#undef ERROR_enter
#undef ERROR_leave

#define TRY \
	{ \
		ERROR_CONTEXT __err_context; \
		{ \
			ERROR_CONTEXT *__err = &__err_context; \
			ERROR_enter(__err); \
			__err->ret = setjmp(__err->env); \
			if (__err->ret == 0)

#define CATCH \
			else

#define END_TRY \
			ERROR_leave(__err); \
		} \
	}


#define ERROR_enter(_err) \
do { \
	_err->prev = ERROR_current; \
	_err->info.code = 0; \
	_err->info.native = 0; \
	_err->handler = ERROR_handler; \
	ERROR_current = _err; \
} while(0)

#define ERROR_leave(_err) \
do { \
	ERROR_CONTEXT *_prev = (_err); \
	if (_prev->prev != ERROR_LEAVE_DONE) \
	{ \
		ERROR_current = _prev->prev; \
		if (ERROR_current) \
		{ \
			if (_prev->info.code) \
			{ \
				JIF.F_ERROR_reset(&ERROR_current->info); \
				ERROR_current->info = _prev->info; \
				ERROR_current->info.native = FALSE; \
			} \
		} \
		else \
			JIF.F_ERROR_reset(&_prev->info); \
		_prev->prev = ERROR_LEAVE_DONE; \
	} \
} while(0)


#ifdef __cplusplus
extern "C" {
#endif

void JIT_init(GB_JIT_INTERFACE *jif, char **__STACK_limit, STACK_CONTEXT *__EXEC_current, VALUE **__SP, VALUE *__TEMP,
	VALUE *__RET, char *__GAMBAS_StopEvent, char ** __EXEC_enum, EXEC_GLOBAL *__EXEC,
	const char **__EXEC_unknown_name, char *__EXEC_profile, char *__EXEC_profile_instr, unsigned char *__EXEC_quit_value,
	void **__EVENT_Last, ERROR_CONTEXT **__ERROR_current, ERROR_HANDLER **__ERROR_handler, const char *__STRING_char_string);
void JIT_compile_and_execute(void);
void JIT_end(void);
void JIT_load_class(CLASS *klass);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
