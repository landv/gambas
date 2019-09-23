/***************************************************************************

  gbc_trans_tree.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

static short _level;
static PATTERN *_current;
static TRANS_TREE _tree[MAX_EXPR_PATTERN];
static int _tree_pos[MAX_EXPR_PATTERN];
static int _tree_length = 0;
static int _tree_line = 0;

static void analyze_expr(short priority, short op_main);
static void analyze_array();


static void THROW_EXPR_TOO_COMPLEX()
{
	THROW("Expression too complex");
}

static void inc_level()
{
	_level++;
	if (_level > MAX_EXPR_LEVEL)
		THROW_EXPR_TOO_COMPLEX();
}


static void dec_level()
{
	_level--;
}

/*#define add_pattern(_pattern) \
do { \
	if (_tree_length >= MAX_EXPR_PATTERN) \
		THROW_EXPR_TOO_COMPLEX(); \
	_tree_pos[_tree_length] = COMPILE_get_column(_current); \
	_tree[_tree_length] = (_pattern); \
	_tree_length++; \
} while (0)*/

static inline void add_pattern(PATTERN pattern)
{
	if (_tree_length >= MAX_EXPR_PATTERN)
		THROW_EXPR_TOO_COMPLEX();
	_tree_pos[_tree_length] = COMPILE_get_column(_current);
	if (JOB->line != _tree_line)
	{
		_tree_line = JOB->line;
		_tree_pos[_tree_length] = -_tree_pos[_tree_length];
	}
	_tree[_tree_length] = pattern;
	_tree_length++;
}


static void remove_last_pattern()
{
	if (_tree_length == 0)
		return;
		
	_tree_length--;
}


static PATTERN get_last_pattern(int dep)
{
	if (_tree_length < dep)
		return NULL_PATTERN;
	else
		return _tree[_tree_length - dep];
}


