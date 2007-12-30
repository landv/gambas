/***************************************************************************

  trans_tree.c

  Convert expression to a tree

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

#define __GBC_TRANS_TREE_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_compile.h"

#include "gbc_trans.h"
#include "gb_code.h"
#include "gbc_read.h"

/*#define DEBUG*/

static short level;
static PATTERN *current;
static TRANS_TREE *tree = NULL;
/*static PATTERN last_pattern;*/

static void analyze_expr(short priority, short op_main);


static void inc_level()
{
  level++;
  if (level > MAX_EXPR_LEVEL)
   THROW("Expression too complex");
}


static void dec_level()
{
  level--;
}


static void add_pattern(PATTERN pattern)
{
  TRANS_TREE *node;
  short index;

  index = (short)ARRAY_count(tree);

  if (index >= MAX_EXPR_PATTERN)
    THROW("Expression too complex");

  node = ARRAY_add(&tree);
  *node = pattern;
  /*last_pattern = pattern;*/


  /*#ifdef DEBUG
  READ_dump_pattern(&pattern);
  #endif*/
}


static void remove_last_pattern()
{
  if (!ARRAY_count(tree))
    return;

  ARRAY_remove_last(&tree);
}


static PATTERN get_last_pattern(int dep)
{
  short index;

  index = (short)ARRAY_count(tree);
  if (index < dep)
    return NULL_PATTERN;

  return tree[index - dep];
}


static void change_last_pattern(int dep, PATTERN pattern)
{
  short index;

  index = (short)ARRAY_count(tree);
  if (index < dep)
    return;

  tree[index - dep] = pattern;
}


static void check_last_first(int dep)
{
  if (PATTERN_is_identifier(get_last_pattern(dep)))
    change_last_pattern(dep, PATTERN_set_flag(get_last_pattern(dep), RT_FIRST));
}


static void add_reserved_pattern(int reserved)
{
  add_pattern(PATTERN_make(RT_RESERVED, reserved));
}


static void add_operator_output(short op, short nparam, boolean has_output)
{
  PATTERN pattern;
  long index;
  SYMBOL *sym;

  /*
     A QUOI SERT CE TEST ?

     add_operator() peut �re appel�sans op�ateur. Cf.
     if (RES_priority(op) < prio) ...
  */

  if (op == RS_NONE || op == RS_UNARY)
    return;

  if (op == RS_EXCL)
  {
    /*if (!PATTERN_is_identifier(get_last_pattern(1)))
      THROW("The operator '!' must be followed by an identifier.");*/

    sym = TABLE_get_symbol(JOB->class->table, PATTERN_index(get_last_pattern(1)));
    TABLE_add_symbol(JOB->class->string, sym->name, sym->len, NULL, &index);

    change_last_pattern(1, PATTERN_make(RT_STRING, index));
    op = RS_LSQR;
    nparam = 2;

    check_last_first(2);
  }

  pattern = PATTERN_make(RT_RESERVED, op);

  if (op == RS_LBRA && has_output)
    pattern = PATTERN_set_flag(pattern, RT_OUTPUT);

  add_pattern(pattern);

  pattern = PATTERN_make(RT_PARAM, nparam);
  add_pattern(pattern);
}


static void add_operator(short op, short nparam)
{
  add_operator_output(op, nparam, FALSE);
}



static void add_subr(PATTERN subr_pattern, short nparam, boolean has_output)
{
  PATTERN pattern;

  if (has_output)
    subr_pattern = PATTERN_set_flag(subr_pattern, RT_OUTPUT);

  add_pattern(subr_pattern);

  pattern = PATTERN_make(RT_PARAM, nparam);
  add_pattern(pattern);
}


static void analyze_make_array()
{
  int n = 0;

	if (PATTERN_is(*current, RS_RSQR))
	{
		current++;
		add_pattern(PATTERN_make(RT_RESERVED, RS_NULL));
		return;
	}

  for(;;)
  {
    n++;
    analyze_expr(0, RS_NONE);
    if (!PATTERN_is(*current, RS_COMMA))
      break;
    current++;
  }

  if (!PATTERN_is(*current, RS_RSQR))
    THROW("Missing ']'");
  current++;

  add_operator(RS_RSQR, n);
}


