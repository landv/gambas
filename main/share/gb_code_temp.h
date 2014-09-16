/***************************************************************************

	gb_code_temp.h

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

//#define DEBUG

#define write_Zxxx(code, val)  write_short(code | ((short)val & 0x0FFF))
#define write_Z8xx(code, val)  write_short(code | ((short)val & 0x07FF))
#define write_ZZxx(code, val)  write_short(code | ((short)val & 0x00FF))

#ifndef PROJECT_EXEC
#define LAST_CODE \
{ \
	if (JOB->debug && !JOB->nobreak) \
		CODE_break(); \
	cur_func->last_code2 = cur_func->last_code; \
	cur_func->last_code = cur_func->ncode; \
}
#else
#define LAST_CODE \
{ \
	cur_func->last_code2 = cur_func->last_code; \
	cur_func->last_code = cur_func->ncode; \
}
#endif

short CODE_stack_usage;
short CODE_stack;

static bool _ignore_next_stack_usage = FALSE;

#ifdef PROJECT_EXEC
#define cur_func EVAL
#else
static FUNCTION *cur_func = NULL;
static int last_line = 0;
#endif

static void alloc_code(void)
{
	cur_func->ncode_max += CODE_INSTR_INC;
	if (!cur_func->code)
		ALLOC(&cur_func->code, sizeof(short) * CODE_INSTR_INC);
	else
		REALLOC(&cur_func->code, sizeof(short) * cur_func->ncode_max);
}

//static void INLINE write_short(short value)
#define write_short(_value) \
({ \
	if (cur_func->ncode >= cur_func->ncode_max) \
		alloc_code(); \
	\
	cur_func->code[cur_func->ncode] = _value; \
	/*fprintf(stderr, "[%d] %04hX\n", cur_func->ncode, (ushort)value);*/ \
	cur_func->ncode++; \
})

#ifdef PROJECT_COMP

static bool _allow_break = FALSE;

void CODE_allow_break(void)
{
	_allow_break = TRUE;
}

static void CODE_break(void)
{
	if (!_allow_break)
		return;

	/*if (last_line < 0)
	{
		if (CODE_get_current_pos())
			return;
	}
	else
	{
		if (JOB->line == last_line)
			return;

		last_line = JOB->line;
	}*/

	#ifdef DEBUG
	printf("BREAK\n");
	#endif

	write_short(C_BREAK);
	_allow_break = FALSE;
}

#endif


static void write_int(int value)
{
	write_short(value & 0xFFFF);
	write_short((unsigned int)value >> 16);
}


/*static void remove_last(void)
{
	ARRAY_remove_last(&cur_func->code);
	cur_func->last_code = ARRAY_count(cur_func->code);
}*/


static void use_stack(int use)
{
	if (_ignore_next_stack_usage)
	{
		_ignore_next_stack_usage = FALSE;
		return;
	}

	CODE_stack += use;
	CODE_stack_usage = Max(CODE_stack_usage, CODE_stack);
	#ifdef DEBUG
	printf("%04d: %d\n", cur_func->ncode, CODE_stack);
	#endif
}

static void CODE_undo()
{
	cur_func->ncode = cur_func->last_code;
	cur_func->last_code = cur_func->last_code2;
	cur_func->last_code2 = (-1);
}

ushort CODE_get_current_pos(void)
{
	return cur_func->ncode;
}

ushort CODE_set_current_pos(ushort pos)
{
	ushort old = cur_func->ncode;
	cur_func->ncode = pos;
	return old;
}


void CODE_ignore_next_stack_usage(void)
{
	_ignore_next_stack_usage = TRUE;
}

#ifdef PROJECT_EXEC

void CODE_begin_function()
{
	CODE_stack = 0;
	CODE_stack_usage = 0;
}

void CODE_end_function()
{
	if (CODE_stack)
		THROW("Internal compiler error: bad stack usage computed!");
}

#else

