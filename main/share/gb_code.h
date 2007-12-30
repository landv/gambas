/***************************************************************************

  code.h

  P-code assembler

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_CODE_H
#define __GB_CODE_H

#include "gb_pcode.h"

#ifndef __CODE_C
EXTERN short CODE_stack_usage;
#endif


#ifdef PROJECT_EXEC

PUBLIC void CODE_begin_function(void);
PUBLIC void CODE_end_function(void);

#else

#include "gb_common_swap.h"

PUBLIC void CODE_begin_function(FUNCTION *func);
PUBLIC void CODE_end_function(FUNCTION *func);

PUBLIC boolean CODE_popify_last(void);
PUBLIC boolean CODE_check_statement_last(void);
PUBLIC boolean CODE_check_pop_local_last(short *local);

PUBLIC void CODE_allow_break(void);
PUBLIC void CODE_break(void);

PUBLIC void CODE_pop_local(short num);
//PUBLIC void CODE_pop_param(short num);
PUBLIC void CODE_pop_global(short global, boolean is_static);
PUBLIC void CODE_pop_symbol(short symbol);
PUBLIC void CODE_pop_unknown(short symbol);
PUBLIC void CODE_pop_optional(short num);

//PUBLIC void CODE_push_special(short spec);
PUBLIC void CODE_push_event(short event);
PUBLIC void CODE_push_extern(short index);

PUBLIC void CODE_jump(void);
PUBLIC void CODE_jump_first(short local);
PUBLIC void CODE_jump_next(void);
PUBLIC void CODE_jump_if_true(void);
PUBLIC void CODE_jump_if_false(void);
PUBLIC void CODE_jump_length(short src, short dst);
PUBLIC void CODE_first(short local);
PUBLIC void CODE_next(bool drop);

PUBLIC void CODE_new(ushort nparam, boolean array, boolean event);

PUBLIC void CODE_quit(void);
PUBLIC void CODE_stop(void);

PUBLIC void CODE_event(boolean on);
PUBLIC void CODE_stop_event(void);

PUBLIC void CODE_try(void);
PUBLIC void CODE_end_try(void);
PUBLIC void CODE_catch(void);

PUBLIC void CODE_pop_ctrl(short num);

#endif /* PROJECT_COMP */

PUBLIC long CODE_get_current_pos(void);
PUBLIC void CODE_ignore_next_stack_usage(void);

PUBLIC void CODE_dump(PCODE *code);

PUBLIC void CODE_push_number(long value);
PUBLIC void CODE_push_const(short value);

PUBLIC void CODE_push_local(short num);
//PUBLIC void CODE_push_param(short num);
PUBLIC void CODE_push_array(short nparam);
PUBLIC void CODE_push_global(short global, boolean is_static, boolean is_function);
PUBLIC void CODE_push_symbol(short symbol);
PUBLIC void CODE_push_unknown(short symbol);
PUBLIC void CODE_push_class(short class);

PUBLIC void CODE_op(short op, short nparam, boolean fixed);

PUBLIC void CODE_push_me(bool);
PUBLIC void CODE_push_super(bool);
PUBLIC void CODE_push_last(void);
PUBLIC void CODE_push_null(void);
PUBLIC void CODE_push_boolean(boolean value);

PUBLIC void CODE_dup(void);

PUBLIC void CODE_return(int return_value);

PUBLIC void CODE_push_char(char car);
PUBLIC void CODE_push_void(void);

PUBLIC void CODE_subr(short subr, short nparam, short optype, boolean output, boolean fixed);
PUBLIC void CODE_subr_output(short subr, short nparam, int output);

PUBLIC void CODE_call(short nparam, boolean output);
PUBLIC void CODE_drop(void);
PUBLIC void CODE_push_return(void);

#endif
