/***************************************************************************

  gbc_trans_ctrl.c

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

#define _TRANS_CTRL_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_compile.h"
#include "gbc_trans.h"
#include "gb_code.h"
#include "gb_limit.h"

/*#define DEBUG*/
/*#define DEBUG_GOTO*/

#define BEGIN_NO_BREAK \
{ \
	bool nobreak = JOB->nobreak; \
	JOB->nobreak = TRUE;

#define END_NO_BREAK \
	JOB->nobreak = nobreak; \
}


static int ctrl_level;
static int ctrl_id;
static int ctrl_local;

static TRANS_CTRL ctrl_data[MAX_CTRL_LEVEL];

static TRANS_CTRL *current_ctrl;

static TRANS_GOTO *goto_info;
static TRANS_LABEL *label_info;
static short *ctrl_parent;


static void control_set_value(int value)
{
	if (ctrl_level <= 0)
		return;

	current_ctrl->value = value;
}


static int control_get_value(void)
{
	if (ctrl_level <= 0)
		return 0;

	return current_ctrl->value;
}


static void control_add_pos(ushort **tab_pos, ushort pos)
{
	if (!(*tab_pos))
		ARRAY_create(tab_pos);

	*((ushort *)ARRAY_add(tab_pos)) = pos;
}


static void control_add_current_pos(void)
{
	control_add_pos(&current_ctrl->pos, CODE_get_current_pos());
}

static void control_add_this_pos(ushort pos)
{
	control_add_pos(&current_ctrl->pos, pos);
}


static void control_jump_each_pos_with(ushort *tab_pos)
{
	int i;

	if (!tab_pos)
		return;

	for (i = 0; i < ARRAY_count(tab_pos); i++)
		CODE_jump_length(tab_pos[i], CODE_get_current_pos());
}


static void control_jump_each_pos(void)
{
	control_jump_each_pos_with(current_ctrl->pos);
}


static TRANS_CTRL *control_get_inner(void)
{
	int level = ctrl_level;
	TRANS_CTRL *ctrl_look;

	for(;;)
	{
		if (!level)
			return NULL;

		level--;
		ctrl_look = &ctrl_data[level];

		if ((ctrl_look->type == RS_DO)
				|| (ctrl_look->type == RS_WHILE)
				|| (ctrl_look->type == RS_REPEAT)
				|| (ctrl_look->type == RS_FOR)
				|| (ctrl_look->type == RS_EACH))
			return ctrl_look;
	}
}


static TRANS_CTRL *control_get_inner_with(void)
{
	int level = ctrl_level;
	TRANS_CTRL *ctrl_look;

	for(;;)
	{
		if (!level)
			return NULL;

		level--;
		ctrl_look = &ctrl_data[level];

		if (ctrl_look->type == RS_WITH)
			return ctrl_look;
	}
}


static void add_goto(int index, int mode)
{
	TRANS_GOTO *info;

	if (goto_info == NULL)
		ARRAY_create(&goto_info);

	info = ARRAY_add(&goto_info);
	info->index = index;
	info->pos = CODE_get_current_pos();
	info->ctrl_id = (ctrl_level == 0) ? 0 : current_ctrl->id;
	info->line = JOB->line;
	info->gosub = mode == RS_GOSUB;
	info->on_goto = mode == RS_NONE;

	#ifdef DEBUG_GOTO
		printf("add_goto: ctrl_id = %d (%ld)\n", info->ctrl_id, ARRAY_count(goto_info));
	#endif

	if (mode == RS_GOSUB)
		CODE_gosub(ctrl_local);
	else if (mode == RS_GOTO)
		CODE_jump();
	else
		CODE_nop();
}