void CODE_begin_function(FUNCTION *func)
{
	cur_func = func;
	CODE_stack = 0;
	CODE_stack_usage = 0;
	if (func->start == NULL)
		last_line = (-1);
	else
		last_line = 0;
}

void CODE_end_function(FUNCTION *func)
{
	if (CODE_stack)
		THROW("Internal compiler error: bad stack usage computed!");
}

#endif


static ushort *get_last_code()
{
	if ((ushort)cur_func->last_code == CODE_NO_POS)
		return NULL;

	return &cur_func->code[cur_func->last_code];
}

static ushort *get_last_code2()
{
	if ((ushort)cur_func->last_code2 == CODE_NO_POS)
		return NULL;

	return &cur_func->code[cur_func->last_code2];
}

bool CODE_popify_last(void)
{
	/*
	#ifdef DEBUG
	printf("CODE_is_last_popable ? ");
	if (!last_code) printf("FALSE, last_code = NULL");
	else printf("0x%04hX", *last_code);
	printf("\n");
	#endif
	*/
	unsigned short *last_code, op;

	last_code = get_last_code();
	if (!last_code)
		return FALSE;

	op = *last_code & 0xFF00;

	if ((op >= C_PUSH_LOCAL) && (op <= C_PUSH_UNKNOWN))
	{
		*last_code += 0x0800;
		use_stack(-2);
		#ifdef DEBUG
		printf("Popify Last\n");
		#endif
		return TRUE;
	}

	if ((op & 0xF000) == C_PUSH_DYNAMIC)
	{
		*last_code += 0x1000;
		use_stack(-2);
		#ifdef DEBUG
		printf("Popify Last\n");
		#endif
		return TRUE;
	}
	
	/*
	if (*last_code == (C_PUSH_MISC | CPM_LAST))
	{
		*last_code = C_PUSH_MISC | CPM_POP_LAST;
		use_stack(-2);
		return TRUE;
	}
	*/
	/*
	if (op == C_CALL)
	{
		*last_code = C_CALL_POP | (*last_code & 0xFF);
		return TRUE;
	}
	*/

	return FALSE;
}


#ifdef PROJECT_COMP

bool CODE_check_statement_last(void)
{
	unsigned short op;
	PCODE *last_code;

	last_code = get_last_code();
	if (!last_code)
		return FALSE;

	op = *last_code & 0xFF00;

	if (op == C_CALL)
		return TRUE;

	op >>= 8;

	if (op >= CODE_FIRST_SUBR && op <= CODE_LAST_SUBR)
		return TRUE;

	return FALSE;
}


bool CODE_check_pop_local_last(short *local)
{
	PCODE *last_code;

	last_code = get_last_code();
	if (!last_code)
		return FALSE;

	if ((*last_code & 0xFF00) == C_POP_LOCAL)
	{
		*local = *last_code & 0xFF;
		return TRUE;
	}

	return FALSE;
}

bool CODE_check_jump_not(void)
{
	ushort op;
	PCODE *last_code = get_last_code();
	
	if (!last_code)
		return FALSE;
		
	op = *last_code & 0xFF00;
	if (op != C_NOT)
		return FALSE;
		
	CODE_undo();
	return TRUE;
}

#endif

bool CODE_check_varptr(void)
{
	unsigned short op;
	PCODE *last_code;

	last_code = get_last_code();
	if (!last_code)
		return TRUE;

	op = *last_code;
	if (!((op & 0xFF00) == C_PUSH_LOCAL || (op & 0xFF00) == C_PUSH_PARAM || (op & 0xF800) == C_PUSH_STATIC || (op & 0xF800) == C_PUSH_DYNAMIC))
		return TRUE;

	*last_code = C_PUSH_INTEGER;
	write_short((short)op);
	return FALSE;
}

bool CODE_check_ismissing(void)
{
	unsigned short op;
	PCODE *last_code;

	last_code = get_last_code();
	if (!last_code)
		return TRUE;

	op = *last_code;
	if ((op & 0xFF00) != C_PUSH_PARAM)
		return TRUE;

	*last_code = C_PUSH_QUICK | (op & 0xFF);
	return FALSE;
}

