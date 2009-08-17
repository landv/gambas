/***************************************************************************

  gbx_subr.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_SUBR_H
#define __GBX_SUBR_H

#include "gb_error.h"
#include "gbx_exec.h"


typedef
  void (*SUBR_FUNC)(VALUE *);

typedef
  double (*MATH_FUNC)(double);

typedef
  double (*MATH_FUNC_2)(double, double);

/*
#ifndef __GBX_SUBR_C
EXTERN int NPARAM;
#endif
*/

#define SUBR_ENTER() \
  int NPARAM = EXEC_code & 0x3F; \
  VALUE *PARAM = (SP - NPARAM)

#define SUBR_GET_PARAM(nparam) \
  VALUE *PARAM = (SP - nparam);

#define SUBR_ENTER_PARAM(nparam) \
  const int NPARAM = nparam; \
  SUBR_GET_PARAM(NPARAM);

#define RETURN RP

#define SUBR_LEAVE() SUBR_leave(NPARAM)

/*  BORROW(RP); \
  SUBR_leave_void(NPARAM); \
  *SP++ = *RP; \
  RP->type = T_VOID;*/

#define SUBR_LEAVE_VOID() SUBR_leave_void(NPARAM);
  /*SP->type = T_VOID; \
  SP++;*/


/* Common routines */

void SUBR_leave(int nparam);
void SUBR_leave_void(int nparam);

bool SUBR_check_string(VALUE *param);
void SUBR_check_integer(VALUE *param);
void SUBR_check_float(VALUE *param);

int SUBR_get_integer(VALUE *param);
double SUBR_get_float(VALUE *param);
void *SUBR_get_pointer(VALUE *param);

char *SUBR_get_string(VALUE *param);
char *SUBR_copy_string(VALUE *param);
void SUBR_get_string_len(VALUE *param, char **str, int *len);

/* subr_math.c */

void SUBR_add_(void);
void SUBR_quo(void);
void SUBR_rem(void);
void SUBR_pow(void);

void SUBR_and_(void);
void SUBR_not(void);

void SUBR_neg_(void);
void SUBR_sgn(void);
void SUBR_pi(void);
void SUBR_math(void);
void SUBR_math2(void);

void SUBR_randomize(void);
void SUBR_rnd(void);
void SUBR_round(void);

/* subr_string.c */

void SUBR_cat(void);
void SUBR_file(void);
//void SUBR_left(void);
//void SUBR_right(void);
//void SUBR_mid(void);
//void SUBR_len(void);
void SUBR_trim(void);
void SUBR_space(void);
void SUBR_string(void);
void SUBR_upper(void);
void SUBR_lower(void);
void SUBR_chr(void);
void SUBR_asc(void);
void SUBR_instr(void);
void SUBR_like(void);
void SUBR_scan(void);
void SUBR_subst(void);
void SUBR_replace(void);
void SUBR_split(void);
void SUBR_iconv(void);
void SUBR_sconv(void);
void SUBR_is_chr(void);
void SUBR_tr(void);
void SUBR_quote(void);
void SUBR_unquote(void);

/* subr_test.c */

void SUBR_comp(void);
void SUBR_case(void);
void SUBR_bit(void);
void SUBR_min_max(void);
void SUBR_if(void);
void SUBR_choose(void);
void SUBR_near(void);
void SUBR_strcomp(void);
void SUBR_is(void);

/* subr_conv.c */

void SUBR_is_type(void);
void SUBR_conv(void);
void SUBR_type(void);
void SUBR_str(void);
void SUBR_val(void);
void SUBR_format(void);
void SUBR_hex(void);
void SUBR_bin(void);

/* subr_time.c */

void SUBR_timer(void);
void SUBR_now(void);
void SUBR_year(void);
void SUBR_time(void);
void SUBR_date(void);
void SUBR_date_op(void);
void SUBR_week(void);

/* subr_file.c */

void SUBR_open(void);
void SUBR_close(void);
void SUBR_print(void);
void SUBR_linput(void);
void SUBR_eof(void);
void SUBR_lof(void);
void SUBR_seek(void);
void SUBR_input(void);
void SUBR_read(void);
void SUBR_write(void);
void SUBR_flush(void);
void SUBR_lock(void);
void SUBR_inp_out(void);

void SUBR_stat(void);
void SUBR_exist(void);
void SUBR_dir(void);
void SUBR_kill(void);
void SUBR_mkdir(void);
void SUBR_rmdir(void);
void SUBR_rename(void);
void SUBR_copy(void);
void SUBR_temp(void);
void SUBR_isdir(void);
void SUBR_access(void);
void SUBR_link(void);
void SUBR_rdir(void);
void SUBR_dfree();

void SUBR_exit_inp_out(void);
#define SUBR_exit SUBR_exit_inp_out

/* subr_extern.c */

void SUBR_alloc(void);
void SUBR_free(void);
void SUBR_realloc(void);
void SUBR_strptr(void);
void SUBR_varptr(void);

/* subr_misc.c */

void SUBR_error(void);
void SUBR_shell(void);
void SUBR_wait(void);
void SUBR_sleep(void);
void SUBR_exec(void);
void SUBR_eval(void);
void SUBR_array(void);
void SUBR_collection(void);
void SUBR_debug(void);

void EVAL_string(char *expr);

#endif
