/***************************************************************************

  gbc_trans_expr.c

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

//#define DEBUG 1

//static bool _accept_statement = FALSE;

static bool _must_drop_vargs = FALSE;

static TYPE _type[MAX_EXPR_LEVEL];
static int _type_level = 0;

#if DEBUG

static void _push_type(TYPE type)
{
	if (_type_level >= MAX_EXPR_LEVEL) // should have been detected by TRANS_tree()
		THROW("Expression too complex");
	
	_type[_type_level++] = type;
}

#define push_type(_type) fprintf(stderr, "push_type: %d in %s.%d\n", (_type).t.id, __func__, __LINE__), _push_type(_type)
#define push_type_id(_id) fprintf(stderr, "push_type_id: %d in %s.%d\n", (_id), __func__, __LINE__), _push_type(TYPE_make_simple(_id))
#define pop_type() fprintf(stderr, "pop type: in %s.%d\n", __func__, __LINE__),(_type[--_type_level])
#define drop_type(_n) fprintf(stderr, "drop type: %d in %s.%d\n", (_n), __func__, __LINE__),(_type_level -= (_n))
#define get_type(_i, _nparam) (fprintf(stderr, "get type(%d,%d): %d in %s.%d\n", (_i), (_nparam), (_type[_type_level + (_i) - (_nparam)].t.id), __func__, __LINE__),(_type[_type_level + (_i) - (_nparam)].t.id))

#else

static void push_type(TYPE type)
{
	if (_type_level >= MAX_EXPR_LEVEL) // should have been detected by TRANS_tree()
		THROW("Expression too complex");
	
	_type[_type_level++] = type;
}

#define push_type_id(_id) push_type(TYPE_make_simple(_id))
#define pop_type() (_type[--_type_level])
#define drop_type(_n) (_type_level -= (_n))
#define get_type(_i, _nparam) (_type[_type_level + (_i) - (_nparam)].t.id)

#endif

static short get_nparam(PATTERN *tree, int count, int *pindex, uint64_t *byref)
{
	PATTERN pattern;
	int nparam = 0;
	int index = *pindex;

	if (index < count)
	{
		pattern = tree[index + 1];
		if (PATTERN_is_param(pattern))
		{
			index++;
			nparam = PATTERN_index(pattern);
		}

		if (byref)
		{
			int shift = 0;
			*byref = 0;
			while (index < count)
			{
				pattern = tree[index + 1];
				if (!PATTERN_is_param(pattern))
					break;
				index++;
				*byref |= (uint64_t)PATTERN_index(pattern) << shift;
				shift += 16;
			}
		}
		else
		{
			while (index < count)
			{
				pattern = tree[index + 1];
				if (!PATTERN_is_param(pattern))
					break;
				index++;
			}
		}

		*pindex = index;
	}

	/*
		Gère le cas où on a codé un subr sans mettre de parenthèses
		=> nparam = 0
	*/

	return (short)nparam;
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
	}
	else
	{
		CLEAR(&decl);
		decl.type = TYPE_make_simple(number.type);
		decl.index = NO_SYMBOL;
		decl.value = index;
		if (number.type == T_LONG)
			decl.lvalue = number.lval;
		CODE_push_const(CLASS_add_constant(JOB->class, &decl));
	}

	if (number.complex)
	{
		CODE_push_complex();
		push_type_id(T_OBJECT);
	}
	else
		push_type_id(number.type);
}


static void push_string(int index, bool trans)
{
	TRANS_DECL decl;
	SYMBOL *sym;
	int len;

	if (index == VOID_STRING)
		len = 0;
	else
	{
		sym = TABLE_get_symbol(JOB->class->string, index);
		len = sym->len;
	}

	if (len == 0)
	{
		CODE_push_void_string();
	}
	else if (len == 1 && !trans)
	{
		CODE_push_char(*(sym->name));
	}
	else
	{
		//CLEAR(&decl);

		if (trans)
			decl.type = TYPE_make_simple(T_CSTRING);
		else
			decl.type = TYPE_make_simple(T_STRING);
		decl.index = NO_SYMBOL;
		decl.value = index;

		CODE_push_const(CLASS_add_constant(JOB->class, &decl));
	}
	
	push_type_id(T_STRING);
}