void CODE_nop(void)
{
	LAST_CODE;
	write_short(C_NOP);
}

void CODE_push_number(int value)
{
	LAST_CODE;

	use_stack(1);

	if (value >= -2048L && value < 2048L)
	{
		#ifdef DEBUG
		printf("PUSH QUICK %d\n", value);
		#endif
		write_Zxxx(C_PUSH_QUICK, value);
	}
	else if (value >= -32768L && value < 32768L)
	{
		#ifdef DEBUG
		printf("PUSH INTEGER %d\n", value);
		#endif
		write_short(C_PUSH_INTEGER);
		write_short((short)value);
	}
	else
	{
		#ifdef DEBUG
		printf("PUSH LONG %d\n", value);
		#endif
		write_short(C_PUSH_LONG);
		write_int(value);
	}
}


void CODE_push_const(ushort value)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH CONST %d %s\n", value, TABLE_get_symbol_name(JOB->class->table, JOB->class->constant[value].index));
	#endif
	
	if (value < 0xF00)
		write_Zxxx(C_PUSH_CONST, value);
	else
	{
		write_Zxxx(C_PUSH_CONST, 0xF00);
		write_short((short)value);
	}
}


void CODE_push_local(short num)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	if (num >= 0)
		printf("PUSH LOCAL %d\n", num);
	else
		printf("PUSH PARAM %d\n", (-1) - num);
	#endif
	if (num >= 0)
		write_ZZxx(C_PUSH_LOCAL, num);
	else
		write_ZZxx(C_PUSH_PARAM, num);
}


#ifdef PROJECT_COMP

void CODE_pop_local(short num)
{
	LAST_CODE;

	use_stack(-1);

	#ifdef DEBUG
	if (num >= 0)
		printf("POP LOCAL #%d\n", num);
	else
		printf("POP PARAM #%d\n", (-1) - num);
	#endif
	if (num >= 0)
		write_ZZxx(C_POP_LOCAL, num);
	else
		write_ZZxx(C_POP_PARAM, num);
}


void CODE_pop_ctrl(short num)
{
	LAST_CODE;

	use_stack(-1);

	#ifdef DEBUG
	printf("POP CTRL #%d\n", num);
	#endif

	write_ZZxx(C_POP_CTRL, num);
}


void CODE_pop_optional(short num)
{
	LAST_CODE;

	use_stack(-1);

	#ifdef DEBUG
	printf("POP OPTIONAL #%d\n", (-1) - num);
	#endif
	write_ZZxx(C_POP_OPTIONAL, num);
}

#endif /* PROJECT_COMP */


void CODE_push_array(short nparam)
{
	LAST_CODE;

	use_stack(1 - nparam);

	write_ZZxx(C_PUSH_ARRAY, nparam);
}


void CODE_push_global(short global, bool is_static, bool is_function)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH %s %d\n", is_static ? "STATIC" : "DYNAMIC", global);
	#endif

	if (is_function)
		write_Z8xx(C_PUSH_FUNCTION, global);
	else if (is_static)
		write_Z8xx(C_PUSH_STATIC, global);
	else
		write_Z8xx(C_PUSH_DYNAMIC, global);
}


#ifdef PROJECT_COMP

void CODE_pop_global(short global, bool is_static)
{
	LAST_CODE;

	use_stack(-1);

	#ifdef DEBUG
	printf("POP %s %d\n", is_static ? "STATIC" : "DYNAMIC", global);
	#endif

	if (is_static)
		write_Z8xx(C_POP_STATIC, global);
	else
		write_Z8xx(C_POP_DYNAMIC, global);
}

#endif

