/***************************************************************************

  trans_code.c

  Bytecode generation

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

#define _TRANS_CODE_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_compile.h"
#include "gbc_trans.h"
#include "gb_code.h"
#include "gb_limit.h"

/*#define DEBUG*/

static FUNCTION *func;

static void add_local(int sym_index, TYPE type, int value)
{
  CLASS_SYMBOL *sym;
  PARAM *loc;

  loc = ARRAY_add(&func->local);
  loc->index = sym_index;
  loc->type = type;
  loc->value = value;

  sym = CLASS_declare(JOB->class, sym_index, FALSE);

  sym->local.type = type;
  sym->local.value = value;
}


static void create_local_from_param()
{
  int i;

  for (i = 0; i < func->nparam; i++)
  {
    if (TYPE_get_id(func->param[i].type) != T_NULL)
    {
      add_local(func->param[i].index, func->param[i].type, (i - func->nparam));
    }
  }
}

static void remove_local()
{
  int i;
  CLASS_SYMBOL *sym;

  for (i = 0; i < ARRAY_count(func->local); i++)
  {
    sym = CLASS_get_symbol(JOB->class, func->local[i].index);
    TYPE_clear(&sym->local.type);
  }
}


static bool TRANS_local(void)
{
  //PATTERN *save = JOB->current;
  int sym_index;
  TRANS_DECL decl;
  PATTERN *sym;
  bool many;
  int f;

  if (!TRANS_is(RS_DIM))
    return FALSE;
  /*JOB->current++;
  else if (!TRANS_check_declaration())
    return FALSE;*/

	for(;;)
	{
		many = FALSE;
		sym = JOB->current;

		for(;;)
		{
			if (!PATTERN_is_identifier(*JOB->current))
				THROW(E_SYNTAX);
			JOB->current++;
			if (!TRANS_is(RS_COMMA))
				break;
			many = TRUE;
		}

		f = TT_DO_NOT_CHECK_AS | TT_CAN_ARRAY | TT_CAN_NEW;
		if (!many)
			f |= TT_CAN_SQUARE;

		if (!TRANS_type(f, &decl))
			THROW(E_SYNTAX);

		for(;;)
		{
			sym_index = PATTERN_index(*sym);
			add_local(sym_index, decl.type, func->nlocal);
			sym++;

			func->nlocal++;

			if (TRANS_init_var(&decl))
				CODE_pop_local(func->nlocal - 1);

			if (JOB->verbose)
				printf("LOCAL %s AS %s\n", TABLE_get_symbol_name(JOB->class->table, sym_index), TYPE_get_desc(decl.type));

			if (!PATTERN_is(*sym, RS_COMMA))
				break;
			sym++;
		}

		/*if (!many)
		{
			if (TRANS_init_var(&decl))
				CODE_pop_local(func->nlocal - 1);
		}*/

		if (!TRANS_is(RS_COMMA))
			break;
	}

  return TRUE;
}


void TRANS_statement(void)
{
  static TRANS_STATEMENT statement[] = {
    { RS_EXIT, TRANS_break },
    { RS_BREAK, TRANS_break },
    { RS_CONTINUE, TRANS_continue },
    { RS_GOTO, TRANS_goto },
    { RS_RETURN, TRANS_return },
    { RS_PRINT, TRANS_print },
    { RS_INPUT, TRANS_input },
    { RS_WRITE, TRANS_write },
    { RS_READ, TRANS_read },
    { RS_OPEN, TRANS_open },
    { RS_CLOSE, TRANS_close },
    { RS_SEEK, TRANS_seek },
    { RS_FLUSH, TRANS_flush },
    { RS_STOP, TRANS_stop },
    { RS_QUIT, TRANS_quit },
    { RS_EXEC, TRANS_exec },
    { RS_SHELL, TRANS_shell },
    { RS_WAIT, TRANS_wait },
    { RS_SLEEP, TRANS_sleep },
    { RS_KILL, TRANS_kill },
    { RS_MOVE, TRANS_move },
    { RS_INC, TRANS_inc },
    { RS_DEC, TRANS_dec },
    { RS_SWAP, TRANS_swap },
    { RS_MKDIR, TRANS_mkdir },
    { RS_RMDIR, TRANS_rmdir },
    { RS_COPY, TRANS_copy },
    { RS_RAISE, TRANS_raise },
    { RS_LINK, TRANS_link },
    { RS_LOCK, TRANS_lock },
    { RS_UNLOCK, TRANS_unlock },
    { RS_TRY, TRANS_try },
    { RS_LINE, TRANS_line_input },
    { RS_OUTPUT, TRANS_output_to },
    { RS_DEBUG, TRANS_debug },
    { RS_ERROR, TRANS_error },
    { RS_PIPE, TRANS_pipe },
    { RS_RANDOMIZE, TRANS_randomize },

    { RS_NONE, NULL }
  };

  PATTERN *look = JOB->current;
  TRANS_STATEMENT *st;
  COMP_INFO *info;

  if (PATTERN_is_reserved(look[0]))
  {
    info = &COMP_res_info[PATTERN_index(*look)];

    if (!info->func)
    {
      for (st = statement; st->id; st++)
      {
        if (PATTERN_is(look[0], st->id))
        {
          info->func = st->func;
          break;
        }
      }
      if (!info->func)
        info->func = TS_NO_SUBR;
    }

    if (info->func && info->func != TS_NO_SUBR)
    {
      JOB->current++;
      (*info->func)();
      return;
    }
  }

  if (!TRANS_affectation(FALSE))
    TRANS_expression(TRUE);
}


