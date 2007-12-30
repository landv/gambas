/***************************************************************************

  subr.h

  The subroutines

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

PUBLIC void SUBR_leave(int nparam);
PUBLIC void SUBR_leave_void(int nparam);

PUBLIC boolean SUBR_check_string(VALUE *param);
PUBLIC void SUBR_check_integer(VALUE *param);
PUBLIC void SUBR_check_float(VALUE *param);

PUBLIC long SUBR_get_integer(VALUE *param);
PUBLIC double SUBR_get_float(VALUE *param);

PUBLIC char *SUBR_get_string(VALUE *param);
PUBLIC char *SUBR_copy_string(VALUE *param);
PUBLIC void SUBR_get_string_len(VALUE *param, char **str, long *len);

/* subr_math.c */

PUBLIC void SUBR_add_(void);
PUBLIC void SUBR_quo(void);
PUBLIC void SUBR_rem(void);
PUBLIC void SUBR_pow(void);

PUBLIC void SUBR_and_(void);
PUBLIC void SUBR_not(void);

PUBLIC void SUBR_neg_(void);
PUBLIC void SUBR_sgn(void);
PUBLIC void SUBR_pi(void);
PUBLIC void SUBR_math(void);
PUBLIC void SUBR_math2(void);

PUBLIC void SUBR_randomize(void);
PUBLIC void SUBR_rnd(void);
PUBLIC void SUBR_round(void);

PUBLIC void SUBR_add_quick(long value);

/* subr_string.c */

PUBLIC void SUBR_cat(void);
PUBLIC void SUBR_file(void);
PUBLIC void SUBR_left(void);
PUBLIC void SUBR_right(void);
PUBLIC void SUBR_mid(void);
PUBLIC void SUBR_len(void);
PUBLIC void SUBR_trim(void);
PUBLIC void SUBR_space(void);
PUBLIC void SUBR_string(void);
PUBLIC void SUBR_upper(void);
PUBLIC void SUBR_lower(void);
PUBLIC void SUBR_chr(void);
PUBLIC void SUBR_asc(void);
PUBLIC void SUBR_instr(void);
PUBLIC void SUBR_like(void);
PUBLIC void SUBR_scan(void);
PUBLIC void SUBR_subst(void);
PUBLIC void SUBR_replace(void);
PUBLIC void SUBR_split(void);
PUBLIC void SUBR_iconv(void);
PUBLIC void SUBR_sconv(void);
PUBLIC void SUBR_is_chr(void);

/* subr_test.c */

PUBLIC void SUBR_comp(void);
PUBLIC void SUBR_case(void);
PUBLIC void SUBR_bit(void);
PUBLIC void SUBR_min_max(void);
PUBLIC void SUBR_if(void);
PUBLIC void SUBR_choose(void);
PUBLIC void SUBR_near(void);
PUBLIC void SUBR_strcomp(void);
PUBLIC void SUBR_is(void);

/* subr_conv.c */

PUBLIC void SUBR_is_type(void);
PUBLIC void SUBR_conv(void);
PUBLIC void SUBR_type(void);
PUBLIC void SUBR_str(void);
PUBLIC void SUBR_val(void);
PUBLIC void SUBR_format(void);
PUBLIC void SUBR_hex(void);
PUBLIC void SUBR_bin(void);

/* subr_time.c */

PUBLIC void SUBR_timer(void);
PUBLIC void SUBR_now(void);
PUBLIC void SUBR_year(void);
PUBLIC void SUBR_time(void);
PUBLIC void SUBR_date(void);
PUBLIC void SUBR_date_op(void);
PUBLIC void SUBR_week(void);

/* subr_file.c */

PUBLIC void SUBR_open(void);
PUBLIC void SUBR_close(void);
PUBLIC void SUBR_print(void);
PUBLIC void SUBR_linput(void);
PUBLIC void SUBR_eof(void);
PUBLIC void SUBR_lof(void);
PUBLIC void SUBR_seek(void);
PUBLIC void SUBR_input(void);
PUBLIC void SUBR_read(void);
PUBLIC void SUBR_write(void);
PUBLIC void SUBR_flush(void);
PUBLIC void SUBR_lock(void);
PUBLIC void SUBR_inp_out(void);

PUBLIC void SUBR_stat(void);
PUBLIC void SUBR_exist(void);
PUBLIC void SUBR_dir(void);
PUBLIC void SUBR_kill(void);
PUBLIC void SUBR_mkdir(void);
PUBLIC void SUBR_rmdir(void);
PUBLIC void SUBR_rename(void);
PUBLIC void SUBR_copy(void);
PUBLIC void SUBR_temp(void);
PUBLIC void SUBR_isdir(void);
PUBLIC void SUBR_access(void);
PUBLIC void SUBR_link(void);
PUBLIC void SUBR_rdir(void);
PUBLIC void SUBR_dfree();

PUBLIC void SUBR_exit_inp_out(void);
#define SUBR_exit SUBR_exit_inp_out

/* subr_extern.c */

PUBLIC void SUBR_alloc(void);
PUBLIC void SUBR_free(void);
PUBLIC void SUBR_realloc(void);
PUBLIC void SUBR_strptr(void);

/* subr_misc.c */

PUBLIC void SUBR_error(void);
PUBLIC void SUBR_shell(void);
PUBLIC void SUBR_wait(void);
PUBLIC void SUBR_sleep(void);
PUBLIC void SUBR_exec(void);
PUBLIC void SUBR_eval(void);
PUBLIC void SUBR_array(void);
PUBLIC void SUBR_debug(void);

#endif
