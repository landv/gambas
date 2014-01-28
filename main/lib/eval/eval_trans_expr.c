/***************************************************************************

	eval_trans_expr.c

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
static int subr_collection_index = -1;

static void find_subr(int *index, const char *name)
{
	if (*index < 0)
		*index = RESERVED_find_subr(name, strlen(name));
}

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
	//SYMBOL *sym;

	if (TRANS_get_number(index, &number))
		THROW(E_SYNTAX);

	if (number.type == T_INTEGER)
	{
		CODE_push_number(number.ival);
	}
	else
	{
		cst.type = number.type;
		if (cst.type == T_FLOAT)
			cst._float.value = number.dval;
		else if (cst.type == T_LONG)
			cst._long.value = number.lval;

		CODE_push_const(EVAL_add_constant(&cst));
	}
	
	if (number.complex)
		CODE_push_complex();
}


static void push_string(int index, bool trans)
{
	CLASS_CONST cst;
	SYMBOL *sym;
	int len;

	if (index == VOID_STRING)
		len = 0;
	else
	{
		sym = TABLE_get_symbol(EVAL->string, index);
		len = sym->len;
	}

	if (len == 0)
	{
		CODE_push_void_string();
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
static void push_class(int index)
{
	TRANS_DECL decl;

	decl.type = TYPE_make(T_STRING, 0, 0);
	decl.index = NO_SYMBOL;
	decl.value = index;
	CODE_push_class(CLASS_add_constant(EVAL->class, &decl));
}
*/


static void trans_class(int index)
{
	SYMBOL *sym = TABLE_get_symbol(EVAL->table, index);

	if (GB.ExistClassLocal(sym->name))
		CODE_push_class(EVAL_add_class(sym->name));
	else
	{
		THROW("Unknown class");
	}
}


static void trans_identifier(int index, bool first, bool point)
{
	SYMBOL *sym = TABLE_get_symbol(EVAL->table, index);

	/* z�o terminal */

	sym->name[sym->len] = 0;

	if (point)
	{
		CODE_push_unknown(EVAL_add_unknown(sym->name));
	}
	else if (first && GB.ExistClassLocal(sym->name))
	{
		//printf("%.*s %s\n", sym->symbol.len, sym->symbol.name, isupper(*sym->symbol.name) ? "U" : "l");
		CODE_push_class(EVAL_add_class(sym->name));
	}
	else
	{
		CODE_push_local(EVAL_add_variable(index));
	}
}


static void trans_subr(int subr, short nparam)
{
	SUBR_INFO *info = &COMP_subr_info[subr];

	//fprintf(stderr, "trans_subr: %d: %s: %d %d\n", subr, info->name, info->min_param, info->max_param);

	if (nparam < info->min_param)
		THROW2("Not enough arguments to &1()", info->name);
	else if (nparam > info->max_param)
		THROW2("Too many arguments to &1()", info->name);

	CODE_subr(info->opcode, nparam, info->optype, info->max_param == info->min_param);
}


void TRANS_operation(short op, short nparam, PATTERN previous)
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
			find_subr(&subr_array_index, ".Array");
			if (nparam > MAX_PARAM_OP)
				CODE_subr(COMP_subr_info[subr_array_index].opcode, MAX_PARAM_OP + 1, CODE_CALL_VARIANT + MAX_PARAM_OP, FALSE);
			else
				trans_subr(subr_array_index, nparam);
			break;

		case OP_COLON:
			find_subr(&subr_collection_index, ".Collection");
			if (nparam > MAX_PARAM_OP)
				CODE_subr(COMP_subr_info[subr_collection_index].opcode, MAX_PARAM_OP, CODE_CALL_VARIANT + MAX_PARAM_OP - 1, FALSE);
			else
				trans_subr(subr_collection_index, nparam);
			break;

		case OP_LBRA:
			CODE_call(nparam);
			break;

		case OP_MINUS:
			if (nparam == 1)
				CODE_op(C_NEG, 0, nparam, TRUE);
			else
				CODE_op(info->code, info->subcode, nparam, TRUE);
			break;

		default:

			CODE_op(info->code, info->subcode, nparam, (info->flag != RSF_OPN));
	}
}


