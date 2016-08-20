/***************************************************************************

  gbc_trans_subr.c

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

#define _TRANS_SUBR_C

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


typedef
	struct {
		char *name;
		SUBR_INFO *info;
		}
	TRANS_SUBR_INFO;


static void trans_subr(int subr, int nparam)
{
	static TRANS_SUBR_INFO subr_info[] =
	{
		{ ".Print" }, { ".Input" }, { ".Write" }, { ".WriteBytes" }, { ".Read" }, 
		{ ".ReadBytes" }, { ".Open" },  { ".Close" }, { "Seek" }, { ".LineInput" }, 
		{ ".Flush" }, { ".Exec" }, { ".Shell" }, { ".Wait" }, { ".Kill" }, 
		{ ".Move" }, { ".Mkdir" }, { ".Rmdir" }, { ".Array" }, {".Collection" }, 
		{ ".Copy" }, { ".Link" },  { ".Error" }, { ".Lock" }, { ".Unlock" }, 
		{ ".LockWait" }, { ".InputFrom" }, { ".OutputTo" }, { ".Debug" }, { ".Sleep" },
		{ ".Randomize" }, { ".ErrorTo" }, { "Left" }, { "Mid" }, { ".OpenMemory" },
		{ ".Chmod" }, { ".Chown" }, { ".Chgrp" }, { ".Use" }, { ".CheckExec" }
	};

	TRANS_SUBR_INFO *tsi = &subr_info[subr];

	if (tsi->info == NULL)
	{
		tsi->info = SUBR_get(tsi->name);
		if (!tsi->info)
			ERROR_panic("Unknown intern subroutine: %s", tsi->name);
	}

	if (subr == TS_SUBR_ARRAY && nparam > MAX_PARAM_OP)
		CODE_subr(tsi->info->opcode, MAX_PARAM_OP + 1, CODE_CALL_VARIANT + MAX_PARAM_OP, FALSE);
	else if (subr == TS_SUBR_COLLECTION && nparam > MAX_PARAM_OP)
		CODE_subr(tsi->info->opcode, MAX_PARAM_OP, CODE_CALL_VARIANT + MAX_PARAM_OP - 1, FALSE);
	else
		CODE_subr(tsi->info->opcode, nparam, tsi->info->optype, tsi->info->min_param == tsi->info->max_param);
}


static bool trans_stream_check(int default_stream, bool check)
{
	if (TRANS_is(RS_SHARP) || default_stream == TS_NONE)
	{
		TRANS_expression(FALSE);
		
		if (check)
		{
			if (PATTERN_is(*JOB->current, RS_COMMA))
			{
				JOB->current++;
				if (PATTERN_is_newline(*JOB->current))
					THROW(E_SYNTAX);
			}
			else
			{
				if (!PATTERN_is_newline(*JOB->current))
					THROW(E_SYNTAX);
			}
		}
		
		return FALSE;
	}
	else
	{
		//if (default_stream == TS_NONE)
		//	THROW("Syntax error. &1 expected", "'#'");

		CODE_push_number(default_stream);
		return TRUE;
	}
}

#define trans_stream(_default_stream) trans_stream_check(_default_stream, TRUE)
#define trans_stream_no_check(_default_stream) trans_stream_check(_default_stream, FALSE)

static void trans_print_debug()
{
	int nparam = 1;
	bool semicolon_or_comma = FALSE;

	for(;;)
	{
		if (PATTERN_is_newline(*JOB->current))
			break;

		TRANS_expression(FALSE);
		nparam++;
		semicolon_or_comma = FALSE;

		if (PATTERN_is_newline(*JOB->current))
			break;

		if (TRANS_is(RS_SCOLON))
		{
			if (TRANS_is(RS_SCOLON))
			{
				CODE_push_char(' ');
				nparam++;
			}
			semicolon_or_comma = TRUE;
		}
		else if (TRANS_is(RS_COMMA))
		{
			CODE_push_char('\t');
			nparam++;
			semicolon_or_comma = TRUE;
		}
		else
			THROW(E_SYNTAX);
	}

	if (!semicolon_or_comma)
	{
		CODE_push_char('\n');
		nparam++;
	}

	trans_subr(TS_SUBR_PRINT, nparam);
	CODE_drop();
}


void TRANS_print(void)
{
	trans_stream(TS_STDOUT);
	trans_print_debug();
}


void TRANS_debug(void)
{
	if (!JOB->debug)
	{
		while (!PATTERN_is_newline(*JOB->current))
			JOB->current++;
		return;
	}

	trans_subr(TS_SUBR_DEBUG, 0);
	CODE_drop();
	CODE_push_number(2); // stderr
	trans_print_debug();
}


void TRANS_error(void)
{
	if (TRANS_is(RS_TO))
	{
		if (TRANS_is(RS_DEFAULT))
			CODE_push_null();
		else
			trans_stream(TS_NONE);
	
		trans_subr(TS_SUBR_ERROR_TO, 1);
	
		if (TRANS_in_affectation == 0)
			CODE_drop();
	}
	else
	{
		CODE_push_number(2); // stderr
		trans_print_debug();
	}
}

static int trans_binary_type(void)
{
	int index;
	int nparam = 0;

	if (PATTERN_is_class(*JOB->current))
	{
		index = CLASS_add_class(JOB->class, PATTERN_index(*JOB->current));
		if (PATTERN_is(JOB->current[1], RS_LSQR))
			index = CLASS_get_array_class(JOB->class, T_OBJECT, index);
		
		CODE_push_class(index);
	}
	else if (PATTERN_is_type(*JOB->current))
	{
		if (PATTERN_is(JOB->current[1], RS_LSQR))
		{
			index = CLASS_get_array_class(JOB->class, RES_get_type(PATTERN_index(*JOB->current)), -1);
			CODE_push_class(index);
		}
		else
		{
			index = RES_get_type(PATTERN_index(*JOB->current));
			CODE_push_number(index);
		}
	}
	else
		THROW(E_SYNTAX);

	JOB->current++;
	nparam++;

	#if 0
	if (TRANS_is(RS_LSQR))
	{
		TRANS_expression(FALSE);
		nparam++;
		TRANS_want(RS_RSQR, NULL);
	}
	else if (TRANS_is(RS_STAR))
	{
		if (!string)
			THROW("Syntax error");
		TRANS_expression(FALSE);
		nparam++;
	}
	#endif
	
	return nparam;
}


void TRANS_write(void)
{
	trans_stream(TS_STDOUT);

	TRANS_expression(FALSE);
	
	if (TRANS_is(RS_AS))
	{
		trans_binary_type();
		trans_subr(TS_SUBR_WRITE, 3);
	}
	else 
	{
		if (TRANS_is(RS_COMMA))
			TRANS_expression(FALSE);
		else
		{
			if (JOB->no_old_read_syntax)
				THROW("Syntax error. &1 expected", "AS");
			CODE_push_number(-1);
		}
		
		trans_subr(TS_SUBR_WRITE_BYTES, 3);
	}
	
	CODE_drop();
}


void TRANS_output_to()
{
	TRANS_want(RS_TO, NULL);

	if (TRANS_is(RS_DEFAULT))
		CODE_push_null();
	else
		trans_stream(TS_NONE);

	trans_subr(TS_SUBR_OUTPUT_TO, 1);

	if (TRANS_in_affectation == 0)
		CODE_drop();
}

void TRANS_input_from()
{
	if (TRANS_is(RS_DEFAULT))
		CODE_push_null();
	else
		trans_stream(TS_NONE);

	trans_subr(TS_SUBR_INPUT_FROM, 1);

	if (TRANS_in_affectation == 0)
		CODE_drop();
}


void TRANS_input(void)
{
	bool stream = TRUE;

	if (TRANS_is(RS_FROM))
	{
		TRANS_input_from();
		return;
	}

	trans_stream(TS_STDIN);

	for(;;)
	{
		trans_subr(TS_SUBR_INPUT, (stream ? 1 : 0));
		stream = FALSE;

		TRANS_reference();

		if (PATTERN_is_newline(*JOB->current))
			break;

		if (!PATTERN_is(*JOB->current, RS_COMMA)
				&& !PATTERN_is(*JOB->current, RS_SCOLON))
			THROW(E_SYNTAX);

		JOB->current++;
	}
}

void TRANS_read_old(void)
{
	PATTERN *save_var;
	PATTERN *save_current;
	TYPE type;
		
	if (JOB->no_old_read_syntax)
		THROW(E_UNEXPECTED, "READ");
	
	trans_stream(TS_STDIN);
	
	save_var = JOB->current;
	type = TRANS_variable_get_type();
	
	if (TRANS_is(RS_COMMA))
	{
		TRANS_expression(FALSE);
		trans_subr(TS_SUBR_READ_BYTES, 2);
	}
	else
	{
		int id = TYPE_get_id(type);
		
		CODE_push_number(id);
		trans_subr(TS_SUBR_READ, 2);
	}
	
	save_current = JOB->current;
	JOB->current = save_var;
	TRANS_reference();
	JOB->current = save_current;
}

void TRANS_read(void)
{
	bool def = trans_stream_no_check(TS_STDIN);

	if (TRANS_is(RS_AS))
	{
		trans_binary_type();
		trans_subr(TS_SUBR_READ, 2);
	}
	else
	{
		if (!def)
			TRANS_want(RS_COMMA, NULL);
		
		TRANS_expression(FALSE);
		trans_subr(TS_SUBR_READ_BYTES, 2);
	}
}

void TRANS_open(void)
{
	int mode = TS_MODE_READ;

	if (TRANS_is(RS_PIPE))
	{
		TRANS_pipe();
		return;
	}
	else if (TRANS_is(RS_MEMORY))
	{
		TRANS_memory();
		return;
	}
	else if (TRANS_is(RS_STRING))
	{
		TRANS_open_string();
		return;
	}


	/* Nom du fichier */

	TRANS_expression(FALSE);

	/* mode d'ouverture */

	if (TRANS_is(RS_FOR))
	{
		if (TRANS_is(RS_READ))
			mode |= TS_MODE_READ | TS_MODE_DIRECT;
		else if (TRANS_is(RS_INPUT))
			mode |= TS_MODE_READ;

		if (TRANS_is(RS_WRITE))
			mode |= TS_MODE_WRITE | TS_MODE_DIRECT;
		else if (TRANS_is(RS_OUTPUT))
			mode |= TS_MODE_WRITE;

		if (TRANS_is(RS_CREATE))
			mode |= TS_MODE_CREATE;
		else if (TRANS_is(RS_APPEND))
			mode |= TS_MODE_APPEND;

		//if (TRANS_is(RS_DIRECT))
		//  mode |= TS_MODE_DIRECT;

		if (TRANS_is(RS_WATCH))
			mode |= TS_MODE_WATCH;

		/*if (TRANS_is(RS_BIG))
			mode |= TS_MODE_BIG;
		else if (TRANS_is(RS_LITTLE))
			mode |= TS_MODE_LITTLE;*/

		/*JOB->current--;
		if (PATTERN_is(*JOB->current, RS_FOR))
			THROW("Syntax error in file open mode");
		JOB->current++;*/
	}

	CODE_push_number(mode);

	trans_subr(TS_SUBR_OPEN, 2);

	/*if (TRANS_in_affectation == 0)
	{
		//CODE_drop();

		TRANS_want(RS_AS, NULL);
		TRANS_ignore(RS_SHARP);

		TRANS_reference();
	}*/
}