bool TRANS_string(PATTERN pattern)
{
	if (!PATTERN_is_string(pattern))
		return TRUE;
	push_string(PATTERN_index(pattern), FALSE);
	return FALSE;
}


static void trans_class(int index)
{
	const char *name;

	if (!CLASS_exist_class(JOB->class, index))
	{
		name = TABLE_get_symbol_name(JOB->class->table, index);
		THROW("Unknown identifier: &1", name);
	}
	
	CODE_push_class(CLASS_add_class(JOB->class, index));
	push_type_id(T_OBJECT);
}

static void trans_identifier(int index, bool point, PATTERN next)
{
	CLASS_SYMBOL *sym = CLASS_get_symbol(JOB->class, index);
	bool is_static;
	bool is_func;
	int type;
	CONSTANT *constant;

#if DEBUG
	fprintf(stderr, "trans_identifier: %.*s\n", sym->symbol.len, sym->symbol.name);
#endif
	
	if (!TYPE_is_null(sym->local.type) && !point)
	{
		CODE_push_local(sym->local.value);
		push_type(sym->local.type);
		sym->local_used = TRUE;
	}
	else if (!TYPE_is_null(sym->global.type) && !point)
	{
		type = TYPE_get_kind(sym->global.type);
		if (!TYPE_is_public(sym->global.type))
			sym->global_used = TRUE;

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
			
			push_type_id(type);
		}
		else if (type == TK_EXTERN)
		{
			if (PATTERN_is_point(next))
				goto __CLASS;

			CODE_push_extern(sym->global.value);
			push_type(JOB->class->ext_func[sym->global.value].type);
		}
		/* That breaks some code if the property has the same name as a class!
		else if (type == TK_PROPERTY)
		{
			CODE_push_me(FALSE);
			CODE_push_unknown(CLASS_add_unknown(JOB->class, index));
		}*/
		else if (type == TK_EVENT || type == TK_LABEL || type == TK_PROPERTY)
		{
			goto __CLASS;
		}
		else /* TK_FUNCTION || TK_VARIABLE */
		{
			is_static = TYPE_is_static(sym->global.type);
			is_func = type == TK_FUNCTION;

			if (is_func && PATTERN_is_point(next))
				goto __CLASS;

			if (!is_static && TYPE_is_static(JOB->func->type))
				THROW("Dynamic symbols cannot be used in static function");

			if (is_func && PATTERN_is_point(next))
				goto __CLASS;
			else
				CODE_push_global(sym->global.value, is_static, is_func);
				
			if (is_func)
				push_type(JOB->class->function[sym->global.value].type);
			else if (is_static)
				push_type(JOB->class->stat[sym->global.value].type);
			else
				push_type(JOB->class->dyn[sym->global.value].type);
		}
	}
	else
	{
		if (point)
		{
			CODE_push_unknown(CLASS_add_unknown(JOB->class, index));
			push_type_id(T_VOID);
		}
		else
			goto __CLASS;
	}

	return;

__CLASS:

	trans_class(index);
}


static void trans_subr(int subr, short nparam)
{
	SUBR_INFO *info = &COMP_subr_info[subr];
	int type;

	if (nparam < info->min_param)
		THROW("Not enough arguments to &1()", info->name);
	else if (nparam > info->max_param)
		THROW("Too many arguments to &1()", info->name);

	if (subr == SUBR_VarPtr)
	{
		if (CODE_check_varptr())
			THROW("VarPtr() argument must be a dynamic, a static or a local variable");
	}
	else if (subr == SUBR_IsMissing)
	{
		JOB->func->use_is_missing = TRUE;
		if (CODE_check_ismissing())
			THROW("IsMissing() requires a function argument");
	}

	CODE_subr(info->opcode, nparam, info->optype, info->max_param == info->min_param);
	
	type = info->type;
	switch(type)
	{
		case RST_SAME:
			type = get_type(0, nparam);
			break;
			
		case RST_BCLR:
			type = get_type(0, nparam);
			break;
	}
	
	if (nparam)
		drop_type(nparam);
	
	push_type_id(type);
}