static void translate_body()
{
  PATTERN *look;
  bool is_proc = (TYPE_get_id(func->type) == T_VOID);
  bool test_newline;
  int line = JOB->line - 1;
  bool just_got_select = FALSE;

  for(;;)
  {
    test_newline = TRUE;
    CODE_allow_break();

    while (line < JOB->line)
    {
      FUNCTION_add_pos_line();
      line++;
    }

    look = JOB->current;

    if (PATTERN_is(look[0], RS_END))
      if (TRANS_is_end_function(is_proc, &look[1]))
        break;

    if (TRANS_newline())
      test_newline = FALSE;
    else if (!TRANS_local())
      break;

    if (test_newline)
      if (!PATTERN_is_newline(*JOB->current))
        THROW(E_UNEXPECTED, READ_get_pattern(JOB->current));
  }


  TRANS_control_init();

  for(;;)
  {
    test_newline = TRUE;
    CODE_allow_break();

    while (line < JOB->line)
    {
      FUNCTION_add_pos_line();
      line++;
    }

    look = JOB->current;

    if (PATTERN_is(look[0], RS_END))
      if (TRANS_is_end_function(is_proc, &look[1]))
        break;

    /*if (PATTERN_is_newline(look[0]))
    {
      JOB->current++;
      JOB->line++;
      continue;
    }*/
    if (TRANS_newline())
    	continue;

    if (just_got_select)
    {
      if (!PATTERN_is(look[0], RS_CASE) && !PATTERN_is(look[0], RS_DEFAULT))
        THROW("Syntax error. CASE or DEFAULT expected after SELECT");
      just_got_select = FALSE;
    }


    if (PATTERN_is_identifier(look[0]) && PATTERN_is(look[1], RS_COLON))
    {
      TRANS_label();
    }
    else if (PATTERN_is(look[0], RS_IF))
    {
      JOB->current++;
      TRANS_if();
    }
    else if (PATTERN_is(look[0], RS_ELSE))
    {
      JOB->current++;
      TRANS_else();
    }
    else if ((PATTERN_is(look[0], RS_END)
              && PATTERN_is(look[1], RS_IF))
             || PATTERN_is(look[0], RS_ENDIF))
    {
      if (PATTERN_is(look[0], RS_END))
        JOB->current += 2;
      else
        JOB->current++;

      TRANS_endif();
    }
    else if (PATTERN_is(look[0], RS_DO))
    {
      JOB->current++;
      TRANS_do(RS_DO);
    }
    else if (PATTERN_is(look[0], RS_WHILE))
    {
      TRANS_do(RS_WHILE);
    }
    else if (PATTERN_is(*look, RS_REPEAT))
    {
      JOB->current++;
      TRANS_do(RS_REPEAT);
    }
    else if (PATTERN_is(look[0], RS_LOOP))
    {
      JOB->current++;
      TRANS_loop(RS_LOOP);
    }
    else if (PATTERN_is(look[0], RS_UNTIL))
    {
      TRANS_loop(RS_UNTIL);
    }
    else if (PATTERN_is(look[0], RS_WEND))
    {
      JOB->current++;
      TRANS_loop(RS_WEND);
    }
    else if (PATTERN_is(look[0], RS_FOR))
    {
      if (PATTERN_is(look[1], RS_EACH))
      {
        JOB->current += 2;
        TRANS_for_each();
      }
      else
      {
        JOB->current++;
        TRANS_for();
      }
    }
    else if (PATTERN_is(look[0], RS_NEXT))
    {
      JOB->current++;
      TRANS_next();
    }
    else if (PATTERN_is(look[0], RS_SELECT))
    {
      JOB->current++;
      TRANS_select();
      just_got_select = TRUE;
    }
    else if (PATTERN_is(look[0], RS_CASE))
    {
      JOB->current++;
      if (PATTERN_is(look[1], RS_ELSE))
      {
        JOB->current++;
        TRANS_default();
      }
      else
        TRANS_case();
    }
    else if (PATTERN_is(look[0], RS_DEFAULT))
    {
      JOB->current++;
      TRANS_default();
    }
    else if (PATTERN_is(look[0], RS_END)
             && PATTERN_is(look[1], RS_SELECT))
    {
      JOB->current += 2;
      TRANS_end_select();
    }
    else if (PATTERN_is(look[0], RS_FINALLY))
    {
      JOB->current++;
      TRANS_finally();
    }
    else if (PATTERN_is(look[0], RS_CATCH))
    {
      JOB->current++;
      TRANS_catch();
    }
    else if (PATTERN_is(*look, RS_WITH))
    {
      JOB->current++;
      TRANS_with();
    }
    else if (PATTERN_is(look[0], RS_END)
             && PATTERN_is(look[1], RS_WITH))
    {
      JOB->current += 2;
      TRANS_end_with();
    }
    else
      TRANS_statement();

    /*
    if (next_newline)
    {
      for(;;)
      {
        if (PATTERN_is_NEWLINE(*JOB->current)
            || PATTERN_is_END(*JOB->current))
          break;
        JOB->current++;
      }
    }
    */

    if (test_newline)
      if (!PATTERN_is_newline(*JOB->current))
        THROW(E_UNEXPECTED, READ_get_pattern(JOB->current));

  }

  TRANS_control_exit();
}