void TRANS_pipe(void)
{
	int mode = TS_MODE_READ;

	/* Nom du fichier */

	TRANS_expression(FALSE);

	/* mode d'ouverture */

	if (TRANS_is(RS_FOR))
	{
		if (TRANS_is(RS_READ))
			mode |= TS_MODE_READ | TS_MODE_DIRECT;
		//else if (TRANS_is(RS_INPUT))
		//  mode |= TS_MODE_READ;

		if (TRANS_is(RS_WRITE))
			mode |= TS_MODE_WRITE | TS_MODE_DIRECT;
		//else if (TRANS_is(RS_OUTPUT))
		//  mode |= TS_MODE_WRITE;

		if (TRANS_is(RS_WATCH))
			mode |= TS_MODE_WATCH;
	}

	CODE_push_number(mode | TS_MODE_PIPE);

	trans_subr(TS_SUBR_OPEN, 2);
}


void TRANS_memory(void)
{
	int mode = TS_MODE_READ;

	/* Memory address */

	TRANS_expression(FALSE);

	/* Open mode */

	if (TRANS_is(RS_FOR))
	{
		if (TRANS_is(RS_READ))
			mode |= TS_MODE_READ | TS_MODE_DIRECT;

		if (TRANS_is(RS_WRITE))
			mode |= TS_MODE_WRITE | TS_MODE_DIRECT;
	}

	CODE_push_number(mode);

	trans_subr(TS_SUBR_OPEN_MEMORY, 2);
}


