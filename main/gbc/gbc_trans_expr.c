/***************************************************************************

  trans_expr.c

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

#define _TRANS_EXPR_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_compile.h"

#include "gbc_trans.h"
#include "gb_code.h"

/*#define DEBUG*/

//static bool _accept_statement = FALSE;

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
     Gère le cas où on a codé un subr sans mettre de parenthèses
     => nparam = 0
  */

  return 0;
}


static void push_number(int index)
{
  TRANS_NUMBER number;
  TRANS_DECL decl;

  if (TRANS_get_number(index, &number))
    THROW(E_SYNTAX);

  if (number.type == T_INTEGER)
  {
    CODE_push_number(number.ival);
    return;
  }

  CLEAR(&decl);
  decl.type = TYPE_make(number.type, 0, 0);
  decl.index = NO_SYMBOL;
  decl.value = index;
  if (number.type == T_LONG)
  	decl.lvalue = number.lval;
  CODE_push_const(CLASS_add_constant(JOB->class, &decl));
}


static void push_string(int index, bool trans)
{
  TRANS_DECL decl;
  SYMBOL *sym;
  int len;

  sym = TABLE_get_symbol(JOB->class->string, index);
  len = sym->len;

  if (len == 0)
  {
    CODE_push_null();
  }
  else if (len == 1 && !trans)
  {
    CODE_push_char(*(sym->name));
  }
  else
  {
    CLEAR(&decl);

    if (trans)
      decl.type = TYPE_make(T_CSTRING, 0, 0);
    else
      decl.type = TYPE_make(T_STRING, 0, 0);
    decl.index = NO_SYMBOL;
    decl.value = index;

    CODE_push_const(CLASS_add_constant(JOB->class, &decl));
  }
}


static void trans_class(int index)
{
  const char *name;

  if (CLASS_exist_class(JOB->class, index))
    CODE_push_class(CLASS_add_class(JOB->class, index));
  else
  {
    name = TABLE_get_symbol_name(JOB->class->table, index);
    THROW("Unknown identifier: &1", name);
  }
}


static void trans_identifier(int index, boolean first, boolean point, PATTERN next)
{
  CLASS_SYMBOL *sym = CLASS_get_symbol(JOB->class, index);
  bool is_static;
  bool is_func;
  int type;
  CONSTANT *constant;

  if (!TYPE_is_null(sym->local.type) && !point)
  {
    CODE_push_local(sym->local.value);
  }
  else if (!TYPE_is_null(sym->global.type) && !point)
  {
    type = TYPE_get_kind(sym->global.type);

    if (type == TK_CONST)
    {
      if (PATTERN_is_point(next))
        goto __CLASS;

      constant = &JOB->class->constant[sym->global.value];
      type = TYPE_get_id(constant->type);
			if (type == T_BOOLEAN)
				CODE_push_boolean(constant->value);
			else if (type == T_INTEGER)
        CODE_push_number(constant->value);
      else
        CODE_push_const(sym->global.value);
    }
    else if (type == TK_EXTERN)
    {
      if (PATTERN_is_point(next))
        goto __CLASS;

      CODE_push_extern(sym->global.value);
    }
    else if (type == TK_EVENT || type == TK_PROPERTY || type == TK_LABEL)
    {
      goto __CLASS;
    }
    else /* TK_FUNCTION || TK_VARIABLE */
    {
      is_static = TYPE_is_static(sym->global.type);
      is_func = type == TK_FUNCTION;

      if (!is_static && TYPE_is_static(JOB->func->type))
        THROW("Dynamic symbols cannot be used in static function");

      if (is_func && PATTERN_is_point(next))
        goto __CLASS;
      else
        CODE_push_global(sym->global.value, is_static, is_func);
    }
  }
  else
  {
    /*index = CLASS_add_symbol(JOB->class, index);*/

    if (point)
      CODE_push_unknown(CLASS_add_unknown(JOB->class, index));
    /* Class must be declared now ! */
    else
      goto __CLASS;
  }

  return;

__CLASS:

	trans_class(index);

	/*
  if (CLASS_exist_class(JOB->class, index))
    CODE_push_class(CLASS_add_class(JOB->class, index));
  else
  {
    name = TABLE_get_symbol_name(JOB->class->table, index);
    THROW("Unknown identifier: &1", name);
  }
  */
}


static void trans_subr(int subr, short nparam, boolean output)
{
  SUBR_INFO *info = &COMP_subr_info[subr];

  if (nparam < info->min_param)
    THROW("Not enough arguments to &1()", info->name);
  else if (nparam > info->max_param)
    THROW("Too many arguments to &1()", info->name);

  CODE_subr(info->opcode, nparam, info->optype, output, (info->max_param == info->min_param));
}