static void trans_operation(short op, short nparam, PATTERN previous)
{
	COMP_INFO *info = &COMP_res_info[op];
	int type = info->type;

	switch (info->value)
	{
		case OP_PT:
			if (nparam == 0)
			{
				TRANS_use_with();
				type = T_OBJECT;
			}
			else if (!PATTERN_is_identifier(previous))
				THROW(E_SYNTAX);
			else
			{
				switch(get_type(0, nparam))
				{
					case T_OBJECT:
					case T_VARIANT:
						break;
						
					default:
						THROW("Not an object");
				}
			}

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

		case OP_COLON:
			TRANS_subr(TS_SUBR_COLLECTION, nparam);
			break;

		case OP_MINUS:
			if (nparam == 1)
			{
				CODE_op(C_NEG, 0, nparam, TRUE);
				type = RST_SAME;
			}
			else
				CODE_op(info->code, info->subcode, nparam, TRUE);
			break;

		default:
			CODE_op(info->code, info->subcode, nparam, (info->flag != RSF_OPN));
	}
	
	switch(type)
	{
		case RST_SAME:
			type = get_type(0, nparam);
			break;
			
		case RST_ADD:
			type = Max(get_type(0, nparam), get_type(1, nparam));
			if (type == T_DATE)
				type = T_FLOAT;
			break;
			
		case RST_AND:
			type = Max(get_type(0, nparam), get_type(1, nparam));
			if (type > T_LONG && type != T_VARIANT && type != T_STRING)
				THROW("Integer expected");
			break;

		case RST_NOT:
			type = get_type(0, nparam);
			if (type == T_STRING || type == T_OBJECT)
				type = T_BOOLEAN;
			break;
			
		case RST_MIN:
			type = Max(get_type(0, nparam), get_type(1, nparam));
			if (type > T_DATE && type != T_VARIANT)
				THROW("Number or Date expected");
	}
	
	if (nparam)
		drop_type(nparam);
	
	push_type_id(type);
}


static void trans_call(short nparam, uint64_t byref)
{
	if (!byref)
	{
		CODE_call(nparam);
	}
	else
	{
		CODE_call_byref(nparam, byref);

		if (_must_drop_vargs)
		{
			CODE_drop_vargs();
			_must_drop_vargs = FALSE;
		}
	}
	
	drop_type(nparam + 1);
	
	push_type_id(T_VARIANT);
}

static void trans_expr_from_tree(TRANS_TREE *tree, int count)
{
	static void *jump[] = {
		&&__CONTINUE, &&__CONTINUE, &&__RESERVED, &&__IDENTIFIER, &&__NUMBER, &&__STRING, &&__TSTRING, &&__CONTINUE, &&__SUBR, &&__CLASS, &&__CONTINUE, &&__CONTINUE
	};
	
	int i, op;
	short nparam;
	PATTERN pattern, next_pattern, prev_pattern;
	uint64_t byref = 0;

	pattern = NULL_PATTERN;
	_type_level = 0;

	#if DEBUG
	fprintf(stderr, "-----------------------\n");
	#endif

	i = 0;
	
	for(i = 0; i < count; i++)
	{
		prev_pattern = pattern;
		pattern = tree[i];
		next_pattern = tree[i + 1];

		goto *jump[PATTERN_type(pattern)];
		
	__NUMBER:
		
		push_number(PATTERN_index(pattern));
		continue;
		
	__STRING:

		push_string(PATTERN_index(pattern), FALSE);
		continue;

	__TSTRING:

		push_string(PATTERN_index(pattern), TRUE);
		continue;

	__IDENTIFIER:
	
		trans_identifier(PATTERN_index(pattern), PATTERN_is_point(pattern), next_pattern);
		continue;

	__CLASS:
	
		trans_class(PATTERN_index(pattern));
		continue;

	__SUBR:
	
		nparam = get_nparam(tree, count, &i, NULL);
		trans_subr(PATTERN_index(pattern), nparam);
		continue;
		
	__RESERVED:

		if (PATTERN_is(pattern, RS_TRUE))
		{
			CODE_push_boolean(TRUE);
			push_type_id(T_BOOLEAN);
		}
		else if (PATTERN_is(pattern, RS_FALSE))
		{
			CODE_push_boolean(FALSE);
			push_type_id(T_BOOLEAN);
		}
		else if (PATTERN_is(pattern, RS_NULL))
		{
			CODE_push_null();
			push_type_id(T_OBJECT);
		}
		else if (PATTERN_is(pattern, RS_ME))
		{
			/*if (FUNCTION_is_static(JOB->func))
				THROW("ME cannot be used in a static function");*/

			CODE_push_me(FALSE);
			push_type_id(T_OBJECT);
		}
		else if (PATTERN_is(pattern, RS_SUPER))
		{
			/*if (FUNCTION_is_static(JOB->func))
				THROW("ME cannot be used in a static function");*/

			CODE_push_super(FALSE);
			push_type_id(T_OBJECT);
		}
		else if (PATTERN_is(pattern, RS_LAST))
		{
			CODE_push_last();
			push_type_id(T_OBJECT);
		}
		else if (PATTERN_is(pattern, RS_AT))
		{
			if (!CODE_popify_last())
				THROW("This expression cannot be passed by reference");
		}
		else if (PATTERN_is(pattern, RS_COMMA))
		{
			CODE_drop();
			drop_type(1);
		}
		else if (PATTERN_is(pattern, RS_ERROR))
		{
			TRANS_subr(TS_SUBR_ERROR, 0);
		}
		else if (PATTERN_is(pattern, RS_OPTIONAL))
		{
			CODE_push_void();
			push_type_id(T_VOID);
		}
		else if (PATTERN_is(pattern, RS_PINF))
		{
			CODE_push_inf(FALSE);
			push_type_id(T_FLOAT);
		}
		else if (PATTERN_is(pattern, RS_MINF))
		{
			CODE_push_inf(TRUE);
			push_type_id(T_FLOAT);
		}
		else if (PATTERN_is(pattern, RS_3PTS))
		{
			_must_drop_vargs = TRUE;
			CODE_push_vargs();
			// push_type_id(T_FLOAT); we push no type
		}
		else
		{
			op = PATTERN_index(pattern);
			if (op == RS_LBRA)
			{
				nparam = get_nparam(tree, count, &i, &byref);
				trans_call(nparam, byref);
			}
			else
			{
				nparam = get_nparam(tree, count, &i, NULL);
				trans_operation((short)op, nparam, prev_pattern);
			}
		}
		
	__CONTINUE:
		;
	}
}