void TRANS_open_string(void)
{
	int mode = TS_MODE_READ;

	/* Nom du fichier */

	if (!PATTERN_is(*JOB->current, RS_FOR))
		TRANS_expression(FALSE);
	else
		CODE_push_null();

	/* mode d'ouverture */

	if (TRANS_is(RS_FOR))
	{
		if (TRANS_is(RS_READ))
			mode |= TS_MODE_READ | TS_MODE_DIRECT;

		if (TRANS_is(RS_WRITE))
			mode |= TS_MODE_WRITE | TS_MODE_DIRECT;
	}

	CODE_push_number(mode | TS_MODE_STRING);

	trans_subr(TS_SUBR_OPEN, 2);
}


void TRANS_close(void)
{
	if (PATTERN_is_newline(*JOB->current))
		THROW(E_SYNTAX);

	TRANS_ignore(RS_SHARP);
	TRANS_expression(FALSE);

	trans_subr(TS_SUBR_CLOSE, 1);

	if (TRANS_in_affectation == 0)
		CODE_drop();
}


void TRANS_lock(void)
{
	if (PATTERN_is_newline(*JOB->current))
		THROW(E_SYNTAX);
	
	if (!TRANS_in_affectation)
		THROW("Useless LOCK");
		
	TRANS_expression(FALSE);

	if (TRANS_is(RS_WAIT))
	{
		TRANS_expression(FALSE);
		trans_subr(TS_SUBR_LOCK_WAIT, 2);
	}
	else
		trans_subr(TS_SUBR_LOCK, 1);
}