/*
void CODE_push_symbol(short symbol)
{
	LAST_CODE;

	use_stack(0);

	#ifdef DEBUG
	printf("PUSH SYMBOL %s\n", TABLE_get_symbol_name(JOB->class->table, symbol));
	#endif

	write_short(C_PUSH_SYMBOL);
	write_short(symbol);
}


void CODE_pop_symbol(short symbol)
{
	LAST_CODE;

	use_stack(-2);

	#ifdef DEBUG
	printf("POP SYMBOL %s\n", TABLE_get_symbol_name(JOB->class->table, symbol));
	#endif

	write_short(C_POP_SYMBOL);
	write_short(symbol);
}
*/


void CODE_push_unknown(short symbol)
{
	LAST_CODE;

	use_stack(0);

	#ifdef DEBUG
	printf("PUSH UNKNOWN %s\n", TABLE_get_symbol_name(JOB->class->table, symbol));
	#endif

	write_short(C_PUSH_UNKNOWN);
	write_short(symbol);
}

void CODE_push_unknown_event(short symbol)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH UNKNOWN EVENT %s\n", TABLE_get_symbol_name(JOB->class->table, symbol));
	#endif

	write_ZZxx(C_PUSH_EVENT, 0xFF);
	write_short(symbol);
}


#ifdef PROJECT_COMP

void CODE_pop_unknown(short symbol)
{
	LAST_CODE;

	use_stack(-2);

	#ifdef DEBUG
	printf("POP UNKNOWN %s\n", TABLE_get_symbol_name(JOB->class->table, symbol));
	#endif

	write_short(C_POP_UNKNOWN);
	write_short(symbol);
}

#endif


void CODE_push_class(short class)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH CLASS %d\n", class);
	#endif

	write_Z8xx(C_PUSH_CLASS, class);

	#ifdef PROJECT_COMP
	JOB->class->class[class].used = TRUE;
	#endif
}

#ifdef PROJECT_COMP

void CODE_jump()
{
	LAST_CODE;

	#ifdef DEBUG
	printf("JUMP\n");
	#endif
	write_short(C_JUMP);
	write_short(0);
}

void CODE_gosub(int ctrl_local)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("GOSUB\n");
	#endif
	write_ZZxx(C_GOSUB, ctrl_local);
	write_short(0);
}

void CODE_on(uchar num)
{
	LAST_CODE;

	use_stack(-1);
	
	#ifdef DEBUG
	printf("ON\n");
	#endif
	write_ZZxx(C_ON, num);
}


void CODE_jump_if_true()
{
	/*
	ushort *last_code = get_last_code();
	ushort op;

	if (last_code && PCODE_is(*last_code, C_NOT))
	{
		remove_last();
		op = C_JUMP_IF_FALSE;
	}
	else
		op = C_JUMP_IF_TRUE;
	*/

	use_stack(-1);

	#ifdef DEBUG
	printf("JUMP IF TRUE\n");
	#endif

	LAST_CODE;

	write_short(C_JUMP_IF_TRUE);
	/**pos = CODE_get_current_pos();*/
	write_short(0);
}


void CODE_jump_if_false()
{
	/*
	ushort *last_code = get_last_code();
	ushort op;

	if (last_code && PCODE_is(*last_code, C_NOT))
	{
		remove_last();
		op = C_JUMP_IF_TRUE;
	}
	else
		op = C_JUMP_IF_FALSE;
	*/

	use_stack(-1);

	#ifdef DEBUG
	printf("JUMP IF FALSE\n");
	#endif

	LAST_CODE;

	write_short(C_JUMP_IF_FALSE);
	/**pos = CODE_get_current_pos();*/
	write_short(0);
}


void CODE_jump_first(short local)
{
	LAST_CODE;

	use_stack(-2);

	#ifdef DEBUG
	printf("JUMP FIRST LOCAL %d\n", local);
	#endif

	write_ZZxx(C_JUMP_FIRST, local);
}


void CODE_jump_next(void)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("JUMP NEXT\n");
	#endif
	write_short(C_JUMP_NEXT);
	/**pos = CODE_get_current_pos();*/
	write_short(0);
}


