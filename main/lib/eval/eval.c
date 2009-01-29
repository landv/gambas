/***************************************************************************

  eval.c

  Expression evaluator

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

#define __EVAL_C

#include "gb_alloc_override.h"
#include "gb_common.h"

#include "gb_array.h"

#include "eval_trans.h"
#include "gb_code.h"
#include "eval.h"

/*#define DEBUG*/

PUBLIC EXPRESSION *EVAL;
PUBLIC EXPRESSION EVAL_read_expr;


PUBLIC void EVAL_init(void)
{
  RESERVED_init();
  CLEAR(&EVAL_read_expr);
}


PUBLIC void EVAL_exit(void)
{
  EVAL_clear(&EVAL_read_expr);
  RESERVED_exit();
}

PUBLIC GB_VALUE *EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION get_value)
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
  EVAL->exec_class.state = CS_READY;
  EVAL->exec_class.load = &EVAL->class_load;

  return GB.Eval(EVAL, get_value);
}


PUBLIC void EVAL_clear(EXPRESSION *expr)
{
  ARRAY_delete(&expr->tree);

  ARRAY_delete(&expr->var);
  ARRAY_delete(&expr->unknown);
  ARRAY_delete(&expr->class);
  ARRAY_delete(&expr->cst);

  TABLE_delete(&expr->string);
  TABLE_delete(&expr->table);

  if (expr->pattern)
    FREE(&expr->pattern, "EVAL_clear");
  if (expr->code)
    FREE(&expr->code,"EVAL_clear");
}


PUBLIC void EVAL_start(EXPRESSION *expr)
{
  ALLOC(&expr->pattern, sizeof(PATTERN) * (16 + expr->len), "EVAL_start");
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

PUBLIC bool EVAL_compile(EXPRESSION *expr)
{
  bool error = FALSE;

  EVAL = expr;

  EVAL_clear(EVAL);

  if (expr->len == 0)
    return TRUE;

  EVAL_start(EVAL);

  TRY
  {
    EVAL_read();
    EVAL_translate();

    EVAL->stack_usage = CODE_stack_usage;
  }
  CATCH
  {
    EVAL_clear(EVAL);
    error = TRUE;
  }
  END_TRY

  #ifdef DEBUG
  CODE_dump(EVAL->code);
  printf("Stack usage = %d\n", CODE_stack_usage);
  #endif

  return error;
}


PUBLIC void EVAL_new(EXPRESSION **expr, char *src, int len)
{
  GB.Alloc((void **)expr, sizeof(EXPRESSION));
  CLEAR(*expr);
  GB.NewString(&((*expr)->source), src, len);
  GB.AddString(&((*expr)->source), "\n\0", 2);
  (*expr)->len = len + 2;
  /*(*expr)->option = option;*/
}


PUBLIC void EVAL_free(EXPRESSION **pexpr)
{
  EVAL_clear(*pexpr);
  GB.FreeString(&(*pexpr)->source);
  GB.Free((void **)pexpr);
}


PUBLIC int EVAL_add_constant(CLASS_CONST *cst)
{
  int num;
  CLASS_CONST *desc;

  num =  ARRAY_count(EVAL->cst);

  desc = ARRAY_add(&EVAL->cst);
  *desc = *cst;

  return num;
}


PUBLIC int EVAL_add_class(char *name)
{
  int num;
  CLASS **desc;

  num =  ARRAY_count(EVAL->class);

  desc = ARRAY_add(&EVAL->class);
  *desc = GB.FindClassLocal(name);

  /*sym->class = num + 1;*/

  return num;
}


PUBLIC int EVAL_add_unknown(char *name)
{
  int num;
  char **desc;

  num =  ARRAY_count(EVAL->unknown);

  desc = ARRAY_add(&EVAL->unknown);
  *desc = name;

  return num;
}


PUBLIC int EVAL_add_variable(int index)
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