static void analyze_single(int op)
{
  PATTERN *pattern;
  bool jump_newline;

  jump_newline = PATTERN_is_newline(*current);
  if (jump_newline)
  {
  	add_pattern(PATTERN_make(RT_NEWLINE, 0));
  	JOB->line++;
    current++;
	}

  if (op == RS_PT || op == RS_EXCL)
    if (!PATTERN_is_identifier(*current))
    {
      if (op == RS_PT)
        THROW("The '.' operator must be followed by an identifier");
      else
        THROW("The '!' operator must be followed by an identifier");
    }

  /* ( expr ) */

  if (PATTERN_is(*current, RS_LBRA))
  {
    long index = ARRAY_count(tree);
    PATTERN last;

    current++;
    analyze_expr(0, RS_NONE);

    if (!PATTERN_is(*current, RS_RBRA))
      THROW("Missing ')'");
    current++;

    if (ARRAY_count(tree) == (index + 1))
    {
      last = get_last_pattern(1);
      if (PATTERN_is_string(last))
        change_last_pattern(1, PATTERN_make(RT_TSTRING, PATTERN_index(last)));
    }
  }

  /* [ expr, expr, ... ] */

  else if (PATTERN_is(*current, RS_LSQR))
  {
    current++;
    analyze_make_array();
  }

  /* - expr | NOT expr */

  else if (PATTERN_is(*current, RS_MINUS) || PATTERN_is(*current, RS_NOT))
  {
    pattern = current;
    current++;

    analyze_expr(RES_priority(RS_NOT), RS_UNARY);
    add_operator(PATTERN_index(*pattern), 1);
  }

  /* . symbol */

  else if (PATTERN_is(*current, RS_PT) && PATTERN_is_identifier(current[1]))
  {
    add_operator(PATTERN_index(current[0]), 0);
    add_pattern(PATTERN_set_flag(current[1], RT_POINT));
    current += 2;
  }

  /* NULL, TRUE, FALSE, ME, PARENT, LAST, ERROR */
  /* nombre, chaine ou symbole */

  else if (PATTERN_is(*current, RS_NULL)
           || PATTERN_is(*current, RS_ME)
           || PATTERN_is(*current, RS_LAST)
           || PATTERN_is(*current, RS_TRUE)
           || PATTERN_is(*current, RS_FALSE)
           || PATTERN_is(*current, RS_ERROR)
           || (!PATTERN_is_reserved(*current) && !PATTERN_is_newline(*current) && !PATTERN_is_end(*current)))
  {
    add_pattern(*current);

    if (PATTERN_is_identifier(*current))
    {
      /*if ((op == RS_NONE || op == RS_UNARY) && (PATTERN_is_identifier(*current)))
        change_last_pattern(1, PATTERN_set_flag(get_last_pattern(1), RT_FIRST));*/
      if (op == RS_PT)
      {
        change_last_pattern(1, PATTERN_set_flag(get_last_pattern(1), RT_POINT));
        check_last_first(2);
      }
    }

    current++;
  }

  else if (PATTERN_is(*current, RS_SUPER))
  {
    add_pattern(*current);
    current++;
    if (!PATTERN_is(*current, RS_PT)
        && !PATTERN_is(*current, RS_EXCL)
        && !PATTERN_is(*current, RS_LBRA)
        && !PATTERN_is(*current, RS_LSQR))
      THROW("SUPER cannot be used alone");
  }

  else
  {
    if (jump_newline)
    {
      current--;
      JOB->line--;
		}

    THROW(E_UNEXPECTED, READ_get_pattern(current));
  }
}


