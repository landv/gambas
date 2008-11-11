/***************************************************************************

  trans.h

  Code translation

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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
  TT_CAN_SQUARE = 2,
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
  TS_MODE_PIPE = 128
  };

enum {
  TS_EXEC_NONE = 0,
  TS_EXEC_READ = 1,
  TS_EXEC_WRITE = 2,
  TS_EXEC_TERM = 4,
  TS_EXEC_STRING = 8
  };

enum {
  TS_SUBR_PRINT,
  TS_SUBR_INPUT,
  TS_SUBR_WRITE,
  TS_SUBR_READ,
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
  TS_SUBR_INPUT_FROM,
  TS_SUBR_OUTPUT_TO,
  TS_SUBR_DEBUG,
  TS_SUBR_SLEEP,
  TS_SUBR_RANDOMIZE,
  TS_SUBR_ERROR_TO,
  TS_SUBR_LEFT,
  TS_SUBR_MID,
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
#endif


PUBLIC void TRANS_reset(void);
PUBLIC boolean TRANS_newline(void);
/*PUBLIC boolean TRANS_type(boolean check_as, boolean square, boolean array, boolean new, TRANS_DECL *result);*/
PUBLIC boolean TRANS_type(int flag, TRANS_DECL *result);
PUBLIC boolean TRANS_get_number(int index, TRANS_NUMBER *result);
PUBLIC boolean TRANS_check_declaration(void);
PUBLIC PATTERN *TRANS_get_constant_value(TRANS_DECL *decl, PATTERN *current);

PUBLIC void TRANS_want(int reserved, char *msg);
PUBLIC boolean TRANS_is(int reserved);
PUBLIC void TRANS_ignore(int reserved);
PUBLIC int TRANS_get_class(PATTERN pattern);
PUBLIC bool TRANS_is_end_function(bool is_proc, PATTERN *look);
char *TRANS_get_num_desc(int num);

/* trans_code.c */

PUBLIC void TRANS_code(void);
PUBLIC boolean TRANS_init_var(TRANS_DECL *decl);
PUBLIC void TRANS_statement(void);
PUBLIC void TRANS_init_optional(TRANS_PARAM *param);

/* trans_expr.c */

PUBLIC void TRANS_expression(bool check);
PUBLIC void TRANS_ignore_expression();
PUBLIC void TRANS_reference(void);
PUBLIC boolean TRANS_affectation(bool check);
PUBLIC void TRANS_operation(short op, short nparam, bool output, PATTERN previous);
PUBLIC void TRANS_new(void);

/* trans_tree.c */

#define RS_UNARY (-1)

PUBLIC TRANS_TREE *TRANS_tree(bool check_statement);

/* trans_ctrl.c */

PUBLIC void TRANS_control_init(void);
PUBLIC void TRANS_control_exit(void);

PUBLIC void TRANS_if(void);
PUBLIC void TRANS_else(void);
PUBLIC void TRANS_endif(void);
PUBLIC void TRANS_goto(void);
PUBLIC void TRANS_do(PATTERN type);
PUBLIC void TRANS_loop(PATTERN type);
PUBLIC void TRANS_select(void);
PUBLIC void TRANS_case(void);
PUBLIC void TRANS_default(void);
PUBLIC void TRANS_end_select(void);
PUBLIC void TRANS_break(void);
PUBLIC void TRANS_continue(void);
PUBLIC void TRANS_return(void);
PUBLIC void TRANS_for(void);
PUBLIC void TRANS_for_each(void);
PUBLIC void TRANS_next(void);
PUBLIC void TRANS_try(void);
PUBLIC void TRANS_finally(void);
PUBLIC void TRANS_catch(void);
PUBLIC void TRANS_label(void);
PUBLIC void TRANS_with(void);
PUBLIC void TRANS_use_with(void);
PUBLIC void TRANS_end_with(void);
PUBLIC void TRANS_raise(void);

/* trans_subr.c */

PUBLIC void TRANS_subr(int subr, int nparam);

PUBLIC void TRANS_print(void);
PUBLIC void TRANS_input(void);
PUBLIC void TRANS_read(void);
PUBLIC void TRANS_write(void);
PUBLIC void TRANS_open(void);
PUBLIC void TRANS_pipe(void);
PUBLIC void TRANS_close(void);
PUBLIC void TRANS_lock(void);
PUBLIC void TRANS_unlock(void);
PUBLIC void TRANS_seek(void);
PUBLIC void TRANS_line_input(void);
PUBLIC void TRANS_flush(void);
PUBLIC void TRANS_stop(void);
PUBLIC void TRANS_quit(void);
PUBLIC void TRANS_exec(void);
PUBLIC void TRANS_shell(void);
PUBLIC void TRANS_wait(void);
PUBLIC void TRANS_sleep(void);
PUBLIC void TRANS_kill(void);
PUBLIC void TRANS_move(void);
PUBLIC void TRANS_inc(void);
PUBLIC void TRANS_dec(void);
PUBLIC void TRANS_swap(void);
PUBLIC void TRANS_mkdir(void);
PUBLIC void TRANS_rmdir(void);
PUBLIC void TRANS_use(void);
PUBLIC void TRANS_copy(void);
PUBLIC void TRANS_link(void);
PUBLIC void TRANS_input_from(void);
PUBLIC void TRANS_output_to(void);
PUBLIC void TRANS_debug(void);
PUBLIC void TRANS_error(void);
PUBLIC void TRANS_scan(void);
PUBLIC void TRANS_randomize(void);
PUBLIC void TRANS_mid(void);

#endif