static void trans_call(const char *name, int nparam)
{
  CLASS_SYMBOL *sym;

  if (!TABLE_find_symbol(JOB->class->table, name, strlen(name), (SYMBOL **)(void *)&sym, NULL))
    return;

  if (TYPE_get_kind(sym->global.type) != TK_FUNCTION)
    return;

  CODE_push_global(sym->global.value, FALSE, TRUE);
  CODE_call(nparam, FALSE);
  CODE_drop();
}


void TRANS_code(void)
{
  int i;
  bool debug;

  debug = JOB->debug;

  for (i = 0; i < ARRAY_count(JOB->class->function); i++)
  {
    func = &JOB->class->function[i];

    CODE_begin_function(func);

    if (JOB->verbose)
      printf("Compiling %s()...\n", TABLE_get_symbol_name(JOB->class->table, func->name));

    /* Do not debug implicite or generated functions */
    if (!func->start || func->name == NO_SYMBOL || TABLE_get_symbol_name(JOB->class->table, func->name)[0] == '$')
      JOB->nobreak = TRUE;
    else
      JOB->nobreak = FALSE;

    /* fonction implicite ? */
    if (!func->start)
    {
      if ((i == FUNC_INIT_DYNAMIC) && (JOB->form != NULL))
      {
        //CODE_event(FALSE);
        trans_call("$load", 0);
        //CODE_event(TRUE);
      }

      CODE_op(C_RETURN, 0, TRUE);
      if (JOB->verbose)
        CODE_dump(func->code, func->ncode);
      continue;
    }

    JOB->line = func->line;
    JOB->current = func->start;
    JOB->func = func;

    create_local_from_param();

    translate_body();

    CODE_return(0);

    CODE_end_function(func);
    FUNCTION_add_pos_line();

    func->stack = func->nlocal + func->nctrl + CODE_stack_usage;

    if (JOB->verbose)
    {
      CODE_dump(func->code, func->ncode);
      printf("%d local(s) %d control(s) ", func->nlocal, func->nctrl);
      printf("%d stack\n", func->stack);
      printf("\n");
    }

    remove_local();
  }

  CLASS_check_properties(JOB->class);

  JOB->func = NULL;
}



bool TRANS_init_var(TRANS_DECL *decl)
{
  int i;
  TRANS_ARRAY *array;
  //PATTERN *save;

  if (decl->is_new)
  {
    if (TYPE_is_array(decl->type) && decl->array.ndim > 0)
    {
      array = &decl->array;

      if (TYPE_is_object(decl->type))
        CODE_push_class(TYPE_get_value(decl->type));
      else
        CODE_push_number(TYPE_get_id(decl->type));

      for (i = 0; i < array->ndim; i++)
        CODE_push_number(array->dim[i]);

      CODE_new(array->ndim + 1, TRUE, FALSE);
      return TRUE;
    }
    else if (decl->is_new)
    {
      JOB->current = decl->init;
      TRANS_new();
      return TRUE;
    }
  }
  else if (decl->init)
  {
    JOB->current = decl->init;
    TRANS_expression(FALSE);
    return TRUE;
  }

  return FALSE;
}


/*
void TRANS_init_object()
{
}
*/

void TRANS_init_optional(TRANS_PARAM *param)
{
  PATTERN *look = param->optional;
  PATTERN *save;

  if (look == NULL)
    return;

  save = JOB->current;

  if (PATTERN_is(*look, RS_COMMA) || PATTERN_is(*look, RS_RBRA))
  {
    CODE_push_void();
  }
  else
  {
    if (!PATTERN_is(*look, RS_EQUAL))
      THROW("Syntax error. Invalid optional parameter");

    look++;
    JOB->current = look;
    TRANS_expression(FALSE);

    if (!PATTERN_is(*JOB->current, RS_COMMA) && !PATTERN_is(*JOB->current, RS_RBRA))
      THROW("Syntax error. Invalid optional parameter");
  }

  JOB->current = save;
}