static void analyze_call()
{
  /*static PATTERN *output[MAX_PARAM_OP];*/

  int nparam_post = 0;
  PATTERN subr_pattern = NULL_PATTERN;
  PATTERN last_pattern = get_last_pattern(1);
  boolean has_output = FALSE;
  /*int i;
  PATTERN *save_current;*/
  SUBR_INFO *info;
  boolean optional = TRUE;

  /*
  get_pattern_subr(last_pattern, &subr);
  */
  if (PATTERN_is_subr(last_pattern))
  {
    subr_pattern = last_pattern;
    remove_last_pattern();
    optional = FALSE;
  }
  else if (PATTERN_is_identifier(last_pattern))
  {
    check_last_first(1);
  }
  else if (PATTERN_is_string(last_pattern) || PATTERN_is_number(last_pattern))
    THROW(E_SYNTAX);

  /* N.B. Le cas o last_pattern = "." n'a pas de test sp�ifique */

  for (;;)
  {
    if (PATTERN_is(*current, RS_RBRA))
    {
      current++;
      break;
    }

    if (nparam_post > 0)
    {
      if (!PATTERN_is(*current, RS_COMMA))
        THROW("Missing ')'");
      current++;
    }

    #if 0
    if (FALSE) /*(PATTERN_is(*current, RS_AMP))*/
    {
      current++;
      output[nparam_post] = current;
      has_output = TRUE;
    }
    else
    {
      output[nparam_post] = NULL;
    }
    #endif

    if (optional && (PATTERN_is(*current, RS_COMMA) || PATTERN_is(*current, RS_RBRA)))
    {
      add_reserved_pattern(RS_OPTIONAL);
    }
    else
    {
      analyze_expr(0, RS_NONE);
    }

    nparam_post++;

    if (nparam_post > MAX_PARAM_FUNC)
      THROW("Too many arguments");
  }

  if (get_last_pattern(1) == PATTERN_make(RT_RESERVED, RS_OPTIONAL))
    THROW("Syntax error. Needless arguments");

  /*
  while (nparam_post > 0)
  {
    if (get_last_pattern(1) != PATTERN_make(RT_RESERVED, RS_OPTIONAL))
      break;

    remove_last_pattern();
    nparam_post--;
  }
  */

  if (subr_pattern == NULL_PATTERN)
    add_operator_output(RS_LBRA, nparam_post, has_output);
  else
  {
    info = &COMP_subr_info[PATTERN_index(subr_pattern)];

    if (nparam_post < info->min_param)
      THROW("Not enough arguments to &1()", info->name);
    else if (nparam_post > info->max_param)
      THROW("Too many arguments to &1()", info->name);

    add_subr(subr_pattern, nparam_post, has_output);
  }

  #if 0
  if (has_output)
  {
    save_current = current;

    for (i = nparam_post - 1; i >= 0; i--)
    {
      if (output[i] != NULL)
      {
        current = output[i];
        analyze_expr(0, RS_NONE);
        add_reserved_pattern(RS_AT);
      }
      else
        add_reserved_pattern(RS_COMMA); /* Provoque un drop */
    }

    if (subr_pattern != NULL_PATTERN)
      add_reserved_pattern(RS_RBRA); /* Provoque le PUSH RETURN dans le cas d'un call*/

    current = save_current;
  }
  #endif
}


static void analyze_array()
{
  int i;

  check_last_first(1);

  for(i = 0; i < MAX_ARRAY_DIM; i++)
  {
    analyze_expr(0, RS_NONE);
    if (!PATTERN_is(*current, RS_COMMA))
      break;
    current++;
  }

  if (!PATTERN_is(*current, RS_RSQR))
    THROW("Missing ']'");
  current++;

  add_operator(RS_LSQR, i + 2);
}


#if 0
static void analyze_expr_check_first(int op_curr)
{
  /* On laisse le marqueur RT_FIRST que si on a affaire �l'op�ateur '.' */
  /*
  last = get_last_pattern();
  if (PATTERN_is_first(last))
  {
    if (op_curr != RS_PT)
      change_last_pattern(PATTERN_unset_flag(last, RT_FIRST));
  }
  */
}
#endif

