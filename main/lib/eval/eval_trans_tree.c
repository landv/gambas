/***************************************************************************

  eval_trans_tree.c

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

#define __TRANS_TREE_C

#include "gb_common.h"
#include "gb_error.h"
#include "gb_reserved.h"

#include "eval_trans.h"
#include "eval_read.h"
#include "eval.h"

/*#define DEBUG*/

static short level;
static PATTERN *current;
/*PRIVATE PATTERN last_pattern;*/

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
	PATTERN *node;
	short index;

	index = (short)ARRAY_count(EVAL->tree);

	if (index >= MAX_EXPR_PATTERN)
		THROW("Expression too complex");

	node = ARRAY_add(&EVAL->tree);
	*node = pattern;
	/*last_pattern = pattern;*/


	/*#ifdef DEBUG
	READ_dump_pattern(&pattern);
	#endif*/
}


static void remove_last_pattern()
{
	if (!ARRAY_count(EVAL->tree))
		return;

	ARRAY_remove_last(&EVAL->tree);
}


static PATTERN get_last_pattern(int dep)
{
	short index;

	index = (short)ARRAY_count(EVAL->tree);
	if (index < dep)
		return NULL_PATTERN;

	return EVAL->tree[index - dep];
}


static void change_last_pattern(int dep, PATTERN pattern)
{
	short index;

	index = (short)ARRAY_count(EVAL->tree);
	if (index < dep)
		return;

	EVAL->tree[index - dep] = pattern;
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



static void add_operator_output(short op, short nparam)
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

	/*if (op == RS_LBRA && has_output)
		pattern = PATTERN_set_flag(pattern, RT_OUTPUT);*/

	add_pattern(pattern);

	pattern = PATTERN_make(RT_PARAM, nparam);
	add_pattern(pattern);
}


static void add_operator(short op, short nparam)
{
	add_operator_output(op, nparam);
}



static void add_subr(PATTERN subr_pattern, short nparam)
{
	PATTERN pattern;

	/*if (has_output)
		subr_pattern = PATTERN_set_flag(subr_pattern, RT_OUTPUT);*/

	add_pattern(subr_pattern);

	pattern = PATTERN_make(RT_PARAM, nparam);
	add_pattern(pattern);
}


static void analyze_make_array()
{
	int n = 0;
	bool checked = FALSE;
	bool collection = FALSE;

	if (!PATTERN_is(*current, RS_RSQR))
	{
		for(;;)
		{
			n++;
			/*if (n > MAX_PARAM_OP)
				THROW("Too many arguments");*/
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
				/*if (n > MAX_PARAM_OP)
					THROW("Too many arguments");*/
				analyze_expr(0, RS_NONE);
			}
			
			if (!PATTERN_is(*current, RS_COMMA))
				break;
			current++;

			if (collection)
			{
				if (n == (MAX_PARAM_OP - 1))
				{
					add_operator(RS_COLON, MAX_PARAM_OP + 1);
					n = 0;
				}
			}
			else
			{
				if (n == MAX_PARAM_OP)
				{
					add_operator(RS_RSQR, MAX_PARAM_OP + 1);
					n = 0;
				}
			}
		}
	}

	if (!PATTERN_is(*current, RS_RSQR))
		THROW("Missing ']'");
	current++;

	add_operator(collection ? RS_COLON : RS_RSQR, n);
}



static void analyze_single(int op)
{
	PATTERN *pattern;

	if (PATTERN_is_newline(*current))
		current++;

	if (op == RS_PT && !PATTERN_is_identifier(*current))
		THROW("The '.' operator must be followed by an identifier");
	else if (op == RS_EXCL && !PATTERN_is_string(*current))
		THROW("The '!' operator must be followed by an identifier");

	/* ( expr ) */

	if (PATTERN_is(*current, RS_LBRA))
	{
		int index = ARRAY_count(EVAL->tree);
		PATTERN last;

		current++;
		analyze_expr(0, RS_NONE);

		if (!PATTERN_is(*current, RS_RBRA))
			THROW("Missing ')'");
		current++;

		if (ARRAY_count(EVAL->tree) == (index + 1))
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

	/* . symbol

	else if (PATTERN_is(*current, RS_PT) && PATTERN_is_identifier(current[1]))
	{
		add_operator(PATTERN_index(current[0]), 0);
		add_pattern(PATTERN_set_flag(current[1], RT_POINT));
		current += 2;
	}*/

	/* NULL, TRUE, FALSE, ME, LAST */
	/* nombre, chaine ou symbole */

	else if (PATTERN_is(*current, RS_NULL)
					|| PATTERN_is(*current, RS_ME)
					|| PATTERN_is(*current, RS_LAST)
					|| PATTERN_is(*current, RS_TRUE)
					|| PATTERN_is(*current, RS_FALSE)
					|| PATTERN_is(*current, RS_PINF)
					|| PATTERN_is(*current, RS_MINF)
					//|| PATTERN_is(*current, RS_ERROR)
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

	else
		THROW2("Unexpected &1", READ_get_pattern(current));
}


static void analyze_call()
{
	int nparam_post = 0;
	PATTERN subr_pattern = NULL_PATTERN;
	PATTERN last_pattern = get_last_pattern(1);
	SUBR_INFO *info;
	bool optional = TRUE;

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
		THROW("VarPtr() cannot be used with Eval()");
		
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
		add_operator_output(RS_LBRA, nparam_post);
	else
	{
		info = &COMP_subr_info[PATTERN_index(subr_pattern)];

		if (nparam_post < info->min_param)
			THROW2("Not enough arguments to &1", info->name);
		else if (nparam_post > info->max_param)
			THROW2("Too many arguments to &1", info->name);

		add_subr(subr_pattern, nparam_post);
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
	int op, op_curr, op_not;
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

	current++;

	if (op == RS_NOT && PATTERN_is_reserved(*current))
	{
		op_not = PATTERN_index(*current);
		if (RES_is_operator(op_not) && RES_can_have_not_before(op_not))
		{
			op = op_not + 1;
			current++;
		}
	}

	/*if ((op == RS_BEGINS || op == RS_ENDS) && PATTERN_is(*current, RS_WITH))
		current++;*/

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


PUBLIC void TRANS_tree()
{
	#ifdef DEBUG
	int i;
	#endif

	/*last_pattern = NULL_PATTERN;*/

	ARRAY_create(&EVAL->tree);
	/*ARRAY_add(&tree);*/

	current = EVAL->current; //EVAL->pattern;
	level = 0;

	if (PATTERN_is_newline(*current) || PATTERN_is_end(*current))
		THROW(E_SYNTAX);
	
	analyze_expr(0, RS_NONE);

	while (PATTERN_is_newline(*current))
		current++;
	
	EVAL->current = current;

	#ifdef DEBUG
		printf("\n");
		for (i = 0; i < ARRAY_count(EVAL->tree); i++)
			READ_dump_pattern(&EVAL->tree[i]);
	#endif
}


#if 0
PUBLIC bool TRANS_is_statement(TRANS_TREE *tree)
{
	PATTERN last;
	int count;

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

	if (PATTERN_is(last, RS_PT) || (PATTERN_is_identifier(last) && count == 0))
	{
		add_operator(RS_LBRA, 0);
		return TRUE;
	}

	if (PATTERN_is_subr(last)
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

}
#endif