static void trans_expr_from_tree(PATTERN *tree)
{
	int i;
	short nparam;
	int count;
	PATTERN pattern, prev_pattern; //, next_pattern

	count = ARRAY_count(tree) - 1;
	pattern = NULL_PATTERN;

	for (i = 0; i <= count; i++)
	{
		prev_pattern = pattern;
		pattern = tree[i];
		/*if (i < count)
			next_pattern = tree[i + 1];
		else
			next_pattern = NULL_PATTERN;*/

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
			trans_subr(PATTERN_index(pattern), nparam);
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
				/*if (FUNCTION_is_static(EVAL->func))
					THROW("ME cannot be used in a static function");*/

				CODE_push_me(TRUE);
			}
			else if (PATTERN_is(pattern, RS_SUPER))
			{
				/*if (FUNCTION_is_static(EVAL->func))
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
			else if (PATTERN_is(pattern, RS_PINF))
			{
				CODE_push_inf(FALSE);
			}
			else if (PATTERN_is(pattern, RS_MINF))
			{
				CODE_push_inf(TRUE);
			}
			else
			{
				nparam = get_nparam(tree, &i);
				TRANS_operation((short)PATTERN_index(pattern), nparam, prev_pattern);
			}
		}
	}
}

#if 0
static void trans_new(void)
{
	int index;
	int i, nparam;
	bool array = FALSE;
	bool event = FALSE;
	bool collection = FALSE;

	if (PATTERN_is_identifier(*EVAL->current))
	{
		index = PATTERN_index(*EVAL->current);
		CODE_push_class(CLASS_add_class(EVAL->class, index));
		nparam = 1;
	}
	else if (PATTERN_is_type(*EVAL->current))
	{
		if (PATTERN_is(EVAL->current[1], RS_LSQR))
		{
			CODE_push_number(RES_get_type(PATTERN_index(*EVAL->current)));
			nparam = 1;
		}
		else
			THROW("Cannot instanciate native types");
	}

	EVAL->current++;

	if (PATTERN_is(*EVAL->current, RS_LSQR))
	{
		if (collection)
			THROW("Array declaration is forbidden with typed collection");

		EVAL->current++;

		for (i = 0;; i++)
		{
			if (i > MAX_ARRAY_DIM)
				THROW("Too many dimensions");

			TRANS_expression(FALSE);
			nparam++;

			if (PATTERN_is(*EVAL->current, RS_RSQR))
				break;

			if (!PATTERN_is(*EVAL->current, RS_COMMA))
				THROW("Comma missing");

			EVAL->current++;
		}

		EVAL->current++;
		array = TRUE;
	}
	else
	{
		if (PATTERN_is(*EVAL->current, RS_LBRA))
		{
			EVAL->current++;

			for(;;)
			{
				if (nparam > MAX_PARAM_FUNC)
					THROW("Too many arguments");

				if (PATTERN_is(*EVAL->current, RS_AT))
					THROW("NEW cannot have output parameters");

				TRANS_expression(FALSE);
				nparam++;

				if (PATTERN_is(*EVAL->current, RS_RBRA))
					break;

				if (!PATTERN_is(*EVAL->current, RS_COMMA))
					THROW("Comma missing");

				EVAL->current++;
			}

			EVAL->current++;
		}

		if (PATTERN_is(*EVAL->current, RS_AS))
		{
			EVAL->current++;
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

void TRANS_expression()
{
	TRANS_tree();

	trans_expr_from_tree(EVAL->tree);

	ARRAY_delete(&EVAL->tree);
}


void TRANS_reference(void)
{
	TRANS_expression();

	if (!CODE_popify_last())
		THROW("Invalid assignment");

	EVAL->assign_code = EVAL->code[EVAL->ncode - 1];
}

static void trans_operation(short op, short nparam, PATTERN previous)
{
	COMP_INFO *info = &COMP_res_info[op];
	CODE_op(info->code, info->subcode, nparam, (info->flag != RSF_OPN));
}

bool TRANS_affectation(void)
{
	/*static TRANS_STATEMENT statement[] = {
		//{ RS_NEW, TRANS_new },
		{ RS_OPEN, TRANS_open },
		{ RS_SHELL, TRANS_shell },
		{ RS_EXEC, TRANS_exec },
		{ RS_RAISE, TRANS_raise },
		{ RS_PIPE, TRANS_pipe },
		{ RS_LOCK, TRANS_lock },
		{ RS_NONE, NULL }
	};*/

	//TRANS_STATEMENT *st;
	PATTERN *look = EVAL->current;
	PATTERN *left, *expr, *after;
	int niv = 0;
	bool equal = FALSE;
	//bool stat = FALSE;
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

	left = EVAL->current;
	*look++ = PATTERN_make(RT_NEWLINE, 0);
	expr = look;

	EVAL->current = expr;

	/*if (op == RS_NONE && (PATTERN_is_reserved(*EVAL->current)))
	{
		if (PATTERN_is(*EVAL->current, RS_NEW))
		{
			EVAL->current++;
			TRANS_in_affectation++;
			TRANS_new();
			TRANS_in_affectation--;
			stat = TRUE;
		}
		else
		{
			for (st = statement; st->id; st++)
			{
				if (PATTERN_is(*EVAL->current, st->id))
				{
					EVAL->current++;
					TRANS_in_affectation++;
					(*st->func)();
					TRANS_in_affectation--;
					stat = TRUE;
				}
			}
		}
	}*/

	//if (!stat)
	//{
		if (op != RS_NONE)
		{
			EVAL->current = left;
			TRANS_expression();
		}

		EVAL->current = expr;
		TRANS_expression();
		after = EVAL->current;

		if (op != RS_NONE)
		{
			/*if (op == RS_AMP)
				CODE_string_add();*/
			trans_operation(op, 2, NULL_PATTERN);
		}
	//}

	after = EVAL->current;

	/*if (dup)
		CODE_dup();*/
	
	CODE_dup(); // So that Assign() returns the assigned value

	EVAL->current = left;
	TRANS_reference();

	EVAL->current = after;

	return TRUE;
}