static void control_enter(int type)
{
	short *parent;

	if (ctrl_level >= MAX_CTRL_LEVEL)
		THROW("Too many nested control structures.");

	ctrl_id++;

	current_ctrl = &ctrl_data[ctrl_level];

	current_ctrl->type = type;
	current_ctrl->value = 0;
	current_ctrl->pos = NULL;
	current_ctrl->state = 0;
	current_ctrl->local = ctrl_local;
	current_ctrl->id = ctrl_id;
	current_ctrl->loop_var = -1;

	parent = ARRAY_add(&ctrl_parent);

	if (ctrl_level == 0)
		*parent = 0;
	else
		*parent = ctrl_data[ctrl_level - 1].id;

	switch (type)
	{
		case RS_FOR:
		case RS_EACH:
			ctrl_local += 2;
			break;

		case RS_SELECT:
		case RS_WITH:
			ctrl_local += 1;
			break;
	}

	JOB->func->nctrl = Max(JOB->func->nctrl, ctrl_local - JOB->func->nlocal);

	ctrl_level++;
}


static void control_leave(void)
{
	control_jump_each_pos_with(current_ctrl->pos_break);

	ARRAY_delete(&current_ctrl->pos);
	ARRAY_delete(&current_ctrl->pos_break);
	ARRAY_delete(&current_ctrl->pos_continue);

	ctrl_local = current_ctrl->local;

	ctrl_level--;

	if (ctrl_level > 0)
		current_ctrl = &ctrl_data[ctrl_level - 1];
	else
		current_ctrl = NULL;
}


static void control_check(int type, const char *msg1, const char *msg2)
{
	if (ctrl_level <= 0)
		THROW(msg1);

	if (current_ctrl->type != type)
		THROW(E_UNEXPECTED, msg2);
}

static void control_check_two(int type1, int type2, const char *msg1, const char *msg2)
{
	if (ctrl_level <= 0)
		THROW(msg1);

	if (current_ctrl->type != type1 && current_ctrl->type != type2)
		THROW(E_UNEXPECTED, msg2);
}

static void control_check_loop_var(short var)
{
	int i;

	for (i = 0; i < (ctrl_level - 1); i++)
	{
		if (ctrl_data[i].loop_var == var)
			THROW("Loop variable already in use");
	}

	current_ctrl->loop_var = var;
}

static void check_try(const char *name)
{
	if (TRANS_in_try)
	{
		TRANS_in_try = FALSE;
		if (name)
			THROW("Cannot use TRY with &1", name);
		else
			THROW("Cannot use TRY twice");
	}
}

void TRANS_control_init(void)
{
	ctrl_level = 0;
	ctrl_id = 0;
	current_ctrl = NULL;

	goto_info = NULL;

	label_info = NULL;

	ctrl_local = JOB->func->nlocal;
	JOB->func->nctrl = 0;

	ARRAY_create(&ctrl_parent);
}


void TRANS_control_exit(void)
{
	int i;
	CLASS_SYMBOL *sym;
	int line;
	TRANS_LABEL *label;
	short id;

	/* GOTO are resolved */

	if (goto_info)
	{
		line = JOB->line;

		for (i = 0; i < ARRAY_count(goto_info); i++)
		{
			JOB->line = goto_info[i].line;
			/*printf("%d\n", JOB->line);*/

			sym = CLASS_get_symbol(JOB->class, goto_info[i].index);

			if (TYPE_get_kind(sym->local.type) != TK_LABEL)
				THROW("Label '&1' not declared", TABLE_get_symbol_name(JOB->class->table, goto_info[i].index));

			label = &label_info[sym->local.value];

			id = goto_info[i].ctrl_id;
			
			if (goto_info[i].gosub)
			{
				if (label->ctrl_id)
					THROW("Forbidden GOSUB");
			}
			else
			{
				for(;;)
				{
					if (label->ctrl_id == id)
						break;

					if (id == 0)
						THROW("Forbidden GOTO");

					#ifdef DEBUG_GOTO
						printf("id = %d ctrl_parent[id - 1] = %d (%ld)\n", id, ctrl_parent[id - 1], ARRAY_count(ctrl_parent));
					#endif
					id = ctrl_parent[id - 1];
				}
			}
			
			CODE_jump_length(goto_info[i].pos, label->pos);
		}

		JOB->line = line;
	}

	/* Remove previously declared labels */

	if (label_info)
	{
		for (i = 0; i < ARRAY_count(label_info); i++)
		{
			sym = CLASS_get_symbol(JOB->class, label_info[i].index);
			TYPE_clear(&sym->local.type);
		}
	}

	ARRAY_delete(&goto_info);
	ARRAY_delete(&ctrl_parent);
	ARRAY_delete(&label_info);

	/* On ne doit pas laisser une structure de controle ouverte */

	if (ctrl_level == 0) return;

	switch (ctrl_data[ctrl_level - 1].type)
	{
		case RS_IF:
			THROW(E_MISSING, "ENDIF");
		case RS_FOR:
		case RS_EACH:
			THROW(E_MISSING, "NEXT");
		case RS_DO:
			THROW(E_MISSING, "LOOP");
		case RS_REPEAT:
			THROW(E_MISSING, "UNTIL");
		case RS_WHILE:
			THROW(E_MISSING, "WEND");
		case RS_SELECT:
			THROW(E_MISSING, "END SELECT");
		case RS_WITH:
			THROW(E_MISSING, "END WITH");
	}
}