static void change_last_pattern(int dep, PATTERN pattern)
{
	if (_tree_length < dep)
		return;
	_tree[_tree_length - dep] = pattern;
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

#define add_operator(_op, _nparam) add_operator_output(_op, _nparam, 0)


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

	count = _tree_length;

	if (count == 0)
		return FALSE;

	count--;
	last = _tree[count];

	while (PATTERN_is_param(last) && (count > 0))
	{
		count--;
		last = _tree[count];
	}

	if (PATTERN_is(last, RS_PT))
		goto _ADD_BRACE;

	if (PATTERN_is_identifier(last))
	{
		if (count == 0)
			goto _ADD_BRACE;

		if (count >= 2)
		{
			last = _tree[count - 1];
			if (PATTERN_is_param(last) && (PATTERN_index(last) == 0)
					&& PATTERN_is(_tree[count - 2], RS_PT))
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

	/*if (PATTERN_is(*_current, RS_RSQR))
	{
		_current++;
		add_pattern(PATTERN_make(RT_RESERVED, RS_NULL));
		return;
	}*/

	if (!PATTERN_is(*_current, RS_RSQR))
	{
		for(;;)
		{
			n++;
			/*if (n > MAX_PARAM_OP)
				THROW("Too many arguments");*/
			analyze_expr(0, RS_NONE);
			
			if (!checked)
			{
				collection = PATTERN_is(*_current, RS_COLON);
				checked = TRUE;
			}
			
			if (collection)
			{
				if (!PATTERN_is(*_current, RS_COLON))
					THROW(E_MISSING, "':'");
				_current++;
				n++;
				/*if (n > MAX_PARAM_OP)
					THROW("Too many arguments");*/
				analyze_expr(0, RS_NONE);
			}
			
			if (!PATTERN_is(*_current, RS_COMMA))
				break;

			_current++;

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

	while (PATTERN_is_newline(*_current))
	{
		add_pattern(PATTERN_make(RT_NEWLINE, 0));
		JOB->line++;
		_current++;
	}
		
	if (!PATTERN_is(*_current, RS_RSQR))
		THROW(E_MISSING, "']'");
	_current++;

	add_operator(collection ? RS_COLON : RS_RSQR, n);
}


static void analyze_single(int op)
{
	PATTERN *pattern;
	bool jump_newline;

	jump_newline = PATTERN_is_newline(*_current);
	if (jump_newline)
	{
		add_pattern(PATTERN_make(RT_NEWLINE, 0));
		JOB->line++;
		_current++;
	}

	if (op == RS_PT && !PATTERN_is_identifier(*_current))
		THROW("The '.' operator must be followed by an identifier");
	else if (op == RS_EXCL && !PATTERN_is_string(*_current))
		THROW("The '!' operator must be followed by an identifier");

	/* ( expr ) */

	if (PATTERN_is(*_current, RS_LBRA))
	{
		int old_length = _tree_length;
		PATTERN last;

		_current++;
		analyze_expr(0, RS_NONE);

		if (!PATTERN_is(*_current, RS_RBRA))
			THROW(E_MISSING, "')'");
		_current++;

		if (_tree_length == (old_length + 1))
		{
			last = get_last_pattern(1);
			if (PATTERN_is_string(last))
				change_last_pattern(1, PATTERN_make(RT_TSTRING, PATTERN_index(last)));
		}
	}

	/* [ expr, expr, ... ] */

	else if (PATTERN_is(*_current, RS_LSQR))
	{
		_current++;
		analyze_make_array();
	}

	/* - expr | NOT expr */

	else if (PATTERN_is(*_current, RS_MINUS) || PATTERN_is(*_current, RS_NOT))
	{
		pattern = _current;
		_current++;

		analyze_expr(RES_priority(RS_NOT), RS_UNARY);
		add_operator(PATTERN_index(*pattern), 1);
	}

	// . symbol

	else if (PATTERN_is(*_current, RS_PT) && PATTERN_is_identifier(_current[1]))
	{
		add_operator(PATTERN_index(_current[0]), 0);
		add_pattern(PATTERN_set_flag(_current[1], RT_POINT));
		add_operator(PATTERN_index(_current[0]), 2);
		_current += 2;
	}

	// . [ ... ]

	else if (PATTERN_is(*_current, RS_PT) && PATTERN_is(_current[1], RS_LSQR))
	{
		add_operator(PATTERN_index(_current[0]), 0);
		//add_pattern(PATTERN_set_flag(RS_RSQR, RT_POINT));
		_current += 2;
		analyze_array();
	}

	// ! symbol

	else if (PATTERN_is(*_current, RS_EXCL) && PATTERN_is_string(_current[1]))
	{
		add_operator(RS_PT, 0);
		add_pattern(PATTERN_set_flag(_current[1], RT_POINT));
		add_operator(RS_EXCL, 0);
		_current += 2;
	}

	/* NULL, TRUE, FALSE, ME, PARENT, LAST, ERROR */
	/* number, string or symbol */

	else if (PATTERN_is(*_current, RS_NULL)
					|| PATTERN_is(*_current, RS_ME)
					|| PATTERN_is(*_current, RS_LAST)
					|| PATTERN_is(*_current, RS_TRUE)
					|| PATTERN_is(*_current, RS_FALSE)
					|| PATTERN_is(*_current, RS_PINF)
					|| PATTERN_is(*_current, RS_MINF)
					|| PATTERN_is(*_current, RS_ERROR)
					|| (!PATTERN_is_reserved(*_current) && !PATTERN_is_newline(*_current) && !PATTERN_is_end(*_current)))
	{
		add_pattern(*_current);

		if (PATTERN_is_identifier(*_current))
		{
			/*if ((op == RS_NONE || op == RS_UNARY) && (PATTERN_is_identifier(*_current)))
				change_last_pattern(1, PATTERN_set_flag(get_last_pattern(1), RT_FIRST));*/
			if (op == RS_PT)
			{
				change_last_pattern(1, PATTERN_set_flag(get_last_pattern(1), RT_POINT));
				check_last_first(2);
			}
		}

		_current++;
	}

	else if (PATTERN_is(*_current, RS_SUPER))
	{
		add_pattern(*_current);
		_current++;
		if (!PATTERN_is(*_current, RS_PT)
				&& !PATTERN_is(*_current, RS_EXCL)
				&& !PATTERN_is(*_current, RS_LBRA)
				&& !PATTERN_is(*_current, RS_LSQR))
			THROW("SUPER cannot be used alone");
	}

	else
	{
		if (jump_newline)
		{
			_current--;
			JOB->line--;
		}

		THROW_UNEXPECTED(_current);
	}
}


static void analyze_call()
{
	static PATTERN *byref_pattern[MAX_PARAM_FUNC];

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

	if (PATTERN_type(subr_pattern) == RT_SUBR && PATTERN_index(subr_pattern) == SUBR_VarPtr)
	{
		if (!PATTERN_is_identifier(_current[0]) || !PATTERN_is(_current[1], RS_RBRA))
			THROW("Syntax error. VarPtr() takes only one identifier");
		
		add_pattern(*_current);
		_current += 2;
		add_subr(subr_pattern, 1);
	}
	else
	{
		for (;;)
		{
			if (PATTERN_is(*_current, RS_RBRA))
			{
				_current++;
				break;
			}
	
			if (nparam_post > 0)
			{
				if (!PATTERN_is(*_current, RS_COMMA))
					THROW(E_MISSING, "',' or ')'");
				_current++;
			}
	
			#if 0
			if (FALSE) /*(PATTERN_is(*_current, RS_AMP))*/
			{
				_current++;
				output[nparam_post] = _current;
				has_output = TRUE;
			}
			else
			{
				output[nparam_post] = NULL;
			}
			#endif
			
			if (optional && (PATTERN_is(*_current, RS_COMMA) || PATTERN_is(*_current, RS_RBRA)))
			{
				add_reserved_pattern(RS_OPTIONAL);
			}
			else if (optional && PATTERN_is(*_current, RS_3PTS) && PATTERN_is(_current[1], RS_RBRA))
			{
				_current++;
				add_reserved_pattern(RS_3PTS);
				nparam_post--;
			}
			else
			{
				if (PATTERN_is(*_current, RS_AT) || PATTERN_is(*_current, RS_BYREF))
				{
					_current++;
					BYREF_SET(byref, nparam_post);
					byref_pattern[nparam_post] = _current;
				}
	
				analyze_expr(0, RS_NONE);
			}
	
			nparam_post++;

			if (nparam_post >= MAX_PARAM_FUNC)
				THROW("Too many arguments");
		}

		last_pattern = get_last_pattern(1);
		if (PATTERN_is(last_pattern, RS_OPTIONAL))
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
	
		if (PATTERN_is_null(subr_pattern))
		{
			add_operator_output(RS_LBRA, nparam_post, byref);
			
			save = _current;
			
			for (i = nparam_post - 1; i >= 0; i--)
			{
				if (BYREF_TEST(byref, i))
				{
					_current = byref_pattern[i];
					analyze_expr(0, RS_NONE);
					//if (!is_statement())
					//	THROW("The &1 argument cannot be passed by reference", TRANS_get_num_desc(i + 1));
					add_pattern(PATTERN_make(RT_RESERVED, RS_AT));
				}
			}
			
			_current = save;
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
		
		if (!PATTERN_is(*_current, RS_COMMA))
			break;
			
		_current++;
	}

	if (!PATTERN_is(*_current, RS_RSQR))
		THROW(E_MISSING, "',' or ')'");
	_current++;

	add_operator(RS_LSQR, i + 2);
}


static void analyze_expr(short priority, short op_main)
{
	short op, op_curr, op_not;
	short prio;
	short nparam;

	inc_level();

	op_curr = op_main;
	op_not = RS_NONE;
	nparam = (op_main == RS_NONE || op_main == RS_UNARY) ? 0 : 1;

	if (PATTERN_is(*_current, RS_NEW))
		THROW("Cannot use NEW operator there");

READ_OPERAND:

	//analyze_expr_check_first(op_curr);

	analyze_single(op_curr);
	nparam++;

	if (nparam > MAX_PARAM_OP)
		THROW("Expression too complex. Too many operands");

READ_OPERATOR:

	if (!PATTERN_is_reserved(*_current))
		goto OPERATOR_END;

	op = PATTERN_index(*_current);

	if (!RES_is_operator(op))
		goto OPERATOR_END;

	if (op == RS_AND || op == RS_OR)
		if (PATTERN_is(_current[1], RS_IF))
			goto OPERATOR_END;

	_current++;
	
	if (op == RS_NOT && PATTERN_is_reserved(*_current))
	{
		op_not = PATTERN_index(*_current);
		if (RES_is_operator(op_not) && RES_can_have_not_before(op_not))
		{
			op = op_not + 1;
			_current++;
		}
	}
	
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
			_current--;
			if (op_not != RS_NONE)
				_current--;
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

	add_operator(op_curr, nparam);

END:

	dec_level();
	return;

}


void TRANS_tree(bool check_statement, TRANS_TREE **result, int *count, int **result_pos)
{
	#ifdef DEBUG
	int i;
	#endif

	_tree_length = 0;
	_tree_line = JOB->line;
	_current = JOB->current;
	_level = 0;

	TRY
	{
		analyze_expr(0, RS_NONE);
		JOB->current = _current;
	}
	CATCH
	{
		JOB->current = _current;
		PROPAGATE();
	}
	END_TRY

	#ifdef DEBUG
		printf("\n");
		for (i = 0; i < _tree_length; i++)
		{
			printf("[% 4d] ", i);
			READ_dump_pattern(&_tree[i]);
		}
	#endif

	if (check_statement && (!is_statement()))
		THROW("This expression cannot be a statement");

	if (result)
	{
		add_pattern(NULL_PATTERN);
		ALLOC(result, sizeof(PATTERN) * _tree_length);
		memcpy(*result, _tree, sizeof(PATTERN) * _tree_length);
		if (result_pos)
			*result_pos = _tree_pos;
		*count = _tree_length - 1;
	}
}

