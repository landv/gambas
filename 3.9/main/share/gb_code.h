/***************************************************************************

  gb_code.h

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

#ifndef __GB_CODE_H
#define __GB_CODE_H

#include "gb_pcode.h"

#ifndef __CODE_C
EXTERN short CODE_stack_usage;
#endif


/* Number of instruction added to a function code buffer at once. Must be a power of 2 */
#define CODE_INSTR_INC 1024
#define CODE_NO_POS (ushort)0xFFFF


#ifdef PROJECT_EXEC

void CODE_begin_function(void);
void CODE_end_function(void);
bool CODE_popify_last(void);

#else

#include "gb_common_swap.h"

void CODE_begin_function(FUNCTION *func);
void CODE_end_function(FUNCTION *func);

bool CODE_popify_last(void);
bool CODE_check_statement_last(void);
bool CODE_check_pop_local_last(short *local);
bool CODE_check_jump_not(void);

bool CODE_check_varptr(void);
bool CODE_check_ismissing(void);

void CODE_allow_break(void);
//void CODE_break(void);

void CODE_pop_local(short num);
//void CODE_pop_param(short num);
void CODE_pop_global(short global, bool is_static);
void CODE_pop_symbol(short symbol);
void CODE_pop_unknown(short symbol);
void CODE_pop_optional(short num);

//void CODE_push_special(short spec);
void CODE_push_event(short event);
void CODE_push_extern(short index);

void CODE_jump(void);
void CODE_gosub(int ctrl_local);
void CODE_on(uchar num);
void CODE_jump_first(short local);
void CODE_jump_next(void);
void CODE_jump_if_true(void);
void CODE_jump_if_false(void);
void CODE_jump_length(ushort src, ushort dst);
void CODE_first(short local);
void CODE_next(bool drop);

void CODE_new(ushort nparam, bool array, bool event);

void CODE_quit(bool ret);
void CODE_stop(void);

void CODE_event(bool on);
void CODE_stop_event(void);

void CODE_try(void);
void CODE_end_try(void);
void CODE_catch(void);

void CODE_pop_ctrl(short num);

#endif /* PROJECT_COMP */

ushort CODE_get_current_pos(void);
ushort CODE_set_current_pos(ushort pos);
void CODE_ignore_next_stack_usage(void);

void CODE_dump(PCODE *code, int count);

void CODE_push_number(int value);
void CODE_push_const(ushort value);

void CODE_push_local(short num);
//void CODE_push_param(short num);
void CODE_push_array(short nparam);
void CODE_push_global(short global, bool is_static, bool is_function);
void CODE_push_symbol(short symbol);
void CODE_push_unknown(short symbol);
void CODE_push_unknown_event(short symbol);
void CODE_push_class(short class);

void CODE_op(short op, short subcode, short nparam, bool fixed);

void CODE_push_me(bool);
void CODE_push_super(bool);
void CODE_push_last(void);
void CODE_push_null(void);
void CODE_push_void_string();
void CODE_push_boolean(bool value);
void CODE_push_inf(bool neg);
void CODE_push_complex();

void CODE_push_vargs();
void CODE_drop_vargs();

void CODE_dup(void);

void CODE_return(int return_value);

void CODE_push_char(uchar car);
void CODE_push_void(void);

void CODE_subr(short subr, short nparam, short optype, bool fixed);
//void CODE_subr_output(short subr, short nparam, int output);

void CODE_call(short nparam);
void CODE_call_byref(short nparam, uint64_t byref);
void CODE_byref(uint64_t byref);
void CODE_drop(void);
void CODE_push_return(void);

void CODE_nop(void);

void CODE_string_add(void);

#endif