static ushort trans_jump_if(bool if_true)
{
	ushort pos;
	
	if (CODE_check_jump_not())
		if_true = !if_true;
	
	pos = CODE_get_current_pos();
		
	if (if_true)
		CODE_jump_if_true();
	else
		CODE_jump_if_false();
		
	return pos;
}

static void trans_endif(void)
{
	if (current_ctrl->state == 0)
		CODE_jump_length(control_get_value(), CODE_get_current_pos());

	control_jump_each_pos();
}

static void trans_else(void)
{
	BEGIN_NO_BREAK
	{
		control_add_current_pos();
		CODE_jump();
	}
	END_NO_BREAK

	CODE_jump_length(control_get_value(), CODE_get_current_pos());

	current_ctrl->state = 1;
}

static void trans_if(void)
{
	int mode = RS_NONE;
	char *msg;

	TRANS_expression(FALSE);

	if (PATTERN_is(*JOB->current, RS_AND))
	{
		mode = RS_AND;
		msg = "AND IF";
	}
	else if (PATTERN_is(*JOB->current, RS_OR))
	{
		mode = RS_OR;
		msg = "OR IF";
	}

	if (mode != RS_NONE)
	{
		control_enter(RS_IF);

		// IF NOT A THEN

		/*control_set_value(CODE_get_current_pos());

		if (mode == RS_AND)
			CODE_jump_if_true();
		else
			CODE_jump_if_false();*/
			
		control_set_value(trans_jump_if(mode == RS_AND));

		//   FALSE
		CODE_ignore_next_stack_usage();
		CODE_push_boolean(mode != RS_AND);

		for(;;)
		{
			if (!PATTERN_is(*JOB->current, mode))
				break;
			JOB->current++;

			TRANS_want(RS_IF, msg);

			// ELSE IF NOT B THEN

			trans_else();

			TRANS_expression(FALSE);

			/*control_set_value(CODE_get_current_pos());

			if (mode == RS_AND)
				CODE_jump_if_true();
			else
				CODE_jump_if_false();*/
				
			control_set_value(trans_jump_if(mode == RS_AND));

			//   FALSE
			CODE_ignore_next_stack_usage();
			CODE_push_boolean(mode != RS_AND);

			if (PATTERN_is(*JOB->current, RS_THEN) || PATTERN_is_newline(*JOB->current))
				break;
		}

		// ELSE

		trans_else();

		//   TRUE
		// ENDIF

		CODE_push_boolean(mode == RS_AND);
		trans_endif();

		control_leave();
	}

	if (PATTERN_is(*JOB->current, RS_THEN))
		JOB->current++;
	else if (!PATTERN_is_newline(*JOB->current))
		THROW(E_EXPECTED, "THEN");

	/*control_set_value(CODE_get_current_pos());
	CODE_jump_if_false();*/
	control_set_value(trans_jump_if(FALSE));
}

static void trans_else_if(void)
{
	trans_else();
	current_ctrl->state = 0;
	trans_if();
}