void CODE_jump_length(ushort src, ushort dst)
{
	if (src >= (cur_func->ncode - 1))
		return;

	int diff = (int)dst - (int)src;
	
	if (diff < -32768 || diff > 32767)
		THROW("Jump is too far");
	
	if (cur_func->code[src] == C_BREAK)
		cur_func->code[src + 2] = (short)(diff - 3); //dst - (src + 2) - 1;
	else if (cur_func->code[src] == C_NOP)
		cur_func->code[src] = (short)diff; //dst - src;
	else
		cur_func->code[src + 1] = (short)(diff - 2); //dst - (src + 1) - 1;
}


void CODE_first(short local)
{
	LAST_CODE;

	use_stack(-1);

	#ifdef DEBUG
	printf("ENUM FIRST LOCAL %d\n", local);
	#endif

	write_ZZxx(C_FIRST, local);
}


void CODE_next(bool drop)
{
	LAST_CODE;

	use_stack(drop ? 0 : 1);

	#ifdef DEBUG
	printf("ENUM NEXT%s\n", drop ? " DROP" : "");
	#endif

	write_ZZxx(C_NEXT, drop ? 1 : 0);
	write_short(0);
}

#endif /* PROJECT_COMP */

void CODE_op(short op, short subcode, short nparam, bool fixed)
{
	if (op == C_ADD || op == C_SUB)
	{
		PCODE *last_code;
		short value, value2;
		
		last_code = get_last_code();
		
		if (last_code && ((*last_code & 0xF000) == C_PUSH_QUICK))
		{
			value = *last_code & 0xFFF;
			if (value >= 0x800) value |= 0xF000;
			if (op == C_SUB) value = (-value);

			#ifdef DEBUG
			printf("ADD QUICK %d\n", value);
			#endif

			*last_code = C_ADD_QUICK | (value & 0x0FFF);

			use_stack(1 - nparam);
			
			// Now, look if we are PUSH QUICK then ADD QUICK
			
			last_code = get_last_code2();
			if (last_code && ((*last_code & 0xF000) == C_PUSH_QUICK))
			{
				value2 = *last_code & 0xFFF;
				if (value2 >= 0x800) value2 |= 0xF000;
				value += value2;
				
				if (value >= -2048L && value < 2048L)
				{
					*last_code = C_PUSH_QUICK | (value & 0x0FFF);
					CODE_undo();
				}
			}
			
			return;
		}
	}

	LAST_CODE;

	use_stack(1 - nparam);

	#ifdef DEBUG
	printf("OP %d (%d)\n", op, nparam);
	#endif

	if (fixed)
		write_ZZxx(op, subcode);
	else
		write_ZZxx(op, nparam);
}


void CODE_push_me(bool debug)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH ME\n");
	#endif

	write_ZZxx(C_PUSH_ME, debug ? 1 : 0);
}


void CODE_push_super(bool debug)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH SUPER\n");
	#endif

	write_ZZxx(C_PUSH_ME, debug ? 3 : 2);
}


void CODE_push_last()
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH LAST\n");
	#endif

	write_ZZxx(C_PUSH_MISC, CPM_LAST);
}


void CODE_push_null()
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH NULL\n");
	#endif

	write_ZZxx(C_PUSH_MISC, CPM_NULL);
}


void CODE_push_void_string()
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH VOID STRING\n");
	#endif

	write_ZZxx(C_PUSH_MISC, CPM_STRING);
}



/*
static bool change_last_call(ushort flag)
{
	ushort *last_code = get_last_code();

	if (!last_code)
		return FALSE;

	if ((*last_code & 0xFF00) == C_CALL)
	{
		*last_code = *last_code | flag;
		return TRUE;
	}
	else if ((ushort)((*last_code) & 0xFF00) >= (ushort)CODE_FIRST_SUBR)
	{
		*last_code = *last_code | flag;
		return TRUE;
	}
	else if (((*last_code & 0xFF00) == C_DROP) && flag == CODE_CALL_DROP)

	return FALSE;
}
*/

void CODE_dup(void)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("DUP\n");
	#endif

	write_short(C_DUP);
}


