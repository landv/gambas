/***************************************************************************

	gbc_trans.h

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

#ifndef __GBC_TRANS_H
#define __GBC_TRANS_H

#include "gbc_type.h"
#include "gb_reserved.h"
#include "gbc_read.h"
#include "gb_limit.h"

#include "gbc_trans_common.h"

enum {
	TT_NOTHING = 0,
	TT_DO_NOT_CHECK_AS = 1,
	TT_CAN_EMBED = 2,
	TT_CAN_ARRAY = 4,
	TT_CAN_NEW = 8
	};

enum {
	TS_MODE_READ = 1,
	TS_MODE_WRITE = 2,
	TS_MODE_APPEND = 4,
	TS_MODE_CREATE = 8,
	TS_MODE_DIRECT = 16,
	TS_MODE_LINE = 32,
	TS_MODE_WATCH = 64,
	TS_MODE_PIPE = 128,
	TS_MODE_MEMORY = 256,
	TS_MODE_STRING = 512
	};

enum {
	TS_EXEC_NONE = 0,
	TS_EXEC_READ = 1,
	TS_EXEC_WRITE = 2,
	TS_EXEC_TERM = 4,
	TS_EXEC_STRING = 8,
	TS_EXEC_WAIT = 16
	};

enum {
	TS_SUBR_PRINT,
	TS_SUBR_INPUT,
	TS_SUBR_WRITE,
	TS_SUBR_WRITE_BYTES,
	TS_SUBR_READ,
	TS_SUBR_READ_BYTES,
	TS_SUBR_OPEN,
	TS_SUBR_CLOSE,
	TS_SUBR_SEEK,
	TS_SUBR_LINE_INPUT,
	TS_SUBR_FLUSH,
	TS_SUBR_EXEC,
	TS_SUBR_SHELL,
	TS_SUBR_WAIT,
	TS_SUBR_KILL,
	TS_SUBR_MOVE,
	TS_SUBR_MKDIR,
	TS_SUBR_RMDIR,
	TS_SUBR_ARRAY,
	TS_SUBR_COLLECTION,
	TS_SUBR_COPY,
	TS_SUBR_LINK,
	TS_SUBR_ERROR,
	TS_SUBR_LOCK,
	TS_SUBR_UNLOCK,
	TS_SUBR_LOCK_WAIT,
	TS_SUBR_INPUT_FROM,
	TS_SUBR_OUTPUT_TO,
	TS_SUBR_DEBUG,
	TS_SUBR_SLEEP,
	TS_SUBR_RANDOMIZE,
	TS_SUBR_ERROR_TO,
	TS_SUBR_LEFT,
	TS_SUBR_MID,
	TS_SUBR_OPEN_MEMORY,
	TS_SUBR_CHMOD,
	TS_SUBR_CHOWN,
	TS_SUBR_CHGRP,
	TS_SUBR_USE,
	TS_SUBR_CHECK_EXEC,
	};

enum {
	TSO_SUBR_SCAN
	};

enum {
	TS_NONE = -1,
	TS_STDIN = 0,
	TS_STDOUT = 1,
	TS_STDERR = 2
	};

#define TS_NO_SUBR ((void (*)())-1)

#ifndef __GBC_TRANS_C
EXTERN int TRANS_in_affectation;
EXTERN bool TRANS_in_try;
//EXTERN int TRANS_in_expression;
#endif

#define TRANS_newline() (PATTERN_is_newline(*JOB->current) ? JOB->line = PATTERN_index(*JOB->current) + 1, JOB->current++, TRUE : FALSE)

void TRANS_reset(void);
/*PUBLIC bool TRANS_type(bool check_as, bool square, bool array, bool new, TRANS_DECL *result);*/
bool TRANS_type(int flag, TRANS_DECL *result);
bool TRANS_get_number(int index, TRANS_NUMBER *result);
bool TRANS_check_declaration(void);
PATTERN *TRANS_get_constant_value(TRANS_DECL *decl, PATTERN *current);

void TRANS_want(int reserved, char *msg);
void TRANS_want_newline(void);
void TRANS_ignore(int reserved);
//int TRANS_get_class(PATTERN pattern);
bool TRANS_is_end_function(bool is_proc, PATTERN *look);
char *TRANS_get_num_desc(int num);

#define TRANS_is(_reserved) (PATTERN_is(*JOB->current, (_reserved)) ? JOB->current++, TRUE : FALSE)

/* trans_code.c */

void TRANS_code(void);
#define TRANS_has_init_var(_decl) ((_decl)->is_new || (_decl)->init)
bool TRANS_init_var(TRANS_DECL *decl);
void TRANS_statement(void);
void TRANS_init_optional(TRANS_PARAM *param);

/* trans_expr.c */

void TRANS_expression(bool check);
void TRANS_ignore_expression();
void TRANS_reference(void);
bool TRANS_affectation(bool check);
void TRANS_operation(short op, short nparam, bool output, PATTERN previous);
void TRANS_new(void);
TYPE TRANS_variable_get_type(void);
void TRANS_class(int index);
bool TRANS_string(PATTERN pattern);

/* trans_tree.c */

#define RS_UNARY (-1)

//TRANS_TREE *TRANS_tree(bool check_statement);
void TRANS_tree(bool check_statement, TRANS_TREE **result, int *count);

/* trans_ctrl.c */

void TRANS_control_init(void);
void TRANS_control_exit(void);

void TRANS_if(void);
void TRANS_else(void);
void TRANS_endif(void);
void TRANS_goto(void);
void TRANS_gosub(void);
void TRANS_on_goto_gosub(void);
void TRANS_do(int type);
void TRANS_loop(int type);
void TRANS_select(void);
void TRANS_case(void);
void TRANS_default(void);
void TRANS_end_select(void);
void TRANS_break(void);
void TRANS_continue(void);
void TRANS_return(void);
void TRANS_for(void);
void TRANS_for_each(void);
void TRANS_next(void);
void TRANS_try(void);
void TRANS_finally(void);
void TRANS_catch(void);
void TRANS_label(void);
void TRANS_with(void);
void TRANS_use_with(void);
void TRANS_end_with(void);
void TRANS_raise(void);
void TRANS_stop(void);

/* trans_subr.c */

void TRANS_subr(int subr, int nparam);

void TRANS_print(void);
void TRANS_input(void);
void TRANS_read(void);
void TRANS_read_old(void);
void TRANS_write(void);
void TRANS_open(void);
void TRANS_pipe(void);
void TRANS_memory(void);
void TRANS_open_string(void);
void TRANS_close(void);
void TRANS_lock(void);
void TRANS_unlock(void);
void TRANS_seek(void);
void TRANS_line_input(void);
void TRANS_flush(void);
void TRANS_quit(void);
void TRANS_exec(void);
void TRANS_shell(void);
void TRANS_wait(void);
void TRANS_sleep(void);
void TRANS_kill(void);
void TRANS_move(void);
void TRANS_chmod(void);
void TRANS_chown(void);
void TRANS_chgrp(void);
void TRANS_inc(void);
void TRANS_dec(void);
void TRANS_swap(void);
void TRANS_mkdir(void);
void TRANS_rmdir(void);
void TRANS_use(void);
void TRANS_copy(void);
void TRANS_link(void);
void TRANS_input_from(void);
void TRANS_output_to(void);
void TRANS_debug(void);
void TRANS_error(void);
void TRANS_scan(void);
void TRANS_randomize(void);
void TRANS_mid(void);
void TRANS_use(void);

#endif