void TRANS_new(void)
{
	int index;
	int i, nparam;
	bool array = FALSE;
	bool event = FALSE;
	//bool collection = FALSE;
	bool check_param = FALSE;

	nparam = 0;

	if (PATTERN_is_class(*JOB->current))
	{
		index = CLASS_add_class(JOB->class, PATTERN_index(*JOB->current));
		if (PATTERN_is(JOB->current[1], RS_LSQR))
			index = CLASS_get_array_class(JOB->class, T_OBJECT, index);

		CODE_push_class(index);
		nparam = 1;
	}
	else if (PATTERN_is_type(*JOB->current))
	{
		if (!PATTERN_is(JOB->current[1], RS_LSQR))
			THROW("Cannot instantiate native types");

		//CODE_push_number(RES_get_type(PATTERN_index(*JOB->current)));
		CODE_push_class(CLASS_get_array_class(JOB->class, RES_get_type(PATTERN_index(*JOB->current)), -1));
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

	if (TRANS_is(RS_LSQR))
	{
		//if (collection)
		//	THROW("Array declaration is forbidden with typed collection");

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
					THROW(E_MISSING, "']'");

				JOB->current++;
			}
		}

		JOB->current++;
		array = TRUE;
	}
	else
	{
		if (TRANS_is(RS_LBRA))
		{
			for(;;)
			{
				if (nparam > MAX_PARAM_FUNC)
					THROW("Too many arguments");

				if (PATTERN_is(*JOB->current, RS_AT) || PATTERN_is(*JOB->current, RS_BYREF))
					THROW("NEW cannot have arguments passed by reference");

				TRANS_expression(FALSE);
				nparam++;

				if (PATTERN_is(*JOB->current, RS_RBRA))
					break;

				if (!PATTERN_is(*JOB->current, RS_COMMA))
					THROW(E_MISSING, "')'");

				JOB->current++;
			}

			JOB->current++;

			if (check_param && nparam == 0)
				THROW("Not enough argument to New()");
		}

		if (TRANS_is(RS_AS))
		{
			TRANS_expression(FALSE);
			nparam++;
			event = TRUE;
		}

		/*
		CODE_call(nparam, FALSE);
		CODE_drop();
		*/
	}

	//if (collection)
	//	CODE_new(nparam, TRUE, event);
	//else
	CODE_new(nparam, array, event);
}