void CODE_return(int return_value)
{
	LAST_CODE;

	if (return_value == 1)
		use_stack(-1);
	
	write_ZZxx(C_RETURN, return_value);

	#ifdef DEBUG
	printf("RETURN (%d)\n", return_value);
	#endif
}


#ifdef PROJECT_COMP

void CODE_quit(bool ret)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("QUIT (%d)\n", ret);
	#endif
	
	if (ret)
		use_stack(-1);

	write_ZZxx(C_QUIT, ret ? 3 : 0);
}


void CODE_stop(void)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("STOP\n");
	#endif

	write_ZZxx(C_QUIT, 1);
}

#endif /* PROJECT_COMP */


void CODE_push_char(uchar car)
{
	LAST_CODE;

	use_stack(1);
	write_ZZxx(C_PUSH_CHAR, car);

	#ifdef DEBUG
	printf("PUSH CHAR %d\n", car);
	#endif
}


void CODE_push_void(void)
{
	LAST_CODE;

	use_stack(1);
	write_ZZxx(C_PUSH_MISC, CPM_VOID);

	#ifdef DEBUG
	printf("PUSH VOID\n");
	#endif
}


#ifdef PROJECT_COMP

void CODE_stop_event(void)
{
	LAST_CODE;

	write_ZZxx(C_QUIT, 2);

	#ifdef DEBUG
	printf("STOP EVENT\n");
	#endif
}

#endif


void CODE_subr(short subr, short nparam, short optype, bool fixed)
{
	LAST_CODE;

	use_stack(1 - nparam);

	#ifdef DEBUG
	printf("SUBR %d (%d)\n", subr, nparam);
	#endif

	if (optype == 0)
	{
		if (fixed)
			nparam = 0;
	}
	else
	{
		nparam = optype;
	}

	subr += CODE_FIRST_SUBR;
	write_short(((subr & 0xFF) << 8) | (nparam & 0xFF));
}


/*void CODE_subr_output(short subr, short nparam, int output)
{
	LAST_CODE;

	use_stack(output - nparam);

	#ifdef DEBUG
	printf("SUBR OUTPUT %d %d (%d)\n", output, subr, nparam);
	#endif

	subr += CODE_FIRST_SUBR;
	write_short(((subr & 0xFF) << 8) | (nparam & 0xFF));
}*/


void CODE_call(short nparam)
{
	LAST_CODE;

	use_stack(-nparam);

	#ifdef DEBUG
	printf("CALL ( %d )\n", nparam);
	#endif

	write_ZZxx(C_CALL, nparam);
}

void CODE_byref(uint64_t byref)
{
	LAST_CODE;
	int n;

	#ifdef DEBUG
	printf("BYREF\n");
	#endif

	if (byref >> 48)
		n = 3;
	else if (byref >> 32)
		n = 2;
	else if (byref >> 16)
		n = 1;
	else
		n = 0;
		
	write_ZZxx(C_BYREF, n);
	while (n >= 0)
	{
		write_short(byref & 0xFFFF);
		byref >>= 16;
		n--;
	}  
}

void CODE_call_byref(short nparam, uint64_t byref)
{
	LAST_CODE;
	int i, n;

	use_stack(-nparam);

	n = 0;
	for (i = 0; i < nparam; i++)
	{
		if (byref & (1ULL << i))
			n++;
	}
	use_stack(n);

	#ifdef DEBUG
	printf("CALL ( %d )\n", nparam);
	#endif

	write_ZZxx(C_CALL, nparam);
	CODE_byref(byref);
}


/*void CODE_push_return(void)
{
	LAST_CODE;

	use_stack(1);
	write_short(C_PUSH_RETURN);

	#ifdef DEBUG
	printf("PUSH RETURN\n");
	#endif
}*/


#ifdef PROJECT_COMP

void CODE_try(void)
{
	LAST_CODE;

	write_short(C_TRY);
	write_short(0);

	#ifdef DEBUG
	printf("TRY\n");
	#endif
}


