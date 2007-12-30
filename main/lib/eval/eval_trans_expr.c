/***************************************************************************

  eval_trans_expr.c

  Expression compiler

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

#define __EVAL_TRANS_EXPR_C

#define PROJECT_EXEC

#include "gb_common.h"
#include <ctype.h>

#include "gb_error.h"
#include "gb_reserved.h"

#include "gb_code.h"
#include "eval_trans.h"
#include "eval.h"

/*#define DEBUG*/

static int subr_array_index = -1;

static short get_nparam(PATTERN *tree, int *index)
{
  PATTERN pattern;

  if (*index < (ARRAY_count(tree) - 1))
  {
    pattern = tree[*index + 1];
    if (PATTERN_is_param(pattern))
    {
      (*index)++;
      return (short)PATTERN_index(pattern);
    }
  }

  /*
     G�e le cas o on a cod�un subr sans mettre de parenth�es
     => nparam = 0
  */

  return 0;
}


static void push_number(int index)
{
  TRANS_NUMBER number;
  CLASS_CONST cst;
  SYMBOL *sym;
  GB_VALUE value;

  if (TRANS_get_number(index, &number))
    THROW(E_SYNTAX);

  if (number.type == T_INTEGER)
  {
    CODE_push_number(number.ival);
    return;
  }

  sym = TABLE_get_symbol(EVAL->table, index);

  cst.type = T_FLOAT;
  if (GB.NumberFromString(GB_NB_READ_FLOAT, sym->name, sym->len, &value))
    THROW("Bad floating point constant");

  cst._float.value = ((GB_FLOAT *)(void *)&value)->value;

  CODE_push_const(EVAL_add_constant(&cst));
}


static void push_string(int index, bool trans)
{
  CLASS_CONST cst;
  SYMBOL *sym;
  int len;

  sym = TABLE_get_symbol(EVAL->string, index);
  len = sym->len;

  if (len == 0)
  {
    CODE_push_null();
  }
  else if (len == 1)
  {
    CODE_push_char(*(sym->name));
  }
  else
  {
    cst.type = trans ? T_CSTRING : T_STRING;
    cst._string.addr = sym->name;
    cst._string.len = len;

    CODE_push_const(EVAL_add_constant(&cst));
  }
}


/*
static void push_class(long index)
{
  TRANS_DECL decl;

  decl.type = TYPE_make(T_STRING, 0, 0);
  decl.index = NO_SYMBOL;
  decl.value = index;
  CODE_push_class(CLASS_add_constant(JOB->class, &decl));
}
*/


static void trans_class(int index)
{
  SYMBOL *sym = TABLE_get_symbol(EVAL->table, index);

	if (GB.ExistClass(sym->name))
    CODE_push_class(EVAL_add_class(sym->name));
  else
  {
    THROW("Unknown class");
  }
}


static void trans_identifier(int index, boolean first, boolean point)
{
  SYMBOL *sym = TABLE_get_symbol(EVAL->table, index);

  /* z�o terminal */

  sym->name[sym->len] = 0;

  if (point)
  {
    CODE_push_unknown(EVAL_add_unknown(sym->name));
  }
  else if (first && GB.ExistClass(sym->name))
  {
    //printf("%.*s %s\n", sym->symbol.len, sym->symbol.name, isupper(*sym->symbol.name) ? "U" : "l");
    CODE_push_class(EVAL_add_class(sym->name));
  }
  else
  {
    CODE_push_local(EVAL_add_variable(index));
  }
}


static void trans_subr(int subr, short nparam, boolean output)
{
  SUBR_INFO *info = &COMP_subr_info[subr];

  //printf("trans_subr: %s: %d %d\n", info->name, info->min_param, info->max_param);

  if (nparam < info->min_param)
    THROW2("Not enough arguments to &1()", info->name);
  else if (nparam > info->max_param)
    THROW2("Too many arguments to &1()", info->name);

  CODE_subr(info->opcode, nparam, info->optype, output, (info->max_param == info->min_param));
}


