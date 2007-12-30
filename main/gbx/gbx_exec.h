/***************************************************************************

  exec.h

  Subroutines for the interpreter : executing methods, native methods,
  the NEW operator, the casting operator, etc.

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
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
#include "gbx_array.h"

#include "gbx_c_enum.h"


typedef
  void (*EXEC_FUNC)();

typedef
  struct {
    CLASS *class;
    OBJECT *object;
    long index;
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

EXTERN EXEC_HOOK EXEC_Hook;

EXTERN CENUM *EXEC_enum;

EXTERN bool EXEC_big_endian;
EXTERN bool EXEC_main_hook_done;
EXTERN int EXEC_return_value;

/*EXTERN long EXEC_const[];*/

#endif

/* Pointeur de r��ence des variables locales */
#define BP EXEC_current.bp
/* Classe en cours */
#define CP EXEC_current.cp
/* Objet en cours */
#define OP EXEC_current.op
/* Pointeur de r��ence des param�res */
#define PP EXEC_current.pp
/* Sauvegarde du pointeur de pile pour un TRY */
#define EP EXEC_current.ep
/* fonction en cours */
#define FP EXEC_current.fp
/* Pointeur de programme */
#define PC EXEC_current.pc
/* Emplacement o aller en cas d'erreur */
#define EC EXEC_current.ec
/* Emplacement de sauvegarde pour TRY */
#define ET EXEC_current.et
/* Last break in the function */
#define TC EXEC_current.tc
/* Stack at the last break in the function */
#define TP EXEC_current.tp

/* Valeur de retour d'une fonction */
#define RP (&RET)

#define HOOK(func) (!EXEC_Hook.func) ? 0 : (*EXEC_Hook.func)
#define HOOK_DEFAULT(func, def) (*((!EXEC_Hook.func) ? def : EXEC_Hook.func))

#define GET_NPARAM(var)         short var = *PC & 0x3F
#define GET_PARAM(var, nparam)  VALUE *var = &SP[-nparam]

#define PUSH() (BORROW(SP), SP++)
#define POP()  (SP--, RELEASE(SP))

PUBLIC void EXEC_init(void);

PUBLIC void EXEC_enter_check(bool defined);
PUBLIC void EXEC_enter(void);
PUBLIC void EXEC_enter_quick(void);
PUBLIC void EXEC_leave(bool drop);
PUBLIC void EXEC_loop(void);

PUBLIC void EXEC_object(VALUE *SP, CLASS **pclass, OBJECT **pobject, bool *pdefined);
PUBLIC void *EXEC_auto_create(CLASS *class, bool ref);

PUBLIC bool EXEC_call_native(void (*exec)(), void *object, TYPE type, VALUE *param);
PUBLIC void EXEC_native();
PUBLIC void EXEC_function_real(bool keep_ret_value);

#define EXEC_function() EXEC_function_real(FALSE)
#define EXEC_function_keep() EXEC_function_real(TRUE)

PUBLIC void EXEC_public(CLASS *class, void *object, const char *name, int nparam);

PUBLIC boolean EXEC_spec(int spec, CLASS *class, void *object, int nparam, boolean drop);
#define EXEC_special EXEC_spec

PUBLIC void EXEC_special_inheritance(int special, CLASS *class, OBJECT *object, int nparam, boolean drop);

PUBLIC void EXEC_nop(void);
PUBLIC void EXEC_ILLEGAL(void);

PUBLIC void EXEC_push_unknown(ushort code);
PUBLIC void EXEC_push_array(void);
//PUBLIC void EXEC_push_special(void);

PUBLIC void EXEC_pop_unknown(void);
PUBLIC void EXEC_pop_array(void);

PUBLIC void EXEC_enum_first(PCODE code);
PUBLIC bool EXEC_enum_next(PCODE code);

PUBLIC void EXEC_new(void);

PUBLIC void BORROW(VALUE *val);
PUBLIC void UNBORROW(VALUE *val);
PUBLIC void RELEASE(VALUE *val);
PUBLIC void RELEASE_many(VALUE *val, int n);

#define RELEASE_MANY(_val, _n) \
{ \
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
}

PUBLIC void EXEC_release_return_value(void);
PUBLIC void EXEC_quit(void);

PUBLIC void EXEC_dup(int n);

#define COPY_VALUE(_dst, _src) \
{ \
  long *d = (long *)(_dst); \
  long *s = (long *)(_src); \
  d[0] = s[0]; \
  d[1] = s[1]; \
  d[2] = s[2]; \
  d[3] = s[3]; \
}

#endif /* */
