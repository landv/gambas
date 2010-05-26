/***************************************************************************

  gbx_exec.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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


typedef
  void (*EXEC_FUNC)();

typedef
  struct {
    CLASS *class;
    OBJECT *object;
    int index;
    //FUNCTION *func;
    CLASS_DESC_METHOD *desc;
    int nparam;
    int nparvar;
    bool drop;
    bool native;
    bool use_stack;
    bool property;
    const char *unknown;
    }
  EXEC_FUNCTION;

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
    int (*image)();
    int (*picture)();
    }
  EXEC_HOOK;

#ifndef __GBX_EXEC_C

EXTERN STACK_CONTEXT EXEC_current;
EXTERN PCODE EXEC_code;
EXTERN VALUE *SP;
EXTERN VALUE TEMP;
EXTERN VALUE RET;
EXTERN EXEC_FUNCTION EXEC;

EXTERN VALUE *EXEC_super;

EXTERN bool EXEC_debug;
EXTERN bool EXEC_arch;
EXTERN bool EXEC_fifo;
EXTERN const char *EXEC_fifo_name;
EXTERN bool EXEC_keep_library;

EXTERN EXEC_HOOK EXEC_Hook;

EXTERN CENUM *EXEC_enum;

EXTERN bool EXEC_big_endian;
EXTERN bool EXEC_main_hook_done;
EXTERN int EXEC_return_value;
EXTERN bool EXEC_got_error;
/*EXTERN long EXEC_const[];*/

#endif

/* Local variables base pointer */
#define BP EXEC_current.bp
/* Current class */
#define CP EXEC_current.cp
/* Current object */
#define OP EXEC_current.op
/* Paramters base pointer */
#define PP EXEC_current.pp
/* Save stack pointer for a TRY */
#define EP EXEC_current.ep
/* Current function */
#define FP EXEC_current.fp
/* Program counter */
#define PC EXEC_current.pc
/* Where to go if there is an error */
#define EC EXEC_current.ec
/* Save register for TRY */
#define ET EXEC_current.et
/* Last break in the function */
#define TC EXEC_current.tc
/* Stack at the last break in the function */
#define TP EXEC_current.tp

/* Function return value pointer */
#define RP (&RET)

#define HOOK(func) (!EXEC_Hook.func) ? 0 : (*EXEC_Hook.func)
#define HOOK_DEFAULT(func, def) (*((!EXEC_Hook.func) ? def : EXEC_Hook.func))

#define GET_NPARAM(var)         short var = *PC & 0x3F
#define GET_PARAM(var, nparam)  VALUE *var = &SP[-nparam]

void EXEC_init(void);

void EXEC_enter_check(bool defined);
void EXEC_enter(void);
void EXEC_enter_quick(void);
void EXEC_leave(bool drop);
void EXEC_loop(void);

void EXEC_object(VALUE *SP, CLASS **pclass, OBJECT **pobject, bool *pdefined);
void *EXEC_auto_create(CLASS *class, bool ref);

bool EXEC_call_native(void (*exec)(), void *object, TYPE type, VALUE *param);
void EXEC_native_check(bool defined);
void EXEC_native_quick(void);
void EXEC_native();
void EXEC_function_real();
void EXEC_function_loop();

#define EXEC_function() EXEC_function_real(), EXEC_release_return_value()
#define EXEC_function_keep() EXEC_function_real()

void EXEC_public(CLASS *class, void *object, const char *name, int nparam);
void EXEC_public_desc(CLASS *class, void *object, CLASS_DESC_METHOD *desc, int nparam);

bool EXEC_special(int spec, CLASS *class, void *object, int nparam, bool drop);

void EXEC_special_inheritance(int special, CLASS *class, OBJECT *object, int nparam, bool drop);

void EXEC_nop(void);

void EXEC_push_unknown(ushort code);
void EXEC_push_array(ushort code);
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

#define BORROW(_value) \
do { \
	VALUE *_v = (_value); \
	TYPE type = _v->type; \
	if (TYPE_is_object(type)) \
	{ \
		OBJECT_REF(_v->_object.object, "BORROW"); \
	} \
	else if (type == T_STRING) \
		STRING_ref(_v->_string.addr); \
	else \
		EXEC_borrow(type, _v); \
} while (0)

#define RELEASE(_value) \
do { \
	VALUE *_v = (_value); \
	TYPE type = _v->type; \
	if (TYPE_is_object(type)) \
	{ \
		OBJECT_UNREF(_v->_object.object, "RELEASE"); \
	} \
	else if (type == T_STRING) \
		STRING_unref(&_v->_string.addr); \
	else \
		EXEC_release(type, _v); \
} while (0)

#define RELEASE_STRING(_value) \
do { \
	VALUE *_v = (_value); \
	if (_v->type == T_STRING) STRING_unref(&_v->_string.addr); \
} while(0)

#define RELEASE_OBJECT(_value) \
do { \
	VALUE *_v = (_value); \
	OBJECT_UNREF(_v->_object.object, "RELEASE_OBJECT"); \
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

#endif /* */