PUBLIC void TRANS_operation(short op, short nparam, boolean output, PATTERN previous)
{
  COMP_INFO *info = &COMP_res_info[op];

  switch (info->value)
  {
    case OP_PT:

      /*if (nparam == 0)
        TRANS_use_with();*/
      if (!PATTERN_is_identifier(previous))
        THROW(E_SYNTAX);
      break;

    case OP_EXCL:
     if (!PATTERN_is_identifier(previous))
      THROW(E_SYNTAX);
     break;

    case OP_LSQR:
      CODE_push_array(nparam);
      break;

    case OP_RSQR:
      if (subr_array_index < 0)
        TABLE_find_symbol(COMP_subr_table, ".Array", 6, NULL, &subr_array_index);

      trans_subr(subr_array_index, nparam, FALSE);
      break;

    case OP_LBRA:
      CODE_call(nparam, output);
      break;

    case OP_MINUS:
      if (nparam == 1)
        CODE_op(C_NEG, nparam, TRUE);
      else
        CODE_op(info->code, nparam, TRUE);
      break;

    default:

      CODE_op(info->code, nparam, (info->flag != RSF_OPN));
  }
}


static void trans_expr_from_tree(PATTERN *tree)
{
  int i;
  short nparam;
  int count;
  PATTERN pattern, next_pattern, prev_pattern;

  count = ARRAY_count(tree) - 1;
  pattern = NULL_PATTERN;

  for (i = 0; i <= count; i++)
  {
    prev_pattern = pattern;
    pattern = tree[i];
    if (i < count)
      next_pattern = tree[i + 1];
    else
      next_pattern = NULL_PATTERN;

    if (PATTERN_is_number(pattern))
      push_number(PATTERN_index(pattern));

    else if (PATTERN_is_string(pattern))
      push_string(PATTERN_index(pattern), FALSE);

    else if (PATTERN_is_tstring(pattern))
      push_string(PATTERN_index(pattern), TRUE);

    else if (PATTERN_is_identifier(pattern))
      trans_identifier(PATTERN_index(pattern), PATTERN_is_first(pattern), PATTERN_is_point(pattern));

    else if (PATTERN_is_class(pattern))
      trans_class(PATTERN_index(pattern));

    else if (PATTERN_is_subr(pattern))
    {
      nparam = get_nparam(tree, &i);
      trans_subr(PATTERN_index(pattern), nparam, PATTERN_is_output(pattern));
    }

    else if (PATTERN_is_reserved(pattern))
    {
      if (PATTERN_is(pattern, RS_TRUE))
      {
        CODE_push_boolean(TRUE);
      }
      else if (PATTERN_is(pattern, RS_FALSE))
      {
        CODE_push_boolean(FALSE);
      }
      else if (PATTERN_is(pattern, RS_NULL))
      {
        CODE_push_null();
      }
      else if (PATTERN_is(pattern, RS_ME))
      {
        /*if (FUNCTION_is_static(JOB->func))
          THROW("ME cannot be used in a static function");*/

        CODE_push_me(TRUE);
      }
      else if (PATTERN_is(pattern, RS_SUPER))
      {
        /*if (FUNCTION_is_static(JOB->func))
          THROW("ME cannot be used in a static function");*/

        CODE_push_super(TRUE);
      }
      else if (PATTERN_is(pattern, RS_LAST))
      {
        CODE_push_last();
      }
      /*
      else if (PATTERN_is(pattern, RS_AT))
      {
        if (!CODE_popify_last())
          THROW("Invalid output parameter");
      }
      */
      else if (PATTERN_is(pattern, RS_COMMA))
      {
        CODE_drop();
      }
      /*else if (PATTERN_is(pattern, RS_ERROR))
      {
        TRANS_subr(TS_SUBR_ERROR, 0);
      }*/
      else if (PATTERN_is(pattern, RS_OPTIONAL))
      {
        CODE_push_void();
      }
      else
      {
        nparam = get_nparam(tree, &i);
        TRANS_operation((short)PATTERN_index(pattern), nparam, PATTERN_is_output(pattern), prev_pattern);
      }
    }
  }
}

