/***************************************************************************

  trans_subr.c

  Subroutine synonymous compiler

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
    { ".Print" }, { ".Input" }, { ".Write" }, { ".Read" },  { ".Open" },
    { ".Close" }, { "Seek" }, { ".LineInput" }, { ".Flush" }, { ".Exec" },
    { ".Shell" }, { ".Wait" }, { ".Kill" }, { ".Move" }, { ".Mkdir" },
    { ".Rmdir" }, { ".Array" }, { ".Copy" }, { ".Link" }, { ".Error" },
    { ".Lock" }, { ".Unlock" }, { ".InputFrom" }, { ".OutputTo" }, { ".Debug" },
    { ".Sleep" }, { ".Randomize" }, { ".ErrorTo" }
  };

  TRANS_SUBR_INFO *tsi = &subr_info[subr];

  if (tsi->info == NULL)
  {
    tsi->info = SUBR_get(tsi->name);
    if (!tsi->info)
    	ERROR_panic("Unknown intern subroutine: %s", tsi->name);
  }

  CODE_subr(tsi->info->opcode, nparam, tsi->info->optype, FALSE, tsi->info->min_param == tsi->info->max_param);
}


static bool trans_stream(int default_stream)
{
  if (TRANS_is(RS_SHARP))
  {
    TRANS_expression(FALSE);

    if (PATTERN_is(*JOB->current, RS_COMMA))
    {
      JOB->current++;
      if (PATTERN_is_newline(*JOB->current))
        THROW(E_SYNTAX);
    }

    return FALSE;
  }
  else
  {
    if (default_stream == TS_NONE)
      THROW("Syntax error. '#' expected");

    CODE_push_number(default_stream);
    return TRUE;
  }
}


static void trans_print_debug(void)
{
  int nparam = 1;
  boolean semicolon = FALSE;

  for(;;)
  {
    if (PATTERN_is_newline(*JOB->current))
      break;

    TRANS_expression(FALSE);
    nparam++;
    semicolon = FALSE;

    if (PATTERN_is_newline(*JOB->current))
      break;

    if (TRANS_is(RS_SCOLON))
    {
      if (TRANS_is(RS_SCOLON))
      {
        CODE_push_char(' ');
        nparam++;
      }
      semicolon = TRUE;
    }
    else if (TRANS_is(RS_COMMA))
    {
      CODE_push_char('\t');
      nparam++;
      semicolon = FALSE;
    }
    else
      THROW(E_SYNTAX);
  }

  if (!semicolon)
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


void TRANS_write(void)
{
  int nparam = 1;

  trans_stream(TS_STDOUT);

  TRANS_expression(FALSE);
  nparam++;

  if (TRANS_is(RS_COMMA))
  {
    TRANS_expression(FALSE);
    nparam++;
  }

  trans_subr(TS_SUBR_WRITE, nparam);
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
  boolean stream = TRUE;

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


void TRANS_read(void)
{
  int nparam = 2;
  PATTERN *save_return;
  PATTERN *save_current;

  trans_stream(TS_STDIN);

  save_return = JOB->current;

  TRANS_expression(FALSE);
  /*trans_calc_type();*/

  if (TRANS_is(RS_COMMA))
  {
    TRANS_expression(FALSE);
    nparam++;
  }

  trans_subr(TS_SUBR_READ, nparam);

  save_current = JOB->current;
  JOB->current = save_return;
  TRANS_reference();

  JOB->current = save_current;
}


void TRANS_open(void)
{
  int mode = TS_MODE_READ;

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

  if (TRANS_in_affectation == 0)
  {
    //CODE_drop();

    /* handle du fichier */

    TRANS_want(RS_AS, NULL);
    TRANS_ignore(RS_SHARP);

    TRANS_reference();
  }
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
    else if (TRANS_is(RS_INPUT))
      mode |= TS_MODE_READ;

    if (TRANS_is(RS_WRITE))
      mode |= TS_MODE_WRITE | TS_MODE_DIRECT;
    else if (TRANS_is(RS_OUTPUT))
      mode |= TS_MODE_WRITE;

    /*if (TRANS_is(RS_CREATE))
      mode |= TS_MODE_CREATE;
    else if (TRANS_is(RS_APPEND))
      mode |= TS_MODE_APPEND;*/

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

  CODE_push_number(mode | TS_MODE_PIPE);

  trans_subr(TS_SUBR_OPEN, 2);

  if (TRANS_in_affectation == 0)
  {
    //CODE_drop();

    /* handle du fichier */

    TRANS_want(RS_AS, NULL);
    TRANS_ignore(RS_SHARP);

    TRANS_reference();
  }
}


void TRANS_close(void)
{
  if (PATTERN_is_newline(*JOB->current))
    THROW(E_SYNTAX);

  TRANS_ignore(RS_SHARP);
  TRANS_expression(FALSE);

  trans_subr(TS_SUBR_CLOSE, 1);
  CODE_drop();
}


void TRANS_lock(void)
{
  if (PATTERN_is_newline(*JOB->current))
    THROW(E_SYNTAX);
  
	if (!TRANS_in_affectation)
		THROW("Useless LOCK");
		
  TRANS_expression(FALSE);
  
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


void TRANS_stop(void)
{
  if (TRANS_is(RS_EVENT))
    CODE_stop_event();
  else
    CODE_stop();
}


void TRANS_quit(void)
{
  CODE_quit();
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
  bool as = FALSE;

  /* programme �ex�uter */
  TRANS_expression(FALSE);

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
    as = TRUE;
  }
  else if (TRANS_is(RS_TO))
  {
    if (TRANS_in_affectation)
      THROW("Syntax error. Cannot use this syntax in affectation.");

    mode = TS_EXEC_STRING;
    wait = TRUE;
  }

  CODE_push_boolean(wait);
  CODE_push_number(mode);

	if (as && TRANS_is(RS_AS))
		TRANS_expression(FALSE);
	else
		CODE_push_null();

  trans_subr(shell ? TS_SUBR_SHELL : TS_SUBR_EXEC, 4);

  if (mode == TS_EXEC_STRING)
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

void TRANS_inc(void)
{
  PATTERN *save = JOB->current;

  TRANS_expression(FALSE);
  CODE_push_number(1);
  CODE_op(C_ADD, 2, TRUE);

  JOB->current = save;
  TRANS_reference();
}

void TRANS_dec(void)
{
  PATTERN *save = JOB->current;

  TRANS_expression(FALSE);
  CODE_push_number(1);
  CODE_op(C_SUB, 2, TRUE);

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

  TRANS_want(RS_COMMA, "comma");

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