static void analyze_expr(short priority, short op_main)
{
  int op_curr;
  int op;
  int prio;
  int nparam;

  inc_level();

  op_curr = op_main;
  nparam = (op_main == RS_NONE || op_main == RS_UNARY) ? 0 : 1;

  /* cas particulier de NEW */
  /* obsol�e : ne doit jamais servir, l'analyse est faite ailleurs */

  if (PATTERN_is(*current, RS_NEW))
    THROW("Cannot use NEW operator there");

  /* analyse des op�andes */

READ_OPERAND:

  //analyze_expr_check_first(op_curr);

  analyze_single(op_curr);
  nparam++;

  if (nparam > MAX_PARAM_OP)
    THROW("Expression too complex. Too many operands");

  /* Lecture de l'op�ateur */

READ_OPERATOR:

  if (!PATTERN_is_reserved(*current))
    goto OPERATOR_END;

  op = PATTERN_index(*current);

  if (!RES_is_operator(op))
    goto OPERATOR_END;

  if (op == RS_AND || op == RS_OR)
    if (PATTERN_is(current[1], RS_IF))
      goto OPERATOR_END;

  current++;

  if (priority)
    prio = priority;
  else if (op_curr == RS_NONE)
    prio = 0;
  else
    prio = RES_priority(op_curr);

  if (op_curr == RS_NONE)
  {
    if (RES_is_binary(op) || RES_is_n_ary(op))
    {
      op_curr = op;
      goto READ_OPERAND;
    }
  }

  if (op_curr == op)
  {
    if (!(RES_is_binary(op) && nparam == 2))
      goto READ_OPERAND;
  }

  if (RES_priority(op) > prio)
  {
    if (op == RS_LSQR)
      analyze_array();
    else if (op == RS_LBRA)
      analyze_call();
    else
      analyze_expr(RES_priority(op), op);

    goto READ_OPERATOR;
  }

  if (RES_priority(op) == prio)
  {
    add_operator(op_curr, nparam);

    if (op == RS_LSQR)
    {
      analyze_array();
      goto READ_OPERATOR;
    }
    else if (op == RS_LBRA)
    {
      analyze_call();
      goto READ_OPERATOR;
    }
    else
    {
      if (RES_is_only(op_curr) || RES_is_only(op))
        THROW("Ambiguous expression. Please use braces");

      nparam = 1;
      op_curr = op;
      goto READ_OPERAND;
    }
  }

  if (RES_priority(op) < prio)
  {
    if ((op_main != RS_NONE) || (priority > 0))
    {
      add_operator(op_curr, nparam);
      current--;
      goto END;
    }

    add_operator(op_curr, nparam);

    if (op == RS_LSQR)
    {
      analyze_array();
      nparam = 1;
      op_curr = op_main;
      goto READ_OPERATOR;
    }
    else if (op == RS_LBRA)
    {
      analyze_call();
      nparam = 1;
      op_curr = op_main;
      goto READ_OPERATOR;
    }
    else
    {
      nparam = 1;
      op_curr = op;
      goto READ_OPERAND;
    }
  }

  dec_level();
  return;

OPERATOR_END:

  //analyze_expr_check_first(op_curr);

  add_operator(op_curr, nparam);

END:

  dec_level();
  return;

}


PUBLIC TRANS_TREE *TRANS_tree(void)
{
  #ifdef DEBUG
  int i;
  #endif

  /*last_pattern = NULL_PATTERN;*/

  ARRAY_create(&tree);
  /*ARRAY_add(&tree);*/

  current = JOB->current;

  TRY
  {
    analyze_expr(0, RS_NONE);
    JOB->current = current;
  }
  CATCH
  {
    JOB->current = current;
    PROPAGATE();
  }
  END_TRY

  #ifdef DEBUG
    printf("\n");
    for (i = 0; i < ARRAY_count(tree); i++)
    {
      printf("[%d] ", i);
      READ_dump_pattern(&tree[i]);
    }
  #endif

  return tree;
}



PUBLIC boolean TRANS_is_statement(TRANS_TREE *_tree)
{
  PATTERN last;
  long count;

  tree = _tree;

  count = ARRAY_count(tree);

  if (count == 0)
    return FALSE;

  count--;
  last = tree[count];

  if (PATTERN_is_param(last) && (count > 0))
  {
    count--;
    last = tree[count];
  }

  if (PATTERN_is(last, RS_PT))
    goto _ADD_BRACE;

  if (PATTERN_is_identifier(last))
  {
    if (count == 0)
      goto _ADD_BRACE;

    if (count >= 2)
    {
      last = tree[count - 1];
      if (PATTERN_is_param(last) && (PATTERN_index(last) == 0)
          && PATTERN_is(tree[count - 2], RS_PT))
        goto _ADD_BRACE;
    }
  }
  else if (PATTERN_is_subr(last)
           || PATTERN_is(last, RS_LBRA)
           || PATTERN_is(last, RS_RBRA)
           || PATTERN_is(last, RS_AT)
           || PATTERN_is(last, RS_COMMA))
    return TRUE;

  #ifdef DEBUG
    printf("Last = ");
    READ_dump_pattern(&last);
  #endif

  return FALSE;

_ADD_BRACE:

  add_operator(RS_LBRA, 0);
  #ifdef DEBUG
  printf("Add ()\n");
  #endif
  return TRUE;
}