void TRANS_unlock(void)
{
	if (PATTERN_is_newline(*JOB->current))
		THROW(E_SYNTAX);

	TRANS_ignore(RS_SHARP);
	TRANS_expression(FALSE);

	trans_subr(TS_SUBR_UNLOCK, 1);
	CODE_drop();
}


void TRANS_seek(void)
{
	int nparam;

	trans_stream(TS_NONE);

	TRANS_expression(FALSE);
	nparam = 2;

	/*
	if (TRANS_is(RS_COMMA))
	{
		TRANS_expression(FALSE);
		nparam++;
	}
	*/

	trans_subr(TS_SUBR_SEEK, nparam);
	CODE_drop();
}


void TRANS_line_input(void)
{
	if (TRANS_is(RS_INPUT))
	{
		trans_stream(TS_STDIN);
		trans_subr(TS_SUBR_LINE_INPUT, 1);
		TRANS_reference();
	}
	else
		THROW(E_SYNTAX);
}


void TRANS_flush(void)
{
	trans_stream(TS_STDOUT);
	trans_subr(TS_SUBR_FLUSH, 1);
	CODE_drop();
}


void TRANS_quit(void)
{
	if (PATTERN_is_newline(*JOB->current))
	{
		CODE_quit(FALSE);
	}
	else
	{
		TRANS_expression(FALSE);
		CODE_quit(TRUE);
	}
	
}

