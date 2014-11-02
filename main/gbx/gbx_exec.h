/***************************************************************************

  gbx_exec.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __GBX_EXEC_H
#define __GBX_EXEC_H

#include "gb_alloc.h"
#include "gb_error.h"
#include "gbx_class.h"
#include "gbx_value.h"
#include "gb_pcode.h"
#include "gbx_stack.h"

#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_variant.h"

#include "gbx_c_enum.h"

enum { GB_LITTLE_ENDIAN, GB_BIG_ENDIAN };

typedef
	void (*EXEC_FUNC)();

typedef
	void (*EXEC_FUNC_CODE)(ushort);

typedef
	struct {
		CLASS *class;
		OBJECT *object;
		int index;
		CLASS_DESC_METHOD *desc;
		char nparam;
		bool native;
		bool use_stack;
		}
	EXEC_GLOBAL;

typedef
	struct {
		void (*main)();
		void (*loop)();
		void (*wait)();
		void (*timer)();
		void (*lang)();
		void (*watch)();
		void (*post)();
		void (*quit)();
		void (*error)();
		double (*timeout)();
		}
	EXEC_HOOK;

enum {
	OP_NOTHING = 0,
	OP_OBJECT_FLOAT,
	OP_FLOAT_OBJECT,
	OP_OBJECT_OTHER,
	OP_OTHER_OBJECT,
	OP_OBJECT_OBJECT
	};

	
#ifndef __GBX_EXEC_C

extern STACK_CONTEXT EXEC_current;
extern VALUE *SP;
extern VALUE TEMP;
extern VALUE RET;

extern VALUE *EXEC_super;

extern bool EXEC_debug;
extern bool EXEC_task;
extern bool EXEC_profile;
extern const char *EXEC_profile_path;
extern bool EXEC_profile_instr;
extern bool EXEC_arch;
extern bool EXEC_fifo;
extern const char *EXEC_fifo_name;
extern bool EXEC_keep_library;
extern bool EXEC_string_add;
extern bool EXEC_break_on_error;

extern EXEC_HOOK EXEC_Hook;

extern CENUM *EXEC_enum;

extern bool EXEC_big_endian;
extern bool EXEC_main_hook_done;
extern bool EXEC_got_error;

extern const char EXEC_should_borrow[];

extern const char *EXEC_unknown_name;
extern char EXEC_unknown_nparam;
extern uchar EXEC_quit_value;

extern EXEC_GLOBAL EXEC;

#endif

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

#define HOOK(func) (!EXEC_Hook.func) ? 0 : (*EXEC_Hook.func)
#define HOOK_DEFAULT(func, def) (*((!EXEC_Hook.func) ? def : EXEC_Hook.func))

#define GET_NPARAM(var)         short var = code & 0x3F
#define GET_PARAM(var, nparam)  VALUE *var = &SP[-nparam]

void EXEC_init(void);

void EXEC_enter_check(bool defined);
void EXEC_enter(void);
void EXEC_enter_quick(void);
void EXEC_leave_keep();
void EXEC_leave_drop();
void EXEC_loop(void);

#define EXEC_object(_val, _pclass, _pobject) \
	((LIKELY(TYPE_is_pure_object((_val)->type))) ? (*(_pclass) = EXEC_object_real(_val, _pobject)), TRUE : \
	TYPE_is_variant((_val)->type) ? (*(_pclass) = EXEC_object_variant(_val, _pobject)), FALSE : \
	EXEC_object_other(_val, _pclass, _pobject))
	
#define EXEC_object_fast(_val, _pclass, _pobject) \
({ \
	if ((_val)->type != T_CLASS) \
		(*(_pclass)) = EXEC_object_real(_val, _pobject); \
	else \
		EXEC_object_other(_val, _pclass, _pobject); \
})

#define EXEC_object_array(_val, _vclass, _vobject) \
	_vobject = (_val)->_object.object; \
	if (!(_vobject)) \
		THROW(E_NULL); \
	_vclass = (_vobject)->class; \
	if ((_val) == EXEC_super) \
		EXEC_super = (_val)->_object.super;

CLASS *EXEC_object_real(VALUE *val, OBJECT **pobject);
CLASS *EXEC_object_variant(VALUE *val, OBJECT **pobject);
bool EXEC_object_other(VALUE *val, CLASS **pclass, OBJECT **pobject);

void *EXEC_auto_create(CLASS *class, bool ref);

bool EXEC_call_native(void (*exec)(), void *object, TYPE type, VALUE *param);
void EXEC_native_check(bool defined);
void EXEC_native_quick(void);
void EXEC_native(void);
void EXEC_function_real(void);
void EXEC_jit_function_loop(void);
void EXEC_function_loop(void);

#define EXEC_function() EXEC_function_real(), EXEC_release_return_value()
#define EXEC_function_keep() EXEC_function_real()

void EXEC_public(CLASS *class, void *object, const char *name, int nparam);
void EXEC_public_desc(CLASS *class, void *object, CLASS_DESC_METHOD *desc, int nparam);

bool EXEC_special(int special, CLASS *class, void *object, int nparam, bool drop);

void EXEC_special_inheritance(int special, CLASS *class, OBJECT *object, int nparam, bool drop);

void EXEC_nop(void);

void EXEC_push_unknown(void);
void EXEC_push_array(ushort code);
int EXEC_push_unknown_event(bool unknown);
//void EXEC_push_special(void);

void EXEC_pop_unknown(void);
void EXEC_pop_array(ushort code);

void EXEC_enum_first(PCODE code);
bool EXEC_enum_next(PCODE code);

void *EXEC_create_object(CLASS *class, int np, char *event);
void EXEC_new(void);

void EXEC_release_return_value(void);
void EXEC_quit(void);

void EXEC_dup(int n);

void EXEC_borrow(TYPE type, VALUE *val);
void UNBORROW(VALUE *val);
void EXEC_release(TYPE type, VALUE *val);
void RELEASE_many(VALUE *val, int n);

void EXEC_push_complex(void);

void EXEC_push_vargs(void);
void EXEC_drop_vargs(void);

#define BORROW(_value) \
do { \
	VALUE *_v = (_value); \
	TYPE type = _v->type; \
	if (TYPE_is_object(type)) \
	{ \
		OBJECT_REF(_v->_object.object); \
	} \
	else if (EXEC_should_borrow[type]) \
	{ \
		if (type == T_STRING) \
			STRING_ref(_v->_string.addr); \
		else \
			EXEC_borrow(type, _v); \
	} \
} while (0)

#define RELEASE(_value) \
do { \
	VALUE *_v = (_value); \
	TYPE type = _v->type; \
	if (TYPE_is_object(type)) \
	{ \
		OBJECT_UNREF(_v->_object.object); \
	} \
	else if (EXEC_should_borrow[type]) \
	{ \
		if (type == T_STRING) \
			STRING_unref(&_v->_string.addr); \
		else \
			EXEC_release(type, _v); \
	} \
} while (0)

#define RELEASE_STRING(_value) \
do { \
	if ((_value)->type == T_STRING) STRING_unref(&(_value)->_string.addr); \
} while(0)

#define RELEASE_OBJECT(_value) \
do { \
	VALUE *_v = (_value); \
	OBJECT_UNREF(_v->_object.object); \
} while (0)

#define RELEASE_MANY(_val, _n) \
do { \
if (_n) \
{ \
	if ((_n) == 1) \
	{ \
		_val--; \
		RELEASE((_val)); \
	} \
	else \
	{ \
		RELEASE_many((_val), (_n)); \
		_val -= (_n); \
	} \
} \
} while (0)

#define PUSH() \
do { \
	BORROW(SP); \
	SP++; \
} while (0)

#define POP() \
do { \
	SP--; \
	RELEASE(SP); \
} while (0)

#define COPY_VALUE(_dst, _src) VALUE_copy(_dst, _src)

#define EXEC_set_native_error(_err) (ERROR_current->info.native = (_err))
#define EXEC_has_native_error() (ERROR_current->info.native)

bool EXEC_check_operator_single(VALUE *P1, uchar op);
int EXEC_check_operator(VALUE *P1, VALUE *P2, uchar op);
void EXEC_operator(uchar what, uchar op, VALUE *P1, VALUE *P2);
void EXEC_operator_object_add_quick(VALUE *P1, double val);
int EXEC_comparator(uchar what, uchar op, VALUE *P1, VALUE *P2);
void EXEC_operator_object_sgn(VALUE *P1);
void EXEC_operator_object_fabs(VALUE *P1);
void EXEC_operator_object_single(uchar op, VALUE *P1);

#endif /* */