#if 0
static void trans_new(void)
{
  long index;
  int i, nparam;
  boolean array = FALSE;
  boolean event = FALSE;
  boolean collection = FALSE;

  if (PATTERN_is_identifier(*JOB->current))
  {
    index = PATTERN_index(*JOB->current);
    CODE_push_class(CLASS_add_class(JOB->class, index));
    nparam = 1;
  }
  else if (PATTERN_is_type(*JOB->current))
  {
    if (PATTERN_is(JOB->current[1], RS_LSQR))
    {
      CODE_push_number(RES_get_type(PATTERN_index(*JOB->current)));
      nparam = 1;
    }
    else
      THROW("Cannot instanciate native types");
  }

  JOB->current++;

  if (PATTERN_is(*JOB->current, RS_LSQR))
  {
    if (collection)
      THROW("Array declaration is forbidden with typed collection");

    JOB->current++;

    for (i = 0;; i++)
    {
      if (i > MAX_ARRAY_DIM)
        THROW("Too many dimensions");

      TRANS_expression(FALSE);
      nparam++;

      if (PATTERN_is(*JOB->current, RS_RSQR))
        break;

      if (!PATTERN_is(*JOB->current, RS_COMMA))
        THROW("Comma missing");

      JOB->current++;
    }

    JOB->current++;
    array = TRUE;
  }
  else
  {
    if (PATTERN_is(*JOB->current, RS_LBRA))
    {
      JOB->current++;

      for(;;)
      {
        if (nparam > MAX_PARAM_FUNC)
          THROW("Too many arguments");

        if (PATTERN_is(*JOB->current, RS_AT))
          THROW("NEW cannot have output parameters");

        TRANS_expression(FALSE);
        nparam++;

        if (PATTERN_is(*JOB->current, RS_RBRA))
          break;

        if (!PATTERN_is(*JOB->current, RS_COMMA))
          THROW("Comma missing");

        JOB->current++;
      }

      JOB->current++;
    }

    if (PATTERN_is(*JOB->current, RS_AS))
    {
      JOB->current++;
      TRANS_expression(FALSE);
      nparam++;
      event = TRUE;
    }

    /*
    CODE_call(nparam, FALSE);
    CODE_drop();
    */
  }

  if (collection)
    CODE_new(nparam, TRUE, event);
  else
    CODE_new(nparam, array, event);
}
#endif

PUBLIC void EVAL_translate()
{
  TRANS_tree();

  trans_expr_from_tree(EVAL->tree);

  ARRAY_delete(&EVAL->tree);

  CODE_return(2);
}

/*
PUBLIC void TRANS_reference(void)
{
  TRANS_expression(FALSE);

  if (!CODE_popify_last())
    THROW("Invalid assignment");
}
*/

/*
PUBLIC boolean TRANS_affectation()
{
  PATTERN *look = JOB->current;
  PATTERN *left, *expr, *after;
  int niv = 0;
  boolean equal = FALSE;

  for(;;)
  {
    if (PATTERN_is_newline(*look) || PATTERN_is_end(*look))
      break;

    if (PATTERN_is(*look, RS_LBRA) || PATTERN_is(*look, RS_LSQR))
    {
      niv++;
    }
    else if (PATTERN_is(*look, RS_RBRA) || PATTERN_is(*look, RS_RSQR))
    {
      if (niv > 0)
        niv--;
    }
    else if (PATTERN_is(*look, RS_EQUAL))
    {
      if (niv == 0)
      {
        equal = TRUE;
        break;
      }
    }

    look++;
  }

  if (!equal)
    return FALSE;

  left = JOB->current;
  *look++ = PATTERN_make(RT_NEWLINE, 0);
  expr = look;

  Can_new = TRUE;
  JOB->current = expr;
  TRANS_expression(FALSE);

  after = JOB->current;
  JOB->current = left;

  TRANS_reference();

  JOB->current = after;

  return TRUE;
}
*/