void TRANS_if(void)
{
	PATTERN *look;
	bool has_else;
	
	control_enter(RS_IF);

	trans_if();

	if (PATTERN_is_newline(*JOB->current))
		return;

	has_else = FALSE;
	look = JOB->current;
	for(;;)
	{
		look++;
		if (PATTERN_is_newline(*look))
			break;
		if (PATTERN_is(*look, RS_ELSE))
		{
			has_else = TRUE;
			*look = PATTERN_make(RT_NEWLINE, 0);
			break;
		}
	}
	
	TRANS_statement();
	
	if (has_else)
	{
		JOB->current++;
		trans_else();
		TRANS_statement();
	}
	
	TRANS_endif();
}


void TRANS_else(void)
{
	control_check(RS_IF, "ELSE without IF", "ELSE");

	if (current_ctrl->state)
		THROW(E_UNEXPECTED, "ELSE");

	if (TRANS_is(RS_IF))
		trans_else_if();
	else
		trans_else();
}


void TRANS_endif(void)
{
	control_check(RS_IF, "ENDIF without IF", "ENDIF");
	trans_endif();
	control_leave();
}


void TRANS_goto(void)
{
	int index;

	check_try("GOTO");
	
	if (!PATTERN_is_identifier(*JOB->current))
		THROW(E_SYNTAX);

	index = PATTERN_index(*JOB->current);
	JOB->current++;

	add_goto(index, RS_GOTO);
}

void TRANS_gosub(void)
{
	int index;

	check_try("GOSUB");
	
	if (!PATTERN_is_identifier(*JOB->current))
		THROW(E_SYNTAX);

	index = PATTERN_index(*JOB->current);
	JOB->current++;

	add_goto(index, RS_GOSUB);
}

void TRANS_on_goto_gosub(void)
{
	bool gosub;
	int pos, n;
	int index;
	
	TRANS_expression(FALSE);
	
	if (TRANS_is(RS_GOTO))
		gosub = FALSE;
	else if (TRANS_is(RS_GOSUB))
		gosub = TRUE;
	else
		THROW(E_SYNTAX);

	check_try(gosub ? "ON GOSUB" : "ON GOTO");
	
	pos = CODE_get_current_pos();
	CODE_nop();
	
	for (n = 1;; n++)
	{
		if (!PATTERN_is_identifier(*JOB->current))
			THROW(E_SYNTAX);
		
		index = PATTERN_index(*JOB->current);
		JOB->current++;

		add_goto(index, RS_NONE);
		
		if (!TRANS_is(RS_COMMA))
			break;
		
		if (n == 127)
			THROW("Too many labels");
	}
	
	pos = CODE_set_current_pos(pos);
	CODE_on(n);
	CODE_set_current_pos(pos);
	
	if (gosub)
		CODE_gosub(ctrl_local);
	else
		CODE_jump();
}

void TRANS_do(int type)
{
	bool is_until;

	control_enter(type);
	control_set_value(CODE_get_current_pos());

	if (type == RS_REPEAT)
		return;

	is_until = PATTERN_is(*JOB->current, RS_UNTIL);

	if (PATTERN_is(*JOB->current, RS_WHILE)
			|| is_until)
	{

		JOB->current++;

		TRANS_expression(FALSE);

		/*control_add_current_pos();

		if (is_until)
			CODE_jump_if_true();
		else
			CODE_jump_if_false();*/
			
		control_add_this_pos(trans_jump_if(is_until));
		
	}
}


void TRANS_loop(int type)
{
	ushort pos;

	bool is_until;

	if (type == RS_LOOP)
		control_check(RS_DO, "LOOP without DO", "LOOP");
	else if (type == RS_UNTIL)
		control_check(RS_REPEAT, "UNTIL without REPEAT", "UNTIL");
	else if (type == RS_WEND)
		control_check(RS_WHILE, "WEND without WHILE", "WEND");

	control_jump_each_pos_with(current_ctrl->pos_continue);

	is_until = PATTERN_is(*JOB->current, RS_UNTIL);

	if ((type != RS_WEND) && (PATTERN_is(*JOB->current, RS_WHILE) || is_until))
	{
		JOB->current++;

		TRANS_expression(FALSE);

		/*pos = CODE_get_current_pos();

		if (is_until)
			CODE_jump_if_false();
		else
			CODE_jump_if_true();*/
			
		pos = trans_jump_if(!is_until);
		
		CODE_jump_length(pos, control_get_value());
	}
	else
	{
		pos = CODE_get_current_pos();
		CODE_jump();
		CODE_jump_length(pos, control_get_value());
	}

	control_jump_each_pos();
	control_leave();
}


