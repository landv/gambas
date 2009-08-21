/***************************************************************************

  gbc_trans_tree.c

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

#define __GBC_TRANS_TREE_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_compile.h"

#include "gbc_trans.h"
#include "gb_code.h"
#include "gbc_read.h"

//#define DEBUG

#define BYREF_TEST(_byref, _n) (_byref & ((uintptr_t)1 << _n))
#define BYREF_SET(_byref, _n) _byref |= ((uintptr_t)1 << _n)

static short level;
static PATTERN *current;
static TRANS_TREE tree[MAX_EXPR_PATTERN];
static int tree_length = 0;
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
  if (tree_length >= MAX_EXPR_PATTERN)
    THROW("Expression too complex");

  tree[tree_length] = pattern;  
  tree_length++;
}


static void remove_last_pattern()
{
  if (tree_length == 0)
    return;
    
  tree_length--;
}


static PATTERN get_last_pattern(int dep)
{
  /*
  short index;

  index = (short)ARRAY_count(tree);
  if (index < dep)
    return NULL_PATTERN;

  return tree[index - dep];
  */
  if (tree_length < dep)
    return NULL_PATTERN;
  else
    return tree[tree_length - dep];
}


static void change_last_pattern(int dep, PATTERN pattern)
{
  /*
  short index;

  index = (short)ARRAY_count(tree);
  if (index < dep)
    return;
  
  tree[index - dep] = pattern;
  */
  if (tree_length < dep)
    return;
  tree[tree_length - dep] = pattern;
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


static void add_operator_output(short op, short nparam, uint64_t byref)
{
  PATTERN pattern;

  /*
     Why this test?

     add_operator() can be called without operator. See:
     if (RES_priority(op) < prio) ...
  */

  if (op == RS_NONE || op == RS_UNARY)
    return;

  if (op == RS_EXCL)
  {
    op = RS_LSQR;
    nparam = 2;

    check_last_first(2);
  }

  pattern = PATTERN_make(RT_RESERVED, op);

  //if (op == RS_LBRA && byref)
  //  pattern = PATTERN_set_flag(pattern, RT_OUTPUT);

  add_pattern(pattern);

  add_pattern(PATTERN_make(RT_PARAM, nparam));

  if (op == RS_LBRA && byref)
  {
  	while (byref)
  	{
	  	add_pattern(PATTERN_make(RT_PARAM, byref & 0xFFFF));
  		byref >>= 16;
  	}
  }
}


static void add_operator(short op, short nparam)
{
  add_operator_output(op, nparam, 0);
}



static void add_subr(PATTERN subr_pattern, short nparam)
{
  PATTERN pattern;

  //if (has_output)
  //  subr_pattern = PATTERN_set_flag(subr_pattern, RT_OUTPUT);

  add_pattern(subr_pattern);

  pattern = PATTERN_make(RT_PARAM, nparam);
  add_pattern(pattern);
}

static bool is_statement(void)
{
  PATTERN last;
  int count;

  count = tree_length;

  if (count == 0)
    return FALSE;

  count--;
  last = tree[count];

  while (PATTERN_is_param(last) && (count > 0))
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
    printf("%08X %08X %d\n", last, PATTERN_make(RT_RESERVED, RS_LBRA), PATTERN_is(last, RS_LBRA));
  #endif

  return FALSE;

_ADD_BRACE:

  add_operator(RS_LBRA, 0);
  #ifdef DEBUG
  printf("Add ()\n");
  #endif
  return TRUE;
}

static void analyze_make_array()
{
  int n = 0;
  bool checked = FALSE;
  bool collection = FALSE;

	if (PATTERN_is(*current, RS_RSQR))
	{
		current++;
		add_pattern(PATTERN_make(RT_RESERVED, RS_NULL));
		return;
	}

  for(;;)
  {
    n++;
    if (n > MAX_PARAM_OP)
    	THROW("Too many arguments");
    analyze_expr(0, RS_NONE);
    
    if (!checked)
    {
    	collection = PATTERN_is(*current, RS_COLON);
    	checked = TRUE;
    }
    
    if (collection)
    {
    	if (!PATTERN_is(*current, RS_COLON))
    		THROW("Missing ':'");
			current++;
			n++;
			if (n > MAX_PARAM_OP)
				THROW("Too many arguments");
	    analyze_expr(0, RS_NONE);
    }
    
    if (!PATTERN_is(*current, RS_COMMA))
      break;
    current++;
  }

  if (!PATTERN_is(*current, RS_RSQR))
    THROW("Missing ']'");
  current++;

  add_operator(collection ? RS_COLON : RS_RSQR, n);
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

  if (op == RS_PT && !PATTERN_is_identifier(*current))
		THROW("The '.' operator must be followed by an identifier");
	else if (op == RS_EXCL && !PATTERN_is_string(*current))
		THROW("The '!' operator must be followed by an identifier");

  /* ( expr ) */

  if (PATTERN_is(*current, RS_LBRA))
  {
    int old_length = tree_length;
    PATTERN last;

    current++;
    analyze_expr(0, RS_NONE);

    if (!PATTERN_is(*current, RS_RBRA))
      THROW("Missing ')'");
    current++;

    if (tree_length == (old_length + 1))
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
  /* number, string or symbol */

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
  static PATTERN *byref_pattern[MAX_PARAM_OP];

  int i, nparam_post = 0;
  PATTERN subr_pattern = NULL_PATTERN;
  PATTERN last_pattern = get_last_pattern(1);
  SUBR_INFO *info;
  bool optional = TRUE;
  uint64_t byref = 0;
  PATTERN *save;

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

  /* N.B. Le cas où last_pattern = "." n'a pas de test spécifique */

	if (subr_pattern && subr_pattern == PATTERN_make(RT_SUBR, SUBR_VarPtr))
	{
		if (!PATTERN_is_identifier(current[0]) || !!PATTERN_is(current[1], RS_RBRA))
			THROW("Syntax error. VarPtr() takes only one identifier");
		
		add_pattern(*current);
		current += 2;
		add_subr(subr_pattern, 1);
	}
	else
	{
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
					THROW("Missing ',' or ')'");
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
				if (PATTERN_is(*current, RS_AT) || PATTERN_is(*current, RS_BYREF))
				{
					current++;
					BYREF_SET(byref, nparam_post);
					byref_pattern[nparam_post] = current;
				}
	
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
		{
			add_operator_output(RS_LBRA, nparam_post, byref);
			
			save = current;
			
			for (i = nparam_post - 1; i >= 0; i--)
			{
				if (BYREF_TEST(byref, i))
				{
					current = byref_pattern[i];
					analyze_expr(0, RS_NONE);
					//if (!is_statement())
					//	THROW("The &1 argument cannot be passed by reference", TRANS_get_num_desc(i + 1));
					add_pattern(PATTERN_make(RT_RESERVED, RS_AT));
				}
			}
			
			current = save;
		}
		else
		{
			info = &COMP_subr_info[PATTERN_index(subr_pattern)];
	
			if (nparam_post < info->min_param)
				THROW("Not enough arguments to &1()", info->name);
			else if (nparam_post > info->max_param)
				THROW("Too many arguments to &1()", info->name);
			else if (byref)
				THROW("Subroutine arguments cannot be passed by reference");
	
			add_subr(subr_pattern, nparam_post);
		}
	}

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
    THROW("Missing comma or ']'");
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
        THROW("Ambiguous expression. Please use brackets");

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


TRANS_TREE *TRANS_tree(bool check_statement)
{
  TRANS_TREE *copy;
  #ifdef DEBUG
  int i;
  #endif

  /*last_pattern = NULL_PATTERN;*/

  //ARRAY_create(&tree);
  tree_length = 0;

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
    for (i = 0; i < tree_length; i++)
    {
      printf("[%d] ", i);
      READ_dump_pattern(&tree[i]);
    }
  #endif

  if (check_statement && (!is_statement()))
    THROW("This expression cannot be a statement");

  ARRAY_create(&copy);
  ARRAY_add_many(&copy, tree_length);
  memcpy(copy, tree, sizeof(PATTERN) * tree_length);

  return copy;
}