void TRANS_expression(bool check_statement)
{
	TRANS_TREE *tree;
	int tree_length;

	if (!check_statement)
	{
		if (TRANS_is(RS_NEW))
		{
			TRANS_new();
			return;
		}
		else if (TRANS_is(RS_READ))
		{
			TRANS_read();
			return;
		}
	}

	TRANS_tree(check_statement, &tree, &tree_length);

	trans_expr_from_tree(tree, tree_length);

	FREE(&tree);

	if (check_statement)
	{
		/*
		if (!CODE_check_statement_last())
			THROW("This expression cannot be a statement");
		*/

		CODE_drop();
	}
}

void TRANS_ignore_expression()
{
	TRANS_tree(FALSE, NULL, NULL);
}

TYPE TRANS_variable_get_type()
{
	TYPE type = TYPE_make_simple(T_VOID);
	TRANS_TREE *tree;
	int count;
	int index;
	CLASS_SYMBOL *sym;

	TRANS_tree(FALSE, &tree, &count);

	if (count == 1 && PATTERN_is_identifier(*tree))
	{
		index = PATTERN_index(*tree);
		sym = CLASS_get_symbol(JOB->class, index);

		if (!TYPE_is_null(sym->local.type))
		{
			return sym->local.type;
		}
		else if (!TYPE_is_null(sym->global.type))
		{
			int type = TYPE_get_kind(sym->global.type);
			if (type == TK_VARIABLE)
				return sym->global.type;
		}
	}

	FREE(&tree);

	return type;
}


void TRANS_reference(void)
{
	TRANS_expression(FALSE);

	if (!CODE_popify_last())
		THROW("Invalid assignment");
}


bool TRANS_affectation(bool dup)
{
	static TRANS_STATEMENT statement[] = {
		//{ RS_NEW, TRANS_new },
		{ RS_OPEN, TRANS_open },
		{ RS_CLOSE, TRANS_close },
		{ RS_SHELL, TRANS_shell },
		{ RS_EXEC, TRANS_exec },
		{ RS_RAISE, TRANS_raise },
		{ RS_PIPE, TRANS_pipe },
		{ RS_LOCK, TRANS_lock },
		{ RS_MEMORY, TRANS_memory },
		//{ RS_READ, TRANS_read },
		{ RS_NONE, NULL }
	};

	TRANS_STATEMENT *st;
	PATTERN *look = JOB->current;
	PATTERN *left, *expr, *after;
	int niv = 0;
	bool equal = FALSE;
	bool stat = FALSE;
	int op = RS_NONE;
	int id = RS_NONE;

	for(;;)
	{
		if (PATTERN_is_newline(*look)) // || PATTERN_is_end(*look))
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

	if (!equal || look == JOB->current)
		return FALSE;

	left = JOB->current;
	*look++ = PATTERN_make(RT_NEWLINE, 0);
	expr = look;

	JOB->current = expr;

	if (op == RS_NONE && (PATTERN_is_reserved(*JOB->current)))
	{
		if (TRANS_is(RS_NEW))
		{
			TRANS_in_affectation++;
			TRANS_new();
			TRANS_in_affectation--;
			id = RS_NEW;
			stat = TRUE;
		}
		else
		{
			for (st = statement; st->id; st++)
			{
				if (TRANS_is(st->id))
				{
					id = st->id;
					TRANS_in_affectation++;
					(*st->func)();
					TRANS_in_affectation--;
					stat = TRUE;
					break;
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
		{
			/*if (op == RS_AMP)
				CODE_string_add();*/
			trans_operation(op, 2, NULL_PATTERN);
		}
	}

	after = JOB->current;

	if (dup)
		CODE_dup();

	if (COMPILE_version >= 0x03070000)
	{
		if (id == RS_EXEC || id == RS_SHELL)
			CODE_dup();
	}

	JOB->current = left;
	TRANS_reference();

	if (!PATTERN_is_newline(*JOB->current))
		THROW(E_SYNTAX);

	JOB->current = after;

	if (COMPILE_version >= 0x03070000)
	{
		if (id == RS_EXEC || id == RS_SHELL)
		{
			TRANS_subr(TS_SUBR_CHECK_EXEC, 1);
			CODE_drop();
		}
	}

	return TRUE;
}


