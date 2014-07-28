/***************************************************************************

  eval.c

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

#define __EVAL_C

#include "gb_alloc_override.h"
#include "gb_common.h"
#include "gb_error.h"
#include "gb_array.h"

#include "eval_trans.h"
#include "gb_code.h"
#include "eval.h"

#include "gb_common_case_temp.h"

/*#define DEBUG*/

EXPRESSION *EVAL;
EXPRESSION EVAL_read_expr;


void EVAL_init(void)
{
  RESERVED_init();
  CLEAR(&EVAL_read_expr);
}


void EVAL_exit(void)
{
  EVAL_clear(&EVAL_read_expr, FALSE);
  RESERVED_exit();
}

GB_VALUE *EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION get_value)
{
  EVAL = expr;

  /* Creates a class and a function for the evaluation context */

  CLEAR(&EVAL->func);
  EVAL->func.type = T_VARIANT;
  EVAL->func.n_param = EVAL->nvar;
  EVAL->func.npmin = EVAL->nvar;
  EVAL->func.stack_usage = EVAL->stack_usage;
  EVAL->func.code = EVAL->code;

  CLEAR(&EVAL->class_load);
  EVAL->class_load.cst = EVAL->cst;
  EVAL->class_load.func = &EVAL->func;
  EVAL->class_load.class_ref = EVAL->class;
  EVAL->class_load.unknown = EVAL->unknown;

  CLEAR(&EVAL->exec_class);
  /*_class.class = CLASS_class;*/
  EVAL->exec_class.ref = 1;
  EVAL->exec_class.count = 1;
  EVAL->exec_class.name = ".Eval";
  EVAL->exec_class.loaded = TRUE;
  EVAL->exec_class.ready = TRUE;
  EVAL->exec_class.load = &EVAL->class_load;

  return GB.Eval(EVAL, get_value);
}


void EVAL_clear(EXPRESSION *expr, bool keep_error)
{
  ARRAY_delete(&expr->tree);

  ARRAY_delete(&expr->var);
  ARRAY_delete(&expr->unknown);
  ARRAY_delete(&expr->class);
  ARRAY_delete(&expr->cst);

  TABLE_delete(&expr->string);
  TABLE_delete(&expr->table);

  if (expr->pattern)
    FREE(&expr->pattern);
  if (expr->code)
    FREE(&expr->code);
	
	if (!keep_error)
		GB.FreeString(&expr->error);
}


void EVAL_start(EXPRESSION *expr)
{
  ALLOC(&expr->pattern, sizeof(PATTERN) * (16 + expr->len));
  expr->pattern_count = 0;

  TABLE_create(&expr->table, sizeof(EVAL_SYMBOL), EVAL->analyze ? TF_NORMAL : TF_IGNORE_CASE);
  TABLE_create(&expr->string, sizeof(SYMBOL), TF_NORMAL);

  ARRAY_create(&expr->cst);
  ARRAY_create(&expr->class);
  ARRAY_create(&expr->unknown);
  expr->code = NULL;
  expr->ncode = 0;
  expr->ncode_max = 0;
  ARRAY_create(&expr->var);

  expr->nvar = 0;
}

bool EVAL_compile(EXPRESSION *expr, bool assign)
{
  bool error = FALSE;

  EVAL = expr;

  EVAL_clear(EVAL, FALSE);

  if (expr->len == 0)
    return TRUE;
	
  EVAL_start(EVAL);

  TRY
  {
    EVAL_read();
		
		EVAL->current = EVAL->pattern;
		
		if (PATTERN_is(*EVAL->current, RS_LET))
		{
			EVAL->current++;
			assign = TRUE;
		}
		
		if (assign)
		{
			if (!TRANS_affectation())
				THROW(E_SYNTAX);
		}
		else
			TRANS_expression();
		
		if (!PATTERN_is_end(*EVAL->current))
			THROW(E_SYNTAX);
		
		CODE_return(1);

    EVAL->stack_usage = CODE_stack_usage;
  }
  CATCH
  {
    EVAL_clear(EVAL, TRUE);
    error = TRUE;
  }
  END_TRY

  #ifdef DEBUG
  CODE_dump(EVAL->code);
  printf("Stack usage = %d\n", CODE_stack_usage);
  #endif

  return error;
}

bool EVAL_get_assignment_symbol(EXPRESSION *expr, const char **name, int *len)
{
	ushort code = EVAL->assign_code;
	int index;
  EVAL_SYMBOL *sym;
	
	if ((code & 0xFF00) == C_POP_PARAM)
	{
		index = -((signed char)(code & 0xFF)) - 1;
		sym = (EVAL_SYMBOL *)TABLE_get_symbol(EVAL->table, EVAL->var[index]);
		*name = sym->sym.name;
		*len = sym->sym.len;
		return FALSE;
	}
	else
		return TRUE;
}

void EVAL_new(EXPRESSION **expr, char *src, int len)
{
  GB.Alloc((void **)expr, sizeof(EXPRESSION));
  CLEAR(*expr);
  (*expr)->source = GB.NewString(src, len);
  (*expr)->source = GB.AddString((*expr)->source, "\n\0", 2);
  (*expr)->len = len + 2;
  /*(*expr)->option = option;*/
}


void EVAL_free(EXPRESSION **pexpr)
{
  EVAL_clear(*pexpr, FALSE);
  GB.FreeString(&(*pexpr)->source);
  GB.Free((void **)pexpr);
}


int EVAL_add_constant(CLASS_CONST *cst)
{
  int num;
  CLASS_CONST *desc;

  num =  ARRAY_count(EVAL->cst);

  desc = ARRAY_add(&EVAL->cst);
  *desc = *cst;

  return num;
}


int EVAL_add_class(char *name)
{
  int num;
  CLASS **desc;

  num =  ARRAY_count(EVAL->class);

  desc = ARRAY_add(&EVAL->class);
  *desc = (CLASS *)GB.FindClassLocal(name);

  /*sym->class = num + 1;*/

  return num;
}


int EVAL_add_unknown(char *name)
{
  int num;
  char **desc;

  num =  ARRAY_count(EVAL->unknown);

  desc = ARRAY_add(&EVAL->unknown);
  *desc = name;

  return num;
}


int EVAL_add_variable(int index)
{
  EVAL_SYMBOL *sym;

  sym = (EVAL_SYMBOL *)TABLE_get_symbol(EVAL->table, index);

  if (sym->local == 0)
  {
    EVAL->nvar++;
    sym->local = EVAL->nvar;

    *((int *)ARRAY_add(&EVAL->var)) = index;
  }

  return (-sym->local);
}