static void trans_select_break(bool do_not_add_pos)
{
	BEGIN_NO_BREAK
	{
		if (current_ctrl->value)
		{
			if (!do_not_add_pos)
			{
				control_add_current_pos();
				CODE_jump();
			}

			CODE_jump_length(current_ctrl->value, CODE_get_current_pos());
		}
	}
	END_NO_BREAK
}


void TRANS_select(void)
{
	control_enter(RS_SELECT);

	if (PATTERN_is(*JOB->current, RS_CASE))
		JOB->current++;

	TRANS_expression(FALSE);
	CODE_pop_ctrl(current_ctrl->local);
}


void TRANS_case(void)
{
	int i;
	short pos;
	short local;
	bool like;

	control_check(RS_SELECT, "CASE without SELECT", "CASE");
	
	if (current_ctrl->state)
		THROW("Default case must be the last one");

	trans_select_break(FALSE);

	local = current_ctrl->local;

	control_enter(RS_CASE);
	
	like = TRANS_is(RS_LIKE);
	
	for(i = 0; ; i++)
	{
		if (i > MAX_CASE_EXPR)
			THROW("Too many expressions in CASE");

		if (PATTERN_is_newline_end(*JOB->current))
			THROW("Unexpected end of line");
		
		/*CODE_dup();
		TRANS_expression(FALSE);
		CODE_op(C_EQ, 2);*/
		
		if (like)
		{
			CODE_push_local(local);
			TRANS_expression(FALSE);
			CODE_op(C_LIKE, 0, 2, TRUE);
		}
		else if (TRANS_is(RS_TO))
		{
			CODE_push_local(local);
			TRANS_expression(FALSE);
			CODE_op(C_LE, 0, 2, TRUE);
		}
		else
		{
			TRANS_expression(FALSE);

			if (TRANS_is(RS_TO))
			{
				CODE_push_local(local);
				CODE_op(C_LE, 0, 2, TRUE);
				CODE_push_local(local);
				TRANS_expression(FALSE);
				CODE_op(C_LE, 0, 2, TRUE);
				CODE_op(C_AND, 0, 2, TRUE);
			}
			else
			{
				CODE_push_local(local);
				CODE_op(C_EQ, 0, 2, TRUE);
			}
		}

		if (!PATTERN_is(*JOB->current, RS_COMMA))
		{
			pos = CODE_get_current_pos();
			CODE_jump_if_false();
			break;
		}

		control_add_current_pos();
		CODE_jump_if_true();

		JOB->current++;
		TRANS_newline();
	}

	control_jump_each_pos();
	control_leave();

	current_ctrl->value = pos;
}


void TRANS_default(void)
{
	control_check(RS_SELECT, "DEFAULT without SELECT", "DEFAULT");

	if (current_ctrl->state)
		THROW("Default case already defined");

	trans_select_break(FALSE);

	current_ctrl->value = 0; /*CODE_get_current_pos();*/
	current_ctrl->state = 1;
}


void TRANS_end_select(void)
{
	control_check(RS_SELECT, "END SELECT without SELECT", "END SELECT");

	/*
	if (current_ctrl->value)
		CODE_jump_length(current_ctrl->value, CODE_get_current_pos());
	*/

	trans_select_break(TRUE);

	control_jump_each_pos();

	control_leave();
}


void TRANS_break(void)
{
	TRANS_CTRL *ctrl_inner = control_get_inner();

	check_try("BREAK");

	if (!ctrl_inner)
		THROW(E_UNEXPECTED, "BREAK");

	control_add_pos(&ctrl_inner->pos_break, CODE_get_current_pos());
	CODE_jump();
}