void CODE_end_try(void)
{
	LAST_CODE;

	write_short(C_END_TRY);

	#ifdef DEBUG
	printf("END TRY\n");
	#endif
}


void CODE_catch(void)
{
	LAST_CODE;

	write_short(C_CATCH);

	#ifdef DEBUG
	printf("CATCH\n");
	#endif
}

#endif


void CODE_drop(void)
{
	//ushort *last_code = get_last_code();
	//ushort subr;

	use_stack(-1);

	#ifdef DEBUG
	printf("DROP\n");
	#endif

	/*if (last_code)
	{
		switch(*last_code & 0xFF00)
		{
			case C_DROP:
				*last_code = (*last_code & 0xFF00) + (*last_code & 0xFF) + 1;
				return;

			case C_CALL:
				*last_code |= CODE_CALL_VOID;
				return;

			default:
				subr = (*last_code) >> 8;
				if (subr >= CODE_FIRST_SUBR && subr <= CODE_LAST_SUBR && (!(*last_code & CODE_CALL_VOID)))
				{
					*last_code |= CODE_CALL_VOID;
					return;
				}
		}
	}*/

	//THROW("Internal compiler error: Bad stack drop!");
	
	LAST_CODE;

	write_ZZxx(C_DROP, 1);
}


/*void CODE_push_special(short spec)
{
	LAST_CODE;

	use_stack(0);

	#ifdef DEBUG
	printf("PUSH SPECIAL %d\n", spec);
	#endif

	write_ZZxx(C_PUSH_SPECIAL, spec);
}
*/

#ifdef PROJECT_COMP

void CODE_push_event(short event)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH EVENT %d\n", event);
	#endif

	write_ZZxx(C_PUSH_EVENT, event);
}

void CODE_push_extern(short index)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH EXTERN %d\n", index);
	#endif

	write_ZZxx(C_PUSH_EXTERN, index);
}

void CODE_new(ushort nparam, bool array, bool event)
{
	LAST_CODE;

	use_stack(1 - nparam);

	#ifdef DEBUG
	printf("NEW %s (%d)\n", (array ? "ARRAY" : (event ? "EVENT" : "")), nparam);
	#endif

	if (array)
		nparam |= CODE_NEW_ARRAY;

	if (event)
		nparam |= CODE_NEW_EVENT;

	write_ZZxx(C_NEW, nparam);
}

#endif

void CODE_push_boolean(bool value)
{
	LAST_CODE;

	use_stack(1);
	write_ZZxx(C_PUSH_MISC, value ? CPM_TRUE : CPM_FALSE);

	#ifdef DEBUG
	printf("PUSH %s\n", value ? "TRUE" : "FALSE");
	#endif
}

void CODE_push_inf(bool neg)
{
	LAST_CODE;

	use_stack(1);

	#ifdef DEBUG
	printf("PUSH %cINF\n", neg ? '-' : '+');
	#endif

	write_ZZxx(C_PUSH_MISC, neg ? CPM_MINF : CPM_PINF);
}

void CODE_push_complex(void)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("PUSH COMPLEX\n");
	#endif

	write_ZZxx(C_PUSH_MISC, CPM_COMPLEX);
}

void CODE_push_vargs(void)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("PUSH VARGS\n");
	#endif

	write_ZZxx(C_PUSH_MISC, CPM_VARGS);
}

void CODE_drop_vargs(void)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("DROP VARGS\n");
	#endif

	write_ZZxx(C_PUSH_MISC, CPM_DROP_VARGS);
}

#ifdef CODE_DUMP

void CODE_dump(PCODE *code, int count)
{
	int i;

	printf("\n");

	for (i = 0; i < count;)
		i += PCODE_dump(stdout, i, &code[i]);

	printf("\n");

}

#endif

void CODE_string_add(void)
{
	LAST_CODE;

	#ifdef DEBUG
	printf("STOP\n");
	#endif

	write_ZZxx(C_QUIT, 3);
}