PUBLIC void TRANS_operation(short op, short nparam, boolean output, PATTERN previous)
{
  COMP_INFO *info = &COMP_res_info[op];

  switch (info->value)
  {
    case OP_PT:

      if (nparam == 0)
        TRANS_use_with();
      else if (!PATTERN_is_identifier(previous))
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
      TRANS_subr(TS_SUBR_ARRAY, nparam);
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


static void trans_expr_from_tree(TRANS_TREE *tree)
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
      trans_identifier(PATTERN_index(pattern), PATTERN_is_first(pattern), PATTERN_is_point(pattern), next_pattern);

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

        CODE_push_me(FALSE);
      }
      else if (PATTERN_is(pattern, RS_SUPER))
      {
        /*if (FUNCTION_is_static(JOB->func))
          THROW("ME cannot be used in a static function");*/

        CODE_push_super(FALSE);
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
      else if (PATTERN_is(pattern, RS_ERROR))
      {
        TRANS_subr(TS_SUBR_ERROR, 0);
      }
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


PUBLIC void TRANS_new(void)
{
  int index;
  int i, nparam;
  boolean array = FALSE;
  boolean event = FALSE;
  boolean collection = FALSE;
  bool check_param = FALSE;

  nparam = 0;

  if (PATTERN_is_class(*JOB->current))
  {
    index = PATTERN_index(*JOB->current);
    CODE_push_class(CLASS_add_class(JOB->class, index));
    nparam = 1;
  }
  else if (PATTERN_is_type(*JOB->current))
  {
    if (!PATTERN_is(JOB->current[1], RS_LSQR))
      THROW("Cannot instanciate native types");

    //CODE_push_number(RES_get_type(PATTERN_index(*JOB->current)));
    CODE_push_class(CLASS_get_array_class(JOB->class, RES_get_type(PATTERN_index(*JOB->current))));
    nparam = 1;
  }
  else if (PATTERN_is(*JOB->current, RS_LBRA))
  {
    /* NEW ("Class", ...) */
    nparam = 0;
    JOB->current--;
    check_param = TRUE;
  }
  else
  	THROW(E_SYNTAX);

  JOB->current++;

  if (PATTERN_is(*JOB->current, RS_LSQR))
  {
    if (collection)
      THROW("Array declaration is forbidden with typed collection");

    JOB->current++;

    if (!PATTERN_is(*JOB->current, RS_RSQR))
    {
      for (i = 0;; i++)
      {
        if (i > MAX_ARRAY_DIM)
          THROW("Too many dimensions");

        TRANS_expression(FALSE);
        nparam++;

        if (PATTERN_is(*JOB->current, RS_RSQR))
          break;

        if (!PATTERN_is(*JOB->current, RS_COMMA))
          THROW("Missing ']'");

        JOB->current++;
      }
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
          THROW("Missing ')'");

        JOB->current++;
      }

      JOB->current++;

      if (check_param && nparam == 0)
        THROW("Not enough argument to New()");
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


PUBLIC void TRANS_expression(boolean check_statement)
{
  TRANS_TREE *tree;

  tree = TRANS_tree(check_statement);

  trans_expr_from_tree(tree);

  ARRAY_delete(&tree);

  if (check_statement)
  {
    /*
    if (!CODE_check_statement_last())
      THROW("This expression cannot be a statement");
    */

    CODE_drop();
  }
}


PUBLIC void TRANS_reference(void)
{
  TRANS_expression(FALSE);

  if (!CODE_popify_last())
    THROW("Invalid assignment");
}


PUBLIC boolean TRANS_affectation(bool dup)
{
  static TRANS_STATEMENT statement[] = {
    //{ RS_NEW, TRANS_new },
    { RS_OPEN, TRANS_open },
    { RS_SHELL, TRANS_shell },
    { RS_EXEC, TRANS_exec },
    { RS_RAISE, TRANS_raise },
    { RS_PIPE, TRANS_pipe },
    { RS_LOCK, TRANS_lock },
    { RS_NONE, NULL }
  };

  TRANS_STATEMENT *st;
  PATTERN *look = JOB->current;
  PATTERN *left, *expr, *after;
  int niv = 0;
  bool equal = FALSE;
  bool stat = FALSE;
  int op;

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
    else if (niv == 0)
    {
      if (PATTERN_is(*look, RS_EQUAL))
      {
        equal = TRUE;
        op = RS_NONE;
        break;
      }
      else if (PATTERN_is_reserved(*look) && RES_is_assignment(PATTERN_index(*look)))
      {
        equal = TRUE;
        op = RES_get_assignment_operator(PATTERN_index(*look));
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

  JOB->current = expr;

  if (op == RS_NONE && (PATTERN_is_reserved(*JOB->current)))
  {
    if (PATTERN_is(*JOB->current, RS_NEW))
    {
      JOB->current++;
      TRANS_in_affectation++;
      TRANS_new();
      TRANS_in_affectation--;
      stat = TRUE;
    }
    else
    {
      for (st = statement; st->id; st++)
      {
        if (PATTERN_is(*JOB->current, st->id))
        {
          JOB->current++;
          TRANS_in_affectation++;
          (*st->func)();
          TRANS_in_affectation--;
          stat = TRUE;
        }
      }
    }
  }

  if (!stat)
  {
    if (op != RS_NONE)
    {
      JOB->current = left;
      TRANS_expression(FALSE);
    }

    JOB->current = expr;
    TRANS_expression(FALSE);
    after = JOB->current;

    if (op != RS_NONE)
      TRANS_operation(op, 2, FALSE, NULL_PATTERN);
  }

  after = JOB->current;

  if (dup)
    CODE_dup();

  JOB->current = left;
  TRANS_reference();

  JOB->current = after;

  return TRUE;
}