void TRANS_continue(void)
{
	TRANS_CTRL *ctrl_inner = control_get_inner();

	check_try("CONTINUE");
	
	if (!ctrl_inner)
		THROW(E_UNEXPECTED, "CONTINUE");

	control_add_pos(&ctrl_inner->pos_continue, CODE_get_current_pos());
	CODE_jump();
}


void TRANS_return(void)
{
	if (FUNCTION_is_procedure(JOB->func))
	{
		if (!(PATTERN_is_newline(*JOB->current) || PATTERN_is_end(*JOB->current)))
			THROW("Return value datatype not specified in function declaration");
	}

	if (PATTERN_is_newline(*JOB->current) || PATTERN_is_end(*JOB->current))
		CODE_return(0);
	else
	{
		TRANS_expression(FALSE);
		CODE_return(1);
	}
}


void TRANS_for(void)
{
	short local;
	bool downto = FALSE;

	control_enter(RS_FOR);

	if (!TRANS_affectation(FALSE))
		THROW(E_SYNTAX);

	if (!CODE_check_pop_local_last(&local))
		THROW("Loop variable must be local");

	control_check_loop_var(local);

	if (TRANS_is(RS_DOWNTO))
		downto = TRUE;
	else
		TRANS_want(RS_TO, "TO");

	TRANS_expression(FALSE);

	if (PATTERN_is(*JOB->current, RS_STEP))
	{
		JOB->current++;
		TRANS_expression(FALSE);
		if (downto)
			CODE_op(C_NEG, 0, 1, TRUE);
	}
	else
	{
		CODE_push_number(downto ? -1 : 1);
	}

	/*CODE_pop_ctrl(current_ctrl->local + 1);
	CODE_pop_ctrl(current_ctrl->local);*/

	if (!PATTERN_is_newline(*JOB->current))
		THROW(E_UNEXPECTED, READ_get_pattern(JOB->current));

	CODE_jump_first(current_ctrl->local);

	control_set_value(CODE_get_current_pos());

	/*
	current = JOB->current;
	JOB->current = loop_var;
	TRANS_expression(FALSE);
	JOB->current = current;
	*/

	control_add_current_pos();
	CODE_jump_next();
	CODE_pop_local(local);

	/*
	current = JOB->current;
	JOB->current = loop_var;
	TRANS_expression(FALSE);

	if (!CODE_popify_last())
		ERROR_panic("Cannot popify FOR expression ??");

	JOB->current = current;
	*/
}


void TRANS_for_each(void)
{
	PATTERN *iterator = JOB->current;
	PATTERN *save;
	bool var = TRUE;

	while (!PATTERN_is(*JOB->current, RS_IN))
	{
		if (PATTERN_is_newline_end(*JOB->current))
		{
			JOB->current = iterator;
			var = FALSE;
			break;
		}
		JOB->current++;
	}

	control_enter(RS_EACH);

	if (var)
		JOB->current++;

	TRANS_expression(FALSE);

	/*CODE_pop_ctrl(current_ctrl->local);*/

	CODE_first(current_ctrl->local);

	control_set_value(CODE_get_current_pos());
	control_add_current_pos();

	if (var)
	{
		CODE_next(FALSE);

		save = JOB->current;
		JOB->current = iterator;

		TRANS_expression(FALSE);

		if (!CODE_popify_last())
			THROW("Invalid assignment");

		TRANS_want(RS_IN, "IN");

		JOB->current = save;
	}
	else
		CODE_next(TRUE);

	return;
}


void TRANS_next(void)
{
	ushort pos;

	control_check_two(RS_FOR, RS_EACH, "NEXT without FOR", "NEXT");

	/*
	if (current_ctrl->type == RS_FOR)
	{
		control_jump_each_pos_with(current_ctrl->pos_continue);

		pos = CODE_get_current_pos();
		CODE_jump();
		CODE_jump_length(pos, control_get_value());

		control_jump_each_pos();
		control_leave();
	}
	else
	{
	*/
	control_jump_each_pos_with(current_ctrl->pos_continue);

	pos = CODE_get_current_pos();
	CODE_jump();
	CODE_jump_length(pos, control_get_value());

	control_jump_each_pos();
	control_leave();
}