void TRANS_randomize(void)
{
	if (PATTERN_is_newline(*JOB->current))
	{
		trans_subr(TS_SUBR_RANDOMIZE, 0);
	}
	else
	{
		TRANS_expression(FALSE);
		trans_subr(TS_SUBR_RANDOMIZE, 1);		
	}
	
	CODE_drop();
}

static void trans_exec_shell(bool shell)
{
	int mode = TS_EXEC_NONE;
	bool wait;
	bool as = TRUE;

	TRANS_expression(FALSE);

	if (TRANS_is(RS_WITH))
		TRANS_expression(FALSE);
	else
		CODE_push_null();
	
	wait = TRANS_is(RS_WAIT);

	if (TRANS_is(RS_FOR))
	{
		if (TRANS_is(RS_READ))
			mode |= TS_EXEC_READ;
		if (TRANS_is(RS_WRITE))
			mode |= TS_EXEC_WRITE;

		if (mode == 0)
		{
			mode |= TS_EXEC_TERM;
			if (TRANS_is(RS_INPUT))
				mode |= TS_EXEC_READ;
			if (TRANS_is(RS_OUTPUT))
				mode |= TS_EXEC_WRITE;
		}
	}
	else if (TRANS_is(RS_TO))
	{
		if (TRANS_in_affectation)
			THROW("Syntax error. Cannot use this syntax in assignment");

		mode = TS_EXEC_STRING;
		wait = TRUE;
		as = FALSE;
	}
	
	if (wait) mode |= TS_EXEC_WAIT;
	CODE_push_number(mode);

	if (as && TRANS_is(RS_AS))
		TRANS_expression(FALSE);
	else
		CODE_push_null();

	trans_subr(shell ? TS_SUBR_SHELL : TS_SUBR_EXEC, 4);

	if (mode & TS_EXEC_STRING)
		TRANS_reference();
	else if (TRANS_in_affectation == 0)
		CODE_drop();
}


void TRANS_exec(void)
{
	trans_exec_shell(FALSE);
}

void TRANS_shell(void)
{
	trans_exec_shell(TRUE);
}


void TRANS_wait(void)
{
	int nparam = 0;

	if (!PATTERN_is_newline(*JOB->current))
	{
		TRANS_expression(FALSE);
		nparam = 1;
	}

	trans_subr(TS_SUBR_WAIT, nparam);
	CODE_drop();
}


void TRANS_sleep(void)
{
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_SLEEP, 1);
	CODE_drop();
}


void TRANS_kill(void)
{
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_KILL, 1);
	CODE_drop();
}

void TRANS_move(void)
{
	TRANS_expression(FALSE);
	TRANS_want(RS_TO, NULL);
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_MOVE, 2);
	CODE_drop();
}

void TRANS_copy(void)
{
	TRANS_expression(FALSE);
	TRANS_want(RS_TO, NULL);
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_COPY, 2);
	CODE_drop();
}

void TRANS_link(void)
{
	TRANS_expression(FALSE);
	TRANS_want(RS_TO, NULL);
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_LINK, 2);
	CODE_drop();
}

void TRANS_chmod(void)
{
	TRANS_expression(FALSE);
	TRANS_want(RS_TO, NULL);
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_CHMOD, 2);
	CODE_drop();
}

