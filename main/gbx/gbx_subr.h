/***************************************************************************

  gbx_subr.h

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
  int NPARAM = code & 0x3F; \
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

#define SUBR_check_string(_value) (TYPE_is_string((_value)->type) ? ((_value)->_string.len == 0) : SUBR_check_string_real(_value))

#define VOID_STRING(_value) \
do { \
	RELEASE_STRING(_value); \
	STRING_void_value(_value); \
} while(0);

bool SUBR_check_string_real(VALUE *param);
void SUBR_check_integer(VALUE *param);
void SUBR_check_float(VALUE *param);

int SUBR_get_integer(VALUE *param);
double SUBR_get_float(VALUE *param);
void *SUBR_get_pointer(VALUE *param);
void *SUBR_get_pointer_or_string(VALUE *param);

char *SUBR_get_string(VALUE *param);
//char *SUBR_copy_string(VALUE *param);
void SUBR_get_string_len(VALUE *param, char **str, int *len);

bool SUBR_get_boolean(VALUE *param);

TYPE SUBR_get_type(VALUE *param);
TYPE SUBR_check_good_type(VALUE *param, int count);

/* subr_math.c */

//void SUBR_add(ushort code);
//void SUBR_sub(ushort code);
//void SUBR_mul(ushort code);
//void SUBR_div(ushort code);
void SUBR_quo(ushort code);
void SUBR_rem(ushort code);
void SUBR_pow(ushort code);

void SUBR_and_(ushort code);
void SUBR_not(ushort code);

void SUBR_neg(ushort code);
void SUBR_int(ushort code);
void SUBR_abs(ushort code);
void SUBR_fix(ushort code);
void SUBR_sgn(ushort code);
void SUBR_pi(ushort code);
void SUBR_math(ushort code);
void SUBR_math2(ushort code);

void SUBR_randomize(ushort code);
void SUBR_rnd(ushort code);
void SUBR_round(ushort code);

void SUBR_isnan(ushort code);

/* subr_string.c */

void SUBR_cat(ushort code);
void SUBR_file(ushort code);
//void SUBR_left(void);
//void SUBR_right(void);
//void SUBR_mid(void);
//void SUBR_len(void);
void SUBR_trim(ushort code);
void SUBR_space(void);
void SUBR_string(void);
void SUBR_upper(ushort code);
void SUBR_lower(void);
void SUBR_chr(void);
void SUBR_asc(ushort code);
void SUBR_instr(ushort code);
void SUBR_like(ushort code);
void SUBR_scan(void);
void SUBR_subst(ushort code);
void SUBR_replace(ushort code);
void SUBR_split(ushort code);
void SUBR_iconv(void);
void SUBR_sconv(ushort code);
void SUBR_is_chr(ushort code);
void SUBR_tr(void);
void SUBR_quote(ushort code);
void SUBR_unquote(ushort code);
void SUBR_swap(ushort code);

/* subr_test.c */

//void SUBR_comp(ushort code);
//void SUBR_compn(ushort code);
//void SUBR_compi(ushort code);
void SUBR_case(ushort code);
void SUBR_bit(ushort code);
void SUBR_min_max(ushort code);
void SUBR_if(ushort code);
void SUBR_choose(ushort code);
void SUBR_near(void);
void SUBR_strcomp(ushort code);
void SUBR_is(ushort code);

/* subr_conv.c */

void SUBR_is_type(ushort code);
void SUBR_conv(ushort code);
void SUBR_type(ushort code);
void SUBR_str(void);
void SUBR_val(void);
void SUBR_format(ushort code);
void SUBR_hex_bin(ushort code);

/* subr_time.c */

void SUBR_timer(void);
void SUBR_now(void);
void SUBR_year(ushort code);
void SUBR_time(ushort code);
void SUBR_date(ushort code);
void SUBR_date_op(ushort code);
void SUBR_week(ushort code);

/* subr_file.c */

void SUBR_open(ushort code);
void SUBR_close(void);
void SUBR_print(ushort code);
void SUBR_linput(void);
void SUBR_eof(ushort code);
void SUBR_lof(ushort code);
void SUBR_seek(ushort code);
void SUBR_input(ushort code);
void SUBR_read(ushort code);
void SUBR_write(ushort code);
void SUBR_flush(void);
void SUBR_lock(ushort code);
void SUBR_inp_out(ushort code);

void SUBR_stat(ushort code);
void SUBR_exist(ushort code);
void SUBR_dir(ushort code);
void SUBR_kill(ushort code);
void SUBR_mkdir(ushort code);
void SUBR_rmdir(ushort code);
void SUBR_move(ushort code);
void SUBR_link(ushort code);
void SUBR_temp(ushort code);
void SUBR_isdir(void);
void SUBR_access(ushort code);
void SUBR_rdir(ushort code);
void SUBR_dfree();

void SUBR_exit_inp_out(void);
#define SUBR_exit SUBR_exit_inp_out

/* subr_extern.c */

void SUBR_alloc(ushort code);
void SUBR_free(void);
void SUBR_realloc(ushort code);
void SUBR_strptr(ushort code);
void SUBR_varptr(ushort code);
void SUBR_ptr(ushort code);
void SUBR_make(ushort code);

/* subr_misc.c */

void SUBR_error(void);
void SUBR_shell(void);
void SUBR_wait(ushort code);
void SUBR_sleep(ushort code);
void SUBR_exec(ushort code);
void SUBR_eval(ushort code);
void SUBR_array(ushort code);
void SUBR_collection(ushort code);
void SUBR_debug(void);

void EVAL_string(char *expr);

#endif