void TRANS_try(void)
{
	ushort pos;

	check_try(NULL);

	pos = CODE_get_current_pos();
	CODE_try();

	TRANS_in_try = TRUE;
	TRANS_statement();
	TRANS_in_try = FALSE;

	CODE_jump_length(pos, CODE_get_current_pos());
	CODE_end_try();
}


void TRANS_finally(void)
{
	ushort pos = CODE_get_current_pos();

	if ((JOB->func->finally != 0)
			|| (JOB->func->catch != 0)
			|| (pos == 0))
		THROW(E_UNEXPECTED, "FINALLY");

	JOB->func->finally = pos;
}


void TRANS_catch(void)
{
	ushort pos = CODE_get_current_pos();

	if ((JOB->func->catch != 0)
			|| (pos == 0))
		THROW(E_UNEXPECTED, "CATCH");

	CODE_catch();

	JOB->func->catch = CODE_get_current_pos();
}


void TRANS_label(void)
{
	CLASS_SYMBOL *sym;
	int sym_index;
	TRANS_LABEL *label;

	sym_index = PATTERN_index(*JOB->current);
	JOB->current++;

	sym = CLASS_declare(JOB->class, sym_index, TK_LABEL, FALSE);

	if (label_info == NULL)
		ARRAY_create(&label_info);

	sym->local.type = TYPE_make(T_VOID, 0L, TK_LABEL);
	sym->local.value = ARRAY_count(label_info);

	label = ARRAY_add(&label_info);

	label->index = sym_index;
	label->ctrl_id = (ctrl_level == 0) ? 0 : current_ctrl->id;
	label->pos = CODE_get_current_pos();

	#ifdef DEBUG_GOTO
	printf("TRANS_label: %.*s ctrl_id = %d\n", sym->symbol.len, sym->symbol.name, label->ctrl_id);
	#endif

	/* on saute le ':' */
	JOB->current++;
	
	/*
	// A new set of control stack slots must be used for a GOSUB subroutine
	
	for (i = 0; i < ARRAY_count(goto_info); i++)
	{
		if (goto_info[i].gosub && goto_info[i].index == sym_index)
		{
			ctrl_local = JOB->func->nctrl + JOB->func->nlocal;
			return;
		}
	}
	*/
}


void TRANS_with(void)
{
	control_enter(RS_WITH);

	if (!TRANS_affectation(TRUE))
		TRANS_expression(FALSE);

	CODE_pop_ctrl(current_ctrl->local);
}


void TRANS_use_with(void)
{
	TRANS_CTRL *ctrl_inner = control_get_inner_with();

	if (ctrl_inner == NULL)
		THROW("Syntax error. Point syntax used outside of WITH / END WITH");

	CODE_push_local(ctrl_inner->local);
}


void TRANS_end_with(void)
{
	control_check(RS_WITH, "END WITH without WITH", "END WITH");

	control_leave();
}


void TRANS_raise(void)
{
	CLASS_SYMBOL *sym;
	int index;
	int np;

	if (!PATTERN_is_identifier(*JOB->current))
		THROW("Syntax error in event name");

	index = PATTERN_index(*JOB->current);
	sym = CLASS_get_symbol(JOB->class, index);

	JOB->current++;

	if (TYPE_get_kind(sym->global.type) == TK_EVENT)
	{
		if (TYPE_is_static(JOB->func->type))
			THROW("Cannot raise events in static function");

		CODE_push_event(sym->global.value);
	}
	else
	{
		CODE_push_unknown_event(CLASS_add_unknown(JOB->class, index));
	}

	np = 0;

	if (TRANS_is(RS_LBRA))
	{
		for (;;)
		{
			if (TRANS_is(RS_RBRA))
				break;

			if (np > 0)
				TRANS_want(RS_COMMA, "comma");

			if (np >= MAX_PARAM_FUNC)
				THROW("Too many arguments");

			TRANS_expression(FALSE);
			np++;
		}
	}

	CODE_call(np);

	if (TRANS_in_affectation == 0)
		CODE_drop();
}