void TRANS_chown(void)
{
	TRANS_expression(FALSE);
	TRANS_want(RS_TO, NULL);
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_CHOWN, 2);
	CODE_drop();
}

void TRANS_chgrp(void)
{
	TRANS_expression(FALSE);
	TRANS_want(RS_TO, NULL);
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_CHGRP, 2);
	CODE_drop();
}

void TRANS_inc(void)
{
	PATTERN *save = JOB->current;

	TRANS_expression(FALSE);
	CODE_push_number(1);
	CODE_op(C_ADD, 0, 2, TRUE);

	JOB->current = save;
	TRANS_reference();
}

void TRANS_dec(void)
{
	PATTERN *save = JOB->current;

	TRANS_expression(FALSE);
	CODE_push_number(1);
	CODE_op(C_SUB, 0, 2, TRUE);

	JOB->current = save;
	TRANS_reference();
}


void TRANS_swap(void)
{
	PATTERN *sa;
	PATTERN *sb;
	PATTERN *current;

	sa = JOB->current;
	TRANS_expression(FALSE);

	TRANS_want(RS_COMMA, "Comma");

	sb = JOB->current;
	TRANS_expression(FALSE);

	current = JOB->current;

	JOB->current = sa;
	TRANS_reference();

	JOB->current = sb;
	TRANS_reference();

	JOB->current = current;
}

void TRANS_mkdir(void)
{
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_MKDIR, 1);
	CODE_drop();
}

void TRANS_rmdir(void)
{
	TRANS_expression(FALSE);
	trans_subr(TS_SUBR_RMDIR, 1);
	CODE_drop();
}

void TRANS_mid()
{
	PATTERN *str;
	PATTERN *pos;
	PATTERN *len;
	PATTERN *save;
	
	TRANS_want(RS_LBRA, "Left bracket");
	
	str = JOB->current;
	TRANS_expression(FALSE);
	
	TRANS_want(RS_COMMA, "Comma");
	
	pos = JOB->current;
	TRANS_expression(FALSE);
	CODE_push_number(1);
	CODE_op(C_SUB, 0, 2, TRUE);
	TRANS_subr(TS_SUBR_LEFT, 2);
	
	if (TRANS_is(RS_COMMA))
	{
		len = JOB->current;
		TRANS_ignore_expression();
	}
	else
	{
		len = NULL;
	}
	
	TRANS_want(RS_RBRA, "Right bracket");
	TRANS_want(RS_EQUAL, "Equal");
	
	TRANS_expression(FALSE);
	
	save = JOB->current;
	
	if (len)
	{
		JOB->current = str;
		TRANS_expression(FALSE);
		JOB->current = pos;
		TRANS_expression(FALSE);
		JOB->current = len;
		TRANS_expression(FALSE);
		CODE_op(C_ADD, 0, 2, TRUE);
		TRANS_subr(TS_SUBR_MID, 2);
	}
	
	CODE_op(C_CAT, 0, len ? 3 : 2, FALSE);
	
	JOB->current = str;
	TRANS_reference();
	
	JOB->current = save;
}



#if 0
void TRANS_scan(void)
{
	PATTERN *save;
	int noutput = 0;

	TRANS_expression(FALSE);
	TRANS_want(RS_WITH, NULL);
	TRANS_expression(FALSE);

	TRANS_want(RS_TO, NULL);

	save = JOB->current;
	for(;;)
	{
		TRANS_expression(FALSE);
		noutput++;
		if (!TRANS_is(RS_COMMA))
			break;
	}
	JOB->current = save;

	trans_subr_output(TSO_SUBR_SCAN, 2 + noutput, noutput);

	for(;;)
	{
		TRANS_reference();
		if (!TRANS_is(RS_COMMA))
			break;
	}
}
#endif

void TRANS_subr(int subr, int nparam)
{
	trans_subr(subr, nparam);
}

