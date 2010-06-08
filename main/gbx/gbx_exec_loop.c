/***************************************************************************

  gbx_exec_loop.c

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

#define __GBX_EXEC_LOOP_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbx_type.h"
#include "gbx_debug.h"

#include "gbx_subr.h"
#include "gb_pcode.h"
#include "gbx_stack.h"
#include "gbx_event.h"
#include "gbx_value.h"
#include "gbx_local.h"
#include "gbx_string.h"
#include "gbx_api.h"
#include "gbx_archive.h"
#include "gbx_extern.h"
#include "gbx_exec.h"
#include "gbx_subr.h"
#include "gbx_math.h"
#include "gbx_c_array.h"
#include "gbx_struct.h"

//#define DEBUG_PCODE 1

#if DEBUG_PCODE
#define PROJECT_EXEC
#include "gb_pcode_temp.h"
#endif

#define SUBR_beep EXEC_ILLEGAL


#define GET_XXX()   (((signed short)(code << 4)) >> 4)
#define GET_UXX()   (code & 0xFFF)
#define GET_7XX()   (code & 0x7FF)
#define GET_XX()    ((signed char)code)
#define GET_UX()    ((unsigned char)code)
#define GET_3X()    (code & 0x3F)

static void my_VALUE_class_read(CLASS *class, VALUE *value, char *addr, CTYPE ctype, void *ref);
static void my_VALUE_class_constant(CLASS *class, VALUE *value, int ind);
static void _SUBR_comp(ushort code);
static void _SUBR_compn(ushort code);
static void _SUBR_add(ushort code);
static void _SUBR_sub(ushort code);
static void _SUBR_mul(ushort code);
static void _SUBR_div(ushort code);


static void *SubrTable[] =
{
  /* 28 */  _SUBR_comp,           _SUBR_compn,          SUBR_compi,           SUBR_compi,
  /* 2C */  SUBR_compi,           SUBR_compi,           SUBR_near,            SUBR_case,
  /* 30 */  _SUBR_add,            _SUBR_sub,            _SUBR_mul,            _SUBR_div,
  /* 34 */  SUBR_neg_,            SUBR_quo,             SUBR_rem,             SUBR_pow,
  /* 38 */  SUBR_and_,            SUBR_and_,            SUBR_and_,            SUBR_not,
  /* 3C */  SUBR_cat,             SUBR_like,            SUBR_file,            SUBR_is,

  NULL,            /* 00 40 */
  NULL,            /* 01 41 */
  NULL,            /* 02 42 */
  NULL,            /* 03 43 */
  SUBR_space,      /* 04 44 */
  SUBR_string,     /* 05 45 */
  SUBR_trim,       /* 06 46 */
  SUBR_upper,      /* 07 47 */
  SUBR_lower,      /* 08 48 */
  SUBR_chr,        /* 09 49 */
  SUBR_asc,        /* 10 4A */
  SUBR_instr,      /* 11 4B */
  SUBR_instr,      /* 12 4C */
  SUBR_subst,      /* 13 4D */
  SUBR_replace,    /* 14 4E */
  SUBR_split,      /* 15 4F */
  SUBR_scan,       /* 16 50 */
  SUBR_strcomp,    /* 17 51 */
  SUBR_iconv,      /* 18 52 */
  SUBR_sconv,      /* 19 53 */
  SUBR_neg_,       /* 20 54 */
  SUBR_neg_,       /* 21 55 */
  SUBR_neg_,       /* 22 56 */
  SUBR_sgn,        /* 23 57 */
  SUBR_math,       /* 24 58 */
  SUBR_pi,         /* 25 59 */
  SUBR_round,      /* 26 5A */
  SUBR_randomize,  /* 27 5B */
  SUBR_rnd,        /* 28 5C */
  SUBR_min_max,    /* 29 5D */
  SUBR_min_max,    /* 30 5E */
  SUBR_if,         /* 31 5F */
  SUBR_choose,     /* 32 60 */
  SUBR_array,      /* 33 61 */
  SUBR_math2,      /* 34 62 */
  SUBR_is_chr,     /* 35 63 */
  SUBR_bit,        /* 36 64 */
  SUBR_is_type,    /* 37 65 */
  SUBR_type,       /* 38 66 */
  SUBR_conv,       /* 39 67 */
  SUBR_bin,        /* 40 68 */
  SUBR_hex,        /* 41 69 */
  SUBR_val,        /* 42 6A */
  SUBR_str,        /* 43 6B */
  SUBR_format,     /* 44 6C */
  SUBR_timer,      /* 45 6D */
  SUBR_now,        /* 46 6E */
  SUBR_year,       /* 47 6F */
  SUBR_week,       /* 48 70 */
  SUBR_date,       /* 49 71 */
  SUBR_time,       /* 50 72 */
  SUBR_date_op,    /* 51 73 */
  SUBR_eval,       /* 52 74 */
  SUBR_error,      /* 53 75 */
  SUBR_debug,      /* 54 76 */
  SUBR_wait,       /* 55 77 */
  SUBR_open,       /* 56 78 */
  SUBR_close,      /* 57 79 */
  SUBR_input,      /* 58 7A */
  SUBR_linput,     /* 59 7B */
  SUBR_print,      /* 60 7C */
  SUBR_read,       /* 61 7D */
  SUBR_write,      /* 62 7E */
  SUBR_flush,      /* 63 7F */
  SUBR_lock,       /* 64 80 */
  SUBR_inp_out,    /* 65 81 */
  SUBR_eof,        /* 66 82 */
  SUBR_lof,        /* 67 83 */
  SUBR_seek,       /* 68 84 */
  SUBR_kill,       /* 69 85 */
  SUBR_mkdir,      /* 70 86 */
  SUBR_rmdir,      /* 71 87 */
  SUBR_rename,     /* 72 88 */
  SUBR_copy,       /* 73 89 */
  SUBR_link,       /* 74 8A */
  SUBR_exist,      /* 75 8B */
  SUBR_access,     /* 76 8C */
  SUBR_stat,       /* 77 8D */
  SUBR_dfree,      /* 78 8E */
  SUBR_temp,       /* 79 8F */
  SUBR_isdir,      /* 80 90 */
  SUBR_dir,        /* 81 91 */
  SUBR_rdir,       /* 82 92 */
  SUBR_exec,       /* 83 93 */
  SUBR_alloc,      /* 84 94 */
  SUBR_free,       /* 85 95 */
  SUBR_realloc,    /* 86 96 */
  SUBR_strptr,     /* 87 97 */
  SUBR_sleep,      /* 88 98 */
  SUBR_varptr,     /* 89 99 */
  SUBR_collection, /* 90 9A */
  SUBR_tr,         /* 91 9B */
  SUBR_quote,      /* 92 9C */
  SUBR_unquote,    /* 93 9D */
	SUBR_eval,       /* 94 9E */
	NULL,            /* 95 9F */
};


void EXEC_loop(void)
{
  static const void *jump_table[256] =
  {
    /* 00 NOP             */  &&_NEXT,
    /* 01 PUSH LOCAL      */  &&_PUSH_LOCAL,
    /* 02 PUSH PARAM      */  &&_PUSH_PARAM,
    /* 03 PUSH ARRAY      */  &&_PUSH_ARRAY,
    /* 04 PUSH UNKNOWN    */  &&_PUSH_UNKNOWN,
    /* 05 PUSH EXTERN     */  &&_PUSH_EXTERN,
    /* 06 BYREF           */  &&_BYREF,
    /* 07 PUSH EVENT      */  &&_PUSH_EVENT,
    /* 08 QUIT            */  &&_QUIT,
    /* 09 POP LOCAL       */  &&_POP_LOCAL,
    /* 0A POP PARAM       */  &&_POP_PARAM,
    /* 0B POP ARRAY       */  &&_POP_ARRAY,
    /* 0C POP UNKNOWN     */  &&_POP_UNKNOWN,
    /* 0D POP OPTIONAL    */  &&_POP_OPTIONAL,
    /* 0E POP CTRL        */  &&_POP_CTRL,
    /* 0F BREAK           */  &&_BREAK,
    /* 10 RETURN          */  &&_RETURN,
    /* 11 PUSH SHORT      */  &&_PUSH_SHORT,
    /* 12 PUSH INTEGER    */  &&_PUSH_INTEGER,
    /* 13 PUSH CHAR       */  &&_PUSH_CHAR,
    /* 14 PUSH MISC       */  &&_PUSH_MISC,
    /* 15 PUSH ME         */  &&_PUSH_ME,
    /* 16 EVENT           */  &&_EVENT,
    /* 17 TRY             */  &&_TRY,
    /* 18 END TRY         */  &&_END_TRY,
    /* 19 CATCH           */  &&_CATCH,
    /* 1A DUP             */  &&_DUP,
    /* 1B DROP            */  &&_DROP,
    /* 1C NEW             */  &&_NEW,
    /* 1D CALL            */  &&_CALL,
    /* 1E CALL QUICK      */  &&_CALL_QUICK,
    /* 1F CALL NORM       */  &&_CALL_NORM,
    /* 20 JUMP            */  &&_JUMP,
    /* 21 JUMP IF TRUE    */  &&_JUMP_IF_TRUE,
    /* 22 JUMP IF FALSE   */  &&_JUMP_IF_FALSE,
    /* 23                 */  &&_ILLEGAL,
    /* 24 JUMP FIRST      */  &&_JUMP_FIRST,
    /* 25 JUMP NEXT       */  &&_JUMP_NEXT,
    /* 26 FIRST           */  &&_ENUM_FIRST,
    /* 27 NEXT            */  &&_ENUM_NEXT,
    /* 28 =               */  &&_SUBR_CODE,
    /* 29 <>              */  &&_SUBR_CODE,
    /* 2A >               */  &&_SUBR_CODE,
    /* 2B <=              */  &&_SUBR_CODE,
    /* 2C <               */  &&_SUBR_CODE,
    /* 2D >=              */  &&_SUBR_CODE,
    /* 2E ==              */  &&_SUBR,
    /* 2F CASE            */  &&_SUBR_CODE,
    /* 30 +               */  &&_SUBR_CODE,
    /* 31 -               */  &&_SUBR_CODE,
    /* 32 *               */  &&_SUBR_CODE,
    /* 33 /               */  &&_SUBR_CODE,
    /* 34 NEG             */  &&_SUBR_CODE,
    /* 35 \               */  &&_SUBR_CODE,
    /* 36 MOD             */  &&_SUBR_CODE,
    /* 37 ^               */  &&_SUBR,
    /* 38 AND             */  &&_SUBR_CODE,
    /* 39 OR              */  &&_SUBR_CODE,
    /* 3A XOR             */  &&_SUBR_CODE,
    /* 3B NOT             */  &&_SUBR_CODE,
    /* 3C &               */  &&_SUBR_CODE,
    /* 3D LIKE            */  &&_SUBR_CODE,
    /* 3E &/              */  &&_SUBR_CODE,
    /* 3F Is              */  &&_SUBR,
    /* 40 Left$           */  &&_SUBR_LEFT,
    /* 41 Mid$            */  &&_SUBR_MID,
    /* 42 Right$          */  &&_SUBR_RIGHT,
    /* 43 Len             */  &&_SUBR_LEN,
    /* 44 Space$          */  &&_SUBR,
    /* 45 String$         */  &&_SUBR,
    /* 46 Trim$           */  &&_SUBR_CODE,
    /* 47 UCase$          */  &&_SUBR,
    /* 48 LCase$          */  &&_SUBR,
    /* 49 Chr$            */  &&_SUBR,
    /* 4A Asc             */  &&_SUBR_CODE,
    /* 4B InStr           */  &&_SUBR_CODE,
    /* 4C RInStr          */  &&_SUBR_CODE,
    /* 4D Subst$          */  &&_SUBR_CODE,
    /* 4E Replace$        */  &&_SUBR_CODE,
    /* 4F Split           */  &&_SUBR_CODE,
    /* 50 Scan            */  &&_SUBR,
    /* 51 Comp            */  &&_SUBR_CODE,
    /* 52 Conv            */  &&_SUBR,
    /* 53 DConv           */  &&_SUBR_CODE,
    /* 54 Abs             */  &&_SUBR_CODE,
    /* 55 Int             */  &&_SUBR_CODE,
    /* 56 Fix             */  &&_SUBR_CODE,
    /* 57 Sgn             */  &&_SUBR_CODE,
    /* 58 Frac...         */  &&_SUBR_CODE,
    /* 59 Pi              */  &&_SUBR_CODE,
    /* 5A Round           */  &&_SUBR_CODE,
    /* 5B Randomize       */  &&_SUBR_CODE,
    /* 5C Rnd             */  &&_SUBR_CODE,
    /* 5D Min             */  &&_SUBR_CODE,
    /* 5E Max             */  &&_SUBR_CODE,
    /* 5F IIf             */  &&_SUBR_CODE,
    /* 60 Choose          */  &&_SUBR_CODE,
    /* 61 Array           */  &&_SUBR_CODE,
    /* 62 ATan2...        */  &&_SUBR_CODE,
    /* 63 IsAscii...      */  &&_SUBR_CODE,
    /* 64 BClr...         */  &&_SUBR_CODE,
    /* 65 IsBoolean...    */  &&_SUBR_CODE,
    /* 66 TypeOf          */  &&_SUBR_CODE,
    /* 67 CBool...        */  &&_SUBR_CODE,
    /* 68 Bin$            */  &&_SUBR_CODE,
    /* 69 Hex$            */  &&_SUBR_CODE,
    /* 6A Val             */  &&_SUBR,
    /* 6B Str             */  &&_SUBR,
    /* 6C Format          */  &&_SUBR_CODE,
    /* 6D Timer           */  &&_SUBR,
    /* 6E Now             */  &&_SUBR,
    /* 6F Year...         */  &&_SUBR_CODE,
    /* 70 Week            */  &&_SUBR_CODE,
    /* 71 Date            */  &&_SUBR_CODE,
    /* 72 Time...         */  &&_SUBR_CODE,
    /* 73 DateAdd...      */  &&_SUBR_CODE,
    /* 74 Eval            */  &&_SUBR_CODE,
    /* 75 Error           */  &&_SUBR,
    /* 76 Debug           */  &&_SUBR,
    /* 77 Wait            */  &&_SUBR_CODE,
    /* 78 Open            */  &&_SUBR_CODE,
    /* 79 Close           */  &&_SUBR,
    /* 7A Input           */  &&_SUBR_CODE,
    /* 7B LineInput       */  &&_SUBR,
    /* 7C Print           */  &&_SUBR_CODE,
    /* 7D Read            */  &&_SUBR_CODE,
    /* 7E Write           */  &&_SUBR_CODE,
    /* 7F Flush           */  &&_SUBR,
    /* 80 Lock...         */  &&_SUBR_CODE,
    /* 81 InputFrom...    */  &&_SUBR_CODE,
    /* 82 Eof             */  &&_SUBR_CODE,
    /* 83 Lof             */  &&_SUBR_CODE,
    /* 84 Seek            */  &&_SUBR_CODE,
    /* 85 Kill            */  &&_SUBR,
    /* 86 Mkdir           */  &&_SUBR,
    /* 87 Rmdir           */  &&_SUBR,
    /* 88 Move            */  &&_SUBR,
    /* 89 Copy            */  &&_SUBR,
    /* 8A Link            */  &&_SUBR,
    /* 8B Exist           */  &&_SUBR,
    /* 8C Access          */  &&_SUBR_CODE,
    /* 8D Stat            */  &&_SUBR_CODE,
    /* 8E Dfree           */  &&_SUBR,
    /* 8F Temp$           */  &&_SUBR_CODE,
    /* 90 IsDir           */  &&_SUBR,
    /* 91 Dir             */  &&_SUBR_CODE,
    /* 92 RDir            */  &&_SUBR_CODE,
    /* 93 Exec...         */  &&_SUBR_CODE,
    /* 94 Alloc           */  &&_SUBR_CODE,
    /* 95 Free            */  &&_SUBR,
    /* 96 Realloc         */  &&_SUBR_CODE,
    /* 97 StrPtr          */  &&_SUBR_CODE,
    /* 98 Sleep           */  &&_SUBR,
    /* 99 VarPtr          */  &&_SUBR,
    /* 9A Collection      */  &&_SUBR_CODE,
    /* 9B Tr$             */  &&_SUBR,
    /* 9C Quote$...       */  &&_SUBR_CODE,
    /* 9D Unquote$        */  &&_SUBR,
    /* 9E                 */  &&_SUBR,
    /* 9F                 */  &&_SUBR,
    /* A0 ADD QUICK       */  &&_ADD_QUICK,
    /* A1 ADD QUICK       */  &&_ADD_QUICK,
    /* A2 ADD QUICK       */  &&_ADD_QUICK,
    /* A3 ADD QUICK       */  &&_ADD_QUICK,
    /* A4 ADD QUICK       */  &&_ADD_QUICK,
    /* A5 ADD QUICK       */  &&_ADD_QUICK,
    /* A6 ADD QUICK       */  &&_ADD_QUICK,
    /* A7 ADD QUICK       */  &&_ADD_QUICK,
    /* A8 ADD QUICK       */  &&_ADD_QUICK,
    /* A9 ADD QUICK       */  &&_ADD_QUICK,
    /* AA ADD QUICK       */  &&_ADD_QUICK,
    /* AB ADD QUICK       */  &&_ADD_QUICK,
    /* AC ADD QUICK       */  &&_ADD_QUICK,
    /* AD ADD QUICK       */  &&_ADD_QUICK,
    /* AE ADD QUICK       */  &&_ADD_QUICK,
    /* AF ADD QUICK       */  &&_ADD_QUICK,
    /* B0 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B1 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B2 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B3 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B4 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B5 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B6 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B7 PUSH CLASS      */  &&_PUSH_CLASS,
    /* B8 PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* B9 PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* BA PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* BB PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* BC PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* BD PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* BE PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* BF PUSH FUNCTION   */  &&_PUSH_FUNCTION,
    /* C0 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C1 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C2 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C3 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C4 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C5 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C6 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C7 PUSH DYNAMIC    */  &&_PUSH_DYNAMIC,
    /* C8 PUSH STATIC     */  &&_PUSH_STATIC,
    /* C9 PUSH STATIC     */  &&_PUSH_STATIC,
    /* CA PUSH STATIC     */  &&_PUSH_STATIC,
    /* CB PUSH STATIC     */  &&_PUSH_STATIC,
    /* CC PUSH STATIC     */  &&_PUSH_STATIC,
    /* CD PUSH STATIC     */  &&_PUSH_STATIC,
    /* CE PUSH STATIC     */  &&_PUSH_STATIC,
    /* CF PUSH STATIC     */  &&_PUSH_STATIC,
    /* D0 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D1 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D2 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D3 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D4 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D5 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D6 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D7 POP DYNAMIC     */  &&_POP_DYNAMIC,
    /* D8 POP STATIC      */  &&_POP_STATIC,
    /* D9 POP STATIC      */  &&_POP_STATIC,
    /* DA POP STATIC      */  &&_POP_STATIC,
    /* DB POP STATIC      */  &&_POP_STATIC,
    /* DC POP STATIC      */  &&_POP_STATIC,
    /* DD POP STATIC      */  &&_POP_STATIC,
    /* DE POP STATIC      */  &&_POP_STATIC,
    /* DF POP STATIC      */  &&_POP_STATIC,
    /* E0 PUSH CONST      */  &&_PUSH_CONST,
    /* E1 PUSH CONST      */  &&_PUSH_CONST,
    /* E2 PUSH CONST      */  &&_PUSH_CONST,
    /* E3 PUSH CONST      */  &&_PUSH_CONST,
    /* E4 PUSH CONST      */  &&_PUSH_CONST,
    /* E5 PUSH CONST      */  &&_PUSH_CONST,
    /* E6 PUSH CONST      */  &&_PUSH_CONST,
    /* E7 PUSH CONST      */  &&_PUSH_CONST,
    /* E8 PUSH CONST      */  &&_PUSH_CONST,
    /* E9 PUSH CONST      */  &&_PUSH_CONST,
    /* EA PUSH CONST      */  &&_PUSH_CONST,
    /* EB PUSH CONST      */  &&_PUSH_CONST,
    /* EC PUSH CONST      */  &&_PUSH_CONST,
    /* ED PUSH CONST      */  &&_PUSH_CONST,
    /* EE PUSH CONST      */  &&_PUSH_CONST,
    /* EF PUSH CONST      */  &&_PUSH_CONST,
    /* F0 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F1 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F2 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F3 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F4 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F5 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F6 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F7 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F8 PUSH QUICK      */  &&_PUSH_QUICK,
    /* F9 PUSH QUICK      */  &&_PUSH_QUICK,
    /* FA PUSH QUICK      */  &&_PUSH_QUICK,
    /* FB PUSH QUICK      */  &&_PUSH_QUICK,
    /* FC PUSH QUICK      */  &&_PUSH_QUICK,
    /* FD PUSH QUICK      */  &&_PUSH_QUICK,
    /* FE PUSH QUICK      */  &&_PUSH_QUICK,
    /* FF PUSH QUICK      */  &&_PUSH_QUICK
  };

  int NO_WARNING(ind);
  ushort code;
  /*ushort uind;*/
  TYPE type;

  void _pop_ctrl(int ind)
  {
    register VALUE *val = &BP[ind];
    RELEASE(val);
    SP--;
    *val = *SP;
  }

  goto _MAIN;

/*-----------------------------------------------*/

_SUBR_CODE:

  (*(EXEC_FUNC_CODE)SubrTable[(code >> 8) - 0x28])(code);

  if (UNLIKELY(PCODE_is_void(code)))
    POP();

/*-----------------------------------------------*/

_NEXT:

  PC++;

/*-----------------------------------------------*/

_MAIN:

#if DEBUG_PCODE
    DEBUG_where();
    fprintf(stderr, "[%4d] ", (int)(intptr_t)(SP - (VALUE *)STACK_base));
    if (*PC >> 8)
      PCODE_dump(stderr, PC - FP->code, PC);
    else
    	fprintf(stderr, "\n");
    fflush(stderr);
#endif

  code = *PC;
  goto *jump_table[code >> 8];

/*-----------------------------------------------*/

_SUBR:

  (*(EXEC_FUNC)SubrTable[(code >> 8) - 0x28])();

_SUBR_END:

  if (UNLIKELY(PCODE_is_void(code)))
    POP();

  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_LOCAL:

  *SP = BP[GET_XX()];
  PUSH();

  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_PARAM:

  *SP = PP[GET_XX()];
  PUSH();

  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_ARRAY:

  EXEC_push_array(code);
  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_UNKNOWN:

  EXEC_push_unknown(code);
  goto _NEXT;

/*-----------------------------------------------*/

/*_PUSH_SPECIAL:

  EXEC_push_special();
  goto _NEXT;*/

/*-----------------------------------------------*/

_PUSH_EVENT:

  /*
    The function called by raising an event is different at each call,
    but the signature remains the same, so optimizing the next CALL
    instruction with CALL QUICK is safe.

    The only problem is when pushing a 'NULL' function, i.e. a function
    that does nothing, because there is no handler for this event.
    Then CALL QUICK must know how to handle these functions.
  */

	ind = GET_UX();

	if (CP->parent)
		ind += CP->parent->n_event;
	
  SP->type = T_FUNCTION;
	SP->_function.kind = FUNCTION_EVENT;
	SP->_function.index = ind;
	SP->_function.defined = FALSE;
	SP->_function.class = NULL;
	SP->_function.object = NULL;
  SP++;
  
  goto _NEXT;

/*-----------------------------------------------*/

_POP_LOCAL:

  {
    register VALUE *val = &BP[GET_XX()];

    VALUE_conv(&SP[-1], val->type);

    RELEASE(val);
    SP--;
    *val = *SP;
  }

  goto _NEXT;

/*-----------------------------------------------*/

_POP_PARAM:

  {
    register VALUE *val = &PP[GET_XX()];

    VALUE_conv(&SP[-1], val->type);

    RELEASE(val);
    SP--;
    *val = *SP;
  }

  goto _NEXT;

/*-----------------------------------------------*/

_POP_CTRL:

  _pop_ctrl(GET_XX());

  goto _NEXT;

/*-----------------------------------------------*/

_POP_ARRAY:

  EXEC_pop_array(code);
  goto _NEXT;

/*-----------------------------------------------*/

_POP_UNKNOWN:

  EXEC_pop_unknown();
  goto _NEXT;

/*-----------------------------------------------*/

_POP_OPTIONAL:

  /*
  if (ind >= 0)
    val = &BP[ind];
  else
    val = &PP[ind];
  */

  {
    register VALUE *val = &BP[GET_XX()];

    if (LIKELY(val->type == T_VOID))
    {
      if (SP[-1].type == T_VOID)
        VALUE_default(&SP[-1], val->_void.ptype);
      else
        VALUE_conv(&SP[-1], val->_void.ptype);
      /* RELEASE(val); Pas n�essaire */
      SP--;
      *val = *SP;
    }
    else
      POP();
  }

  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_SHORT:

  SP->type = T_INTEGER;
  PC++;
  SP->_integer.value = *((short *)PC);
  SP++;
  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_INTEGER:

  SP->type = T_INTEGER;
  PC++;
  SP->_integer.value = PC[0] | ((uint)PC[1] << 16);
  SP++;
	PC += 2;
  goto _MAIN;

/*-----------------------------------------------*/

_PUSH_CHAR:

  STRING_char_value(SP, (char)GET_UX());
  SP++;
  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_ME:

  if (UNLIKELY(GET_UX() & 1))
  {
    if (DEBUG_info->op)
    {
      SP->_object.class = DEBUG_info->cp;
      SP->_object.object = DEBUG_info->op;
    }
    else if (DEBUG_info->cp)
    {
      SP->type = T_CLASS;
      SP->_class.class = DEBUG_info->cp;
    }
    else
      SP->type = T_NULL;
  }
  else
  {
    if (LIKELY(OP != NULL))
    {
      SP->_object.class = CP;
      SP->_object.object = OP;
    }
    else
    {
      SP->type = T_CLASS;
      SP->_class.class = CP;
    }
  }

	if (UNLIKELY(GET_UX() & 2))
	{
		// The used class must be in the stack, because it is tested by exec_push && exec_pop
		if (LIKELY(OP != NULL))
		{
			SP->_object.class = SP->_object.class->parent;
  		SP->_object.super = EXEC_super;
		}
		else
		{
			SP->_class.class = SP->_class.class->parent;
			SP->_class.super = EXEC_super;
		}
		
  	EXEC_super = SP;
	}

  PUSH();
  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_MISC:

  {
    static const void *_jump[] =
      { &&__PUSH_NULL, &&__PUSH_VOID, &&__PUSH_FALSE, &&__PUSH_TRUE, &&__PUSH_LAST, &&__PUSH_STRING };

    goto *_jump[GET_UX()];

  __PUSH_NULL:

    SP->type = T_NULL;
    //SP->_integer.value = 0;
    SP++;
    goto _NEXT;

  __PUSH_VOID:

    SP->type = T_VOID;
    SP++;
    goto _NEXT;

  __PUSH_FALSE:

    SP->type = T_BOOLEAN;
    SP->_integer.value = 0;
    SP++;
    goto _NEXT;

  __PUSH_TRUE:

    SP->type = T_BOOLEAN;
    SP->_integer.value = -1;
    SP++;
    goto _NEXT;

  __PUSH_LAST:

    SP->type = T_OBJECT;
    SP->_object.object = EVENT_Last;
    PUSH();
    goto _NEXT;

  __PUSH_STRING:
    
		SP->type = T_STRING;
    SP->_string.addr = NULL;
		SP->_string.start = SP->_string.len = 0;
    PUSH();
    goto _NEXT;
	}

/*-----------------------------------------------*/

_EVENT:

  GAMBAS_StopEvent = TRUE;
  goto _NEXT;

/*-----------------------------------------------*/

/*
_PUSH_RETURN:

  *SP++ = *RP;
  goto _NEXT;
*/

/*-----------------------------------------------*/

_DUP:

  *SP = SP[-1];
  PUSH();
  goto _NEXT;

/*-----------------------------------------------*/

_DROP:

  ind = GET_3X();

  while (ind > 0)
  {
    POP();
    ind--;
  }

  goto _NEXT;

/*-----------------------------------------------*/

_NEW:

  EXEC_new();
  goto _NEXT;

/*-----------------------------------------------*/

_JUMP:

  PC += (signed short)PC[1] + 2;
  goto _MAIN;


/*-----------------------------------------------*/

_JUMP_IF_TRUE:

  VALUE_conv_boolean(&SP[-1]);
  SP--;
  if (UNLIKELY(SP->_boolean.value != 0))
    PC += (signed short)PC[1];

	PC += 2;
  goto _MAIN;

/*-----------------------------------------------*/

_JUMP_IF_FALSE:

  VALUE_conv_boolean(&SP[-1]);
  SP--;
  if (UNLIKELY(SP->_boolean.value == 0))
    PC += (signed short)PC[1];

	PC += 2;
  goto _MAIN;

/*-----------------------------------------------*/

_RETURN:

  if (LIKELY(GET_UX() != 0))
  {
		type = FP->type;
		if (UNLIKELY(TYPE_is_pure_object(type) && ((CLASS *)type)->override))
		{
			type = (TYPE)(((CLASS *)type)->override);
			//FP->type = type;
		}
		
    VALUE_conv(&SP[-1], type);
    SP--;
    *RP = *SP;
		//ERROR_clear();
    EXEC_leave(FALSE);
  }
  else
  {
    VALUE_default(RP, FP->type);
		//ERROR_clear();
    EXEC_leave(FALSE);
  }

  if (UNLIKELY(PC == NULL))
    return;

  goto _NEXT;

/*-----------------------------------------------*/

_CALL:

  {
    static const void *call_jump[] =
      { &&__CALL_NULL, &&__CALL_NATIVE, &&__CALL_PRIVATE, &&__CALL_PUBLIC,
        &&__CALL_EVENT, &&__CALL_EXTERN, &&__CALL_UNKNOWN, &&__CALL_CALL };

    register VALUE * NO_WARNING(val);

    ind = GET_3X();
    val = &SP[-(ind + 1)];

    if (UNLIKELY(!TYPE_is_function(val->type)))
    {
			bool defined = EXEC_object(val, &EXEC.class, (OBJECT **)&EXEC.object);
	    
			val->type = T_FUNCTION;
	    val->_function.kind = FUNCTION_CALL;
	    val->_function.defined = defined;
	    val->_function.class = EXEC.class;
	    val->_function.object = EXEC.object;
      //goto _CALL;
    }
		else
		{
			EXEC.class = val->_function.class;
			EXEC.object = val->_function.object;
		}

		EXEC.drop = PCODE_is_void(code);
		EXEC.nparam = ind;
		EXEC.use_stack = TRUE;

		if (UNLIKELY(!val->_function.defined))
			*PC |= CODE_CALL_VARIANT;

		goto *call_jump[(int)val->_function.kind];

  __CALL_NULL:

    while (ind > 0)
    {
      POP();
      ind--;
    }

    POP();

    if (!PCODE_is_void(code))
    {
      /*VALUE_default(SP, (TYPE)(val->_function.function));*/
      SP->type = T_NULL;
      SP++;
    }

    goto _NEXT;

  __CALL_NATIVE:

    EXEC.native = TRUE;
    EXEC.index = val->_function.index;
    EXEC.desc = &EXEC.class->table[EXEC.index].desc->method;
    //EXEC.use_stack = TRUE;

    goto __EXEC_NATIVE;

  __CALL_PRIVATE:

    EXEC.native = FALSE;
    EXEC.index = val->_function.index;

    goto __EXEC_ENTER;

  __CALL_PUBLIC:

    EXEC.native = FALSE;
    EXEC.desc = &EXEC.class->table[val->_function.index].desc->method;
    EXEC.index = (int)(intptr_t)(EXEC.desc->exec);
    EXEC.class = EXEC.desc->class;

    goto __EXEC_ENTER;

  __EXEC_ENTER:

  	EXEC_enter_check(val->_function.defined);
    goto _MAIN;

  __EXEC_NATIVE:

    EXEC_native_check(val->_function.defined);
    goto _NEXT;

  __CALL_EVENT:

		ind = GB_Raise(OP, val->_function.index, (-EXEC.nparam));

		#if 0
    EXEC.desc = &EXEC.class->table[val->_function.index].desc->method;
    EXEC.class = EXEC.desc->class;
    EXEC.native = FUNCTION_is_native(EXEC.desc);

		old_last = EVENT_Last;
    EVENT_Last = OP;

    if (EXEC.native)
    { 
      //EXEC.use_stack = TRUE;
      EXEC_native();
    }
    else
    {
      EXEC.index = (int)(EXEC.desc->exec);
      EXEC_function();
    }

		ind = GAMBAS_StopEvent ? -1 : 0;
		#endif

		POP(); // function

		if (!PCODE_is_void(code))
		{
			SP->type = T_BOOLEAN;
			SP->_boolean.value = ind ? -1 : 0;
			SP++;
		}
		
		//EVENT_Last = old_last;

    goto _NEXT;

  __CALL_UNKNOWN:

    EXEC.property = FALSE;
    EXEC.unknown = CP->load->unknown[val->_function.index];
    EXEC.desc = CLASS_get_special_desc(EXEC.class, SPEC_UNKNOWN);
    //EXEC.use_stack = TRUE;
    goto __CALL_SPEC;

  __CALL_CALL:

    EXEC.desc = CLASS_get_special_desc(EXEC.class, SPEC_CALL);
    if (UNLIKELY(!EXEC.desc && !EXEC.object && EXEC.nparam == 1 && !EXEC.class->is_virtual))
    {
    	SP[-2] = SP[-1];
    	SP--;
			VALUE_conv_object(SP - 1, (TYPE)EXEC.class);
			goto _NEXT;
    }
    
    goto __CALL_SPEC;

  __CALL_SPEC:

    if (UNLIKELY(!EXEC.desc))
    	THROW(E_NFUNC);

    EXEC.native = FUNCTION_is_native(EXEC.desc);

    if (EXEC.native)
    {
      EXEC_native();
      goto _NEXT;
    }
    else
    {
      EXEC.index = (int)(intptr_t)(EXEC.desc->exec);
      EXEC.class = EXEC.desc->class;
      EXEC_enter();
      goto _MAIN;
    }

  __CALL_EXTERN:

    EXEC.index = val->_function.index;
    EXTERN_call();
    goto _NEXT;
  }

/*-----------------------------------------------*/

_CALL_QUICK:

  {
    static const void *call_jump[] =
      { &&__CALL_NULL, &&__CALL_NATIVE_Q, &&__CALL_PRIVATE_Q, &&__CALL_PUBLIC_Q };

    register VALUE * NO_WARNING(val);

    ind = GET_3X();
    val = &SP[-(ind + 1)];

    EXEC.class = val->_function.class;
    EXEC.object = val->_function.object;
    EXEC.drop = PCODE_is_void(code);
    EXEC.nparam = ind;

    if (UNLIKELY(!val->_function.defined))
      *PC |= CODE_CALL_VARIANT;

    //if (call_jump[(int)val->_function.kind] == 0)
    //  fprintf(stderr, "val->_function.kind = %d ?\n", val->_function.kind);

    goto *call_jump[(int)val->_function.kind];

  __CALL_PRIVATE_Q:

    EXEC.native = FALSE;
    EXEC.index = val->_function.index;

    goto __EXEC_ENTER_Q;

  __CALL_PUBLIC_Q:

    EXEC.native = FALSE;
    EXEC.desc = &EXEC.class->table[val->_function.index].desc->method;
    EXEC.index = (int)(intptr_t)(EXEC.desc->exec);
    EXEC.class = EXEC.desc->class;

  __EXEC_ENTER_Q:

    EXEC_enter_quick();
    goto _MAIN;
    
  __CALL_NATIVE_Q:
  
    EXEC.native = TRUE;
    EXEC.index = val->_function.index;
    EXEC.desc = &EXEC.class->table[EXEC.index].desc->method;
    
    EXEC_native_quick();
    goto _NEXT;
  }

/*-----------------------------------------------*/

_CALL_NORM:

  {
    static const void *call_jump[] =
      { &&__CALL_NULL, &&__CALL_NATIVE_N, &&__CALL_PRIVATE_N, &&__CALL_PUBLIC_N };

    register VALUE * NO_WARNING(val);

    ind = GET_3X();
    val = &SP[-(ind + 1)];

    EXEC.class = val->_function.class;
    EXEC.object = val->_function.object;
    EXEC.drop = PCODE_is_void(code);
    EXEC.nparam = ind;
    EXEC.use_stack = TRUE;

    if (UNLIKELY(!val->_function.defined))
      *PC |= CODE_CALL_VARIANT;

    goto *call_jump[(int)val->_function.kind];

  __CALL_PRIVATE_N:

    EXEC.native = FALSE;
    EXEC.index = val->_function.index;

    goto __EXEC_ENTER_N;

  __CALL_PUBLIC_N:

    EXEC.native = FALSE;
    EXEC.desc = &EXEC.class->table[val->_function.index].desc->method;
    EXEC.index = (int)(intptr_t)(EXEC.desc->exec);
    EXEC.class = EXEC.desc->class;

  __EXEC_ENTER_N:

    EXEC_enter();
    goto _MAIN;
    
  __CALL_NATIVE_N:
    
    EXEC.native = TRUE;
    EXEC.index = val->_function.index;
    EXEC.desc = &EXEC.class->table[EXEC.index].desc->method;
    
    EXEC_native();
    goto _NEXT;
  }

/*-----------------------------------------------*/

_JUMP_FIRST:

  PC[1] &= 0xFF00;
  goto _NEXT;

/*-----------------------------------------------*/

_JUMP_NEXT:

  {
    static const void *jn_jump[] = { &&_JN_START, &&_JN_NEXT_1, &&_JN_NEXT_2, &&_JN_NEXT_3, &&_JN_NEXT_4, &&_JN_NEXT_5, &&_JN_NEXT_6 };

    VALUE * NO_WARNING(end);
    VALUE * NO_WARNING(inc);
    register VALUE * NO_WARNING(val);
    
    end = &BP[PC[-1] & 0xFF];
    inc = end + 1;
    val = &BP[PC[2] & 0xFF];

    goto *jn_jump[GET_UX()];

  _JN_START:

    type = val->type;

		// The step value must stay negative, even if the loop variable is a byte

    if (LIKELY(TYPE_is_integer(type)))
    {
    	VALUE_conv_integer(&SP[-1]);
  	}
    else
    {
    	VALUE_conv(&SP[-1], type);
		}

   	VALUE_conv(&SP[-2], type);

    ind = PC[-1] & 0xFF;

    _pop_ctrl(ind + 1); /* modifie val ! */
    _pop_ctrl(ind);

    val = &BP[PC[2] & 0xFF];

    if (LIKELY(TYPE_is_integer(type)))
    {
      if (LIKELY(inc->_integer.value > 0))
      {
        *PC |= 1;
        goto _JN_TEST_1;
      }
      else
      {
        *PC |= 2;
        goto _JN_TEST_2;
      }
    }
    else if (TYPE_is_float(type))
    {
      if (inc->_float.value > 0)
      {
        *PC |= 3;
        goto _JN_TEST_3;
      }
      else
      {
        *PC |= 4;
        goto _JN_TEST_4;
      }
    }
    else if (TYPE_is_long(type))
    {
      if (inc->_long.value > 0)
      {
        *PC |= 5;
        goto _JN_TEST_5;
      }
      else
      {
        *PC |= 6;
        goto _JN_TEST_6;
      }
    }
    else
      THROW(E_TYPE, "Number", TYPE_get_name(type));

  _JN_NEXT_1:

    val->_integer.value += inc->_integer.value;

  _JN_TEST_1:

    if (LIKELY(val->_integer.value <= end->_integer.value))
		{
			PC += 3;
      goto _MAIN;
		}

    goto _JN_END;

  _JN_NEXT_2:

    val->_integer.value += inc->_integer.value;

  _JN_TEST_2:

    if (LIKELY(val->_integer.value >= end->_integer.value))
		{
			PC += 3;
			goto _MAIN;
		}

    goto _JN_END;

  _JN_NEXT_3:

    val->_float.value += inc->_float.value;

  _JN_TEST_3:

    if (LIKELY(val->_float.value <= end->_float.value))
		{
			PC += 3;
			goto _MAIN;
		}

    goto _JN_END;

  _JN_NEXT_4:

    val->_float.value += inc->_float.value;

  _JN_TEST_4:

    if (LIKELY(val->_float.value >= end->_float.value))
		{
			PC += 3;
			goto _MAIN;
		}

    goto _JN_END;

  _JN_NEXT_5:

    val->_long.value += inc->_long.value;

  _JN_TEST_5:

    if (LIKELY(val->_long.value <= end->_long.value))
		{
			PC += 3;
			goto _MAIN;
		}

    goto _JN_END;

  _JN_NEXT_6:

    val->_long.value += inc->_long.value;

  _JN_TEST_6:

    if (LIKELY(val->_long.value >= end->_long.value))
		{
			PC += 3;
			goto _MAIN;
		}

    goto _JN_END;

  _JN_END:

    PC += (signed short)PC[1] + 2;
    goto _MAIN;
  }

/*-----------------------------------------------*/

_ENUM_FIRST:

  _pop_ctrl(GET_XX());
  EXEC_enum_first(code);
  goto _NEXT;

/*-----------------------------------------------*/

_ENUM_NEXT:

  if (UNLIKELY(EXEC_enum_next(code)))
    goto _JUMP;
  else
	{
		PC += 2;
    goto _MAIN;
	}

/*-----------------------------------------------*/

_PUSH_CLASS:

	{
		CLASS *class = CP->load->class_ref[GET_7XX()];

		//CLASS_load(class);
		//fprintf(stderr, "PUSH CLASS: %s %s\n", class->name, class->auto_create ? "AUTO CREATE" : "");

		SP->type = T_CLASS;
		SP->_class.class = class;
		SP++;

	  //fprintf(stderr, "PUSH CLASS: %s in %s\n", SP->_class.class->name, SP->_class.class->component ? SP->_class.class->component->name : NULL);
	}

  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_FUNCTION:

  /*ind = GET_7XX();*/

  SP->type = T_FUNCTION;
  SP->_function.class = CP;
  SP->_function.object = OP;
  SP->_function.kind = FUNCTION_PRIVATE;
  SP->_function.index = GET_7XX();
  SP->_function.defined = TRUE;

  OBJECT_REF(OP, "exec_loop._PUSH_FUNCTION (FUNCTION)");
  SP++;

  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_EXTERN:

  /*ind = GET_7XX();*/

  SP->type = T_FUNCTION;
  SP->_function.class = CP;
  SP->_function.object = NULL;
  SP->_function.kind = FUNCTION_EXTERN;
  SP->_function.index = GET_UX();
  SP->_function.defined = TRUE;

  //OBJECT_REF(OP, "exec_loop._PUSH_FUNCTION (FUNCTION)");
  SP++;

  goto _NEXT;

/*-----------------------------------------------*/

  {
    CLASS_VAR *var;
    char *addr;
		void *ref;

_PUSH_DYNAMIC:

    var = &CP->load->dyn[GET_7XX()];

    if (UNLIKELY(OP == NULL))
      THROW_ILLEGAL();

		ref = OP;
    addr = &OP[var->pos];
    goto __READ;

_PUSH_STATIC:

    var = &CP->load->stat[GET_7XX()];
    addr = (char *)CP->stat + var->pos;
		ref = CP;
    goto __READ;

__READ:

    my_VALUE_class_read(CP, SP, addr, var->type, ref);

    PUSH();
    goto _NEXT;


_POP_DYNAMIC:

    var = &CP->load->dyn[GET_7XX()];

    if (UNLIKELY(OP == NULL))
      THROW_ILLEGAL();

    addr = &OP[var->pos];
    goto __WRITE;

_POP_STATIC:

    var = &CP->load->stat[GET_7XX()];
    addr = (char *)CP->stat + var->pos;
    goto __WRITE;

__WRITE:

    VALUE_class_write(CP, &SP[-1], addr, var->type);
    POP();

    goto _NEXT;

  }

/*-----------------------------------------------*/

_PUSH_CONST:

	ind = GET_UXX();
  my_VALUE_class_constant(CP, SP, ind);
  SP++;
  goto _NEXT;


/*-----------------------------------------------*/

_PUSH_QUICK:

  SP->type = T_INTEGER;
  SP->_integer.value = GET_XXX();
  SP++;
  goto _NEXT;

/*-----------------------------------------------*/

_ADD_QUICK:

	{
		static void *_aq_jump[] = {
			NULL, &&__AQ_BOOLEAN, &&__AQ_BYTE, &&__AQ_SHORT, &&__AQ_INTEGER, &&__AQ_LONG, &&__AQ_FLOAT, &&__AQ_FLOAT, &&__AQ_DATE, &&__AQ_STRING, &&__AQ_STRING
			};
	
		TYPE NO_WARNING(type);
		int NO_WARNING(value);
		VALUE * NO_WARNING(P1);
		void * NO_WARNING(jump_end);
	
		P1 = SP - 1;
	
		if (UNLIKELY(TYPE_is_variant(P1->type)))
		{
			jump_end = &&__AQ_VARIANT_END;
			VARIANT_undo(P1);
		}
		else
			jump_end = &&__AQ_END;
	
		type = P1->type;
		value = GET_XXX();
	
		if (LIKELY(type <= T_CSTRING))
			goto *_aq_jump[type];
		else
			THROW(E_TYPE, "Number", TYPE_get_name(type));
	
	__AQ_BOOLEAN:
		
		fprintf(stderr, "warning: ");
		DEBUG_where();
		fprintf(stderr, "ADD QUICK with Boolean\n");
		
		P1->_integer.value ^= (value & 1) ? -1 : 0;
		goto *jump_end;
	
	__AQ_BYTE:
		
		P1->_integer.value = (unsigned char)(P1->_integer.value + value);
		goto *jump_end;
	
	__AQ_SHORT:
	
		P1->_integer.value = (short)(P1->_integer.value + value);
		goto *jump_end;
	
	__AQ_INTEGER:
	
		P1->_integer.value += value;
		goto *jump_end;
	
	__AQ_LONG:
	
		P1->_long.value += (int64_t)value;
		goto *jump_end;
	
	__AQ_DATE:
	__AQ_STRING:
		
		VALUE_conv_float(P1);
	
	__AQ_FLOAT:
	
		P1->_float.value += (double)value;
		goto *jump_end;
	
	__AQ_VARIANT_END:
	
		VALUE_conv_variant(P1);
	
	__AQ_END:
  	goto _NEXT;
	}

/*-----------------------------------------------*/

_TRY:

  EP = SP;
  ET = EC;
  EC = PC + (signed short)PC[1] + 2;

  #if DEBUG_ERROR
  fprintf(stderr, "exec TRY %p\n", EC);
  #endif

	PC += 2;
  goto _MAIN;

/*-----------------------------------------------*/

_END_TRY:

  #if DEBUG_ERROR
  fprintf(stderr, "exec END TRY %p\n", PC);
  #endif

  // If EP was reset to null, then an error occurred
 	EXEC_got_error = (EP == NULL);
  EP = NULL;
  EC = ET;
  ET = NULL;
  goto _NEXT;

/*-----------------------------------------------*/

_CATCH:

  if (UNLIKELY(EC == NULL))
    goto _NEXT;
  else
    goto _RETURN;

/*-----------------------------------------------*/

_BREAK:

  if (EXEC_debug && CP && CP->component == COMPONENT_main)
  {
    //TRACE.ec = PC + 1;
    //TRACE.ep = SP;

    TC = PC + 1;
    TP = SP;

    ind = GET_UX();

    if (ind == 0)
    {
      if (!DEBUG_info->stop)
        goto _NEXT;
			
      // Return from (void stack)
			if (DEBUG_info->leave)
			{
				if (STACK_get_current()->pc)
					goto _NEXT;
				if (FP == DEBUG_info->fp)
					goto _NEXT;
        if (BP > DEBUG_info->bp)
          goto _NEXT;
			}
			// Forward or Return From
      else if (DEBUG_info->fp != NULL)
      {
        if (BP > DEBUG_info->bp)
          goto _NEXT;
      }
      // otherwise, Next
    }

    DEBUG.Breakpoint(ind);
  }
  else
  	*PC = C_NOP;

  goto _NEXT;

/*-----------------------------------------------*/

_QUIT:

  if (GET_UX() == 0)
    EXEC_quit();

  if (EXEC_debug && CP && CP->component == COMPONENT_main)
    DEBUG.Breakpoint(0);
	//else
	//	TRACE_backtrace(stderr);

  goto _NEXT;

/*-----------------------------------------------*/

_BYREF:

	if (LIKELY(PC == FP->code))
	{
		PC += GET_UX() + 2;
		goto _MAIN;
	}

	THROW(E_BYREF);

/*-----------------------------------------------*/

_ILLEGAL:

  THROW_ILLEGAL();

/*-----------------------------------------------*/

#define EXEC_code code

_SUBR_LEFT:

	{
		int val;
	
		SUBR_ENTER();
	
		if (LIKELY(!SUBR_check_string(PARAM)))
		{
			if (NPARAM == 1)
				val = 1;
			else
			{
				VALUE_conv_integer(&PARAM[1]);
				val = PARAM[1]._integer.value;
			}
		
			if (UNLIKELY(val < 0))
				val += PARAM->_string.len;
		
			PARAM->_string.len = MinMax(val, 0, PARAM->_string.len);
		}
	
		SP -= NPARAM;
		SP++;
	}
	goto _SUBR_END;

/*-----------------------------------------------*/

_SUBR_RIGHT:
	
	{
		int val;
		int new_len;
	
		SUBR_ENTER();
	
		if (LIKELY(!SUBR_check_string(PARAM)))
		{
			if (NPARAM == 1)
				val = 1;
			else
			{
				VALUE_conv_integer(&PARAM[1]);
				val = PARAM[1]._integer.value;
			}
		
			if (UNLIKELY(val < 0))
				val += PARAM->_string.len;
		
			new_len = MinMax(val, 0, PARAM->_string.len);
		
			PARAM->_string.start += PARAM->_string.len - new_len;
			PARAM->_string.len = new_len;
		}
	
		SP -= NPARAM;
		SP++;
	}
	goto _SUBR_END;

/*-----------------------------------------------*/

_SUBR_MID:
	{
		int start;
		int len;
		bool null;
	
		SUBR_ENTER();
	
		null = SUBR_check_string(PARAM);
	
		VALUE_conv_integer(&PARAM[1]);
		start = PARAM[1]._integer.value - 1;
	
		if (UNLIKELY(start < 0))
			THROW(E_ARG);
	
		if (UNLIKELY(null))
			goto _SUBR_MID_FIN;
			
		if (UNLIKELY(start >= PARAM->_string.len))
		{
			RELEASE_STRING(PARAM);
			STRING_void_value(PARAM);
			goto _SUBR_MID_FIN;
		}
	
		if (NPARAM == 2)
			len = PARAM->_string.len;
		else
		{
			VALUE_conv_integer(&PARAM[2]);
			len = PARAM[2]._integer.value;
		}
	
		if (UNLIKELY(len < 0))
			len = Max(0, PARAM->_string.len - start + len);
	
		len = MinMax(len, 0, PARAM->_string.len - start);
	
		if (UNLIKELY(len == 0))
		{
			RELEASE_STRING(PARAM);
			PARAM->_string.addr = NULL;
			PARAM->_string.start = 0;
		}
		else
			PARAM->_string.start += start;
	
		PARAM->_string.len = len;
	
	_SUBR_MID_FIN:
	
		SP -= NPARAM;
		SP++;
	}
	goto _SUBR_END;

/*-----------------------------------------------*/

_SUBR_LEN:
	{
		int len;
	
		SUBR_GET_PARAM(1);
	
		if (UNLIKELY(SUBR_check_string(PARAM)))
			len = 0;
		else
			len = PARAM->_string.len;
	
		RELEASE_STRING(PARAM);
	
		PARAM->type = T_INTEGER;
		PARAM->_integer.value = len;
	}
	goto _SUBR_END;
	
/*-----------------------------------------------*/

#if 0
_SUBR_COMP:

	{
		static void *jump[17] = {
			&&__SC_VARIANT, &&__SC_BOOLEAN, &&__SC_BYTE, &&__SC_SHORT, &&__SC_INTEGER, &&__SC_LONG, &&__SC_SINGLE, &&__SC_FLOAT, &&__SC_DATE,
			&&__SC_STRING, &&__SC_STRING, &&__SC_POINTER, &&__SC_ERROR, &&__SC_ERROR, &&__SC_ERROR, &&__SC_NULL, &&__SC_OBJECT
			};

		static void *test[] = { &&__SC_EQ, &&__SC_NE, &&__SC_GT, &&__SC_LE, &&__SC_LT, &&__SC_GE };

		char result;
		char op;
		VALUE *P1;
		VALUE *P2;

		op = (code - C_EQ) >> 8;

		P1 = SP - 2;
		P2 = P1 + 1;

		goto *jump[code & 0x1F];

	__SC_BOOLEAN:
	__SC_BYTE:
	__SC_SHORT:
	__SC_INTEGER:

		{
			int tmp = P1->_integer.value - P2->_integer.value;
			result = tmp > 0 ? 1 : tmp < 0 ? -1 : 0;
			goto __SC_END;
		}
		
	__SC_LONG:

		VALUE_conv(P1, T_LONG);
		VALUE_conv(P2, T_LONG);

		result = llsgn(P1->_long.value - P2->_long.value);
		goto __SC_END;

	__SC_DATE:

		VALUE_conv(P1, T_DATE);
		VALUE_conv(P2, T_DATE);

		result = DATE_comp_value(P1, P2);
		goto __SC_END;

	__SC_NULL:

		if (op < 2)
		{
			if (P2->type == T_NULL)
			{
				result = !VALUE_is_null(P1);
				goto __SC_END_RELEASE;
			}
			else if (P1->type == T_NULL)
			{
				result = !VALUE_is_null(P2);
				goto __SC_END_RELEASE;
			}
		}

	__SC_STRING:

		VALUE_conv_string(P1);
		VALUE_conv_string(P2);

	//   if (op < 2 && P1->_string.len != P2->_string.len)
	//     result = 1;
	//   else
	//     result = STRING_comp_value(P1, P2);
		if (op < 2)
		{
			if (P1->_string.len != P2->_string.len)
				result = 1;
			else
				result = !STRING_equal_same(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len);
		}
		else
			result = STRING_compare(P1->_string.addr + P1->_string.start, P1->_string.len, P2->_string.addr + P2->_string.start, P2->_string.len);
		goto __SC_END_RELEASE;

	__SC_SINGLE:
	__SC_FLOAT:

		VALUE_conv_float(P1);
		VALUE_conv_float(P2);

		result = fsgn(P1->_float.value - P2->_float.value);
		goto __SC_END;

	__SC_POINTER:

		VALUE_conv(P1, T_POINTER);
		VALUE_conv(P2, T_POINTER);

		result = P1->_pointer.value > P2->_pointer.value ? 1 : P1->_pointer.value < P2->_pointer.value ? -1 : 0;
		goto __SC_END;

	__SC_OBJECT:

		result = OBJECT_comp_value(P1, P2);
		goto __SC_END_RELEASE;

	__SC_VARIANT:

		{
			bool variant = FALSE;
		
			if (TYPE_is_variant(P1->type))
			{
				VARIANT_undo(P1);
				variant = TRUE;
			}

			if (TYPE_is_variant(P2->type))
			{
				VARIANT_undo(P2);
				variant = TRUE;
			}

			type = Max(P1->type, P2->type);

			if (op > 1)
			{
				if (type == T_NULL || TYPE_is_string(type))
				{
					TYPE typem = Min(P1->type, P2->type);
					if (!TYPE_is_string(typem))
						THROW(E_TYPE, TYPE_get_name(typem), TYPE_get_name(type));
				}
				else if (TYPE_is_object(type))
					goto __SC_ERROR;
			}
			else
			{
				if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
					type = T_OBJECT;
				else if (TYPE_is_object(type))
					THROW(E_TYPE, "Object", TYPE_get_name(Min(P1->type, P2->type)));
			}

			if (!variant)
				*PC |= type;

			goto *jump[type];
		}

	__SC_ERROR:

		THROW(E_TYPE, "Number, Date or String", TYPE_get_name(type));

	__SC_END_RELEASE:

		RELEASE(P1);
		RELEASE(P2);

	__SC_END:

		P1->type = T_BOOLEAN;
		SP--;

		goto *test[(int)op];

	__SC_EQ:
		P1->_boolean.value = result == 0 ? -1 : 0;
		goto _SUBR_END;

	__SC_NE:
		P1->_boolean.value = result != 0 ? -1 : 0;
		goto _SUBR_END;

	__SC_GT:
		P1->_boolean.value = result > 0 ? -1 : 0;
		goto _SUBR_END;

	__SC_GE:
		P1->_boolean.value = result >= 0 ? -1 : 0;
		goto _SUBR_END;

	__SC_LT:
		P1->_boolean.value = result < 0 ? -1 : 0;
		goto _SUBR_END;

	__SC_LE:
		P1->_boolean.value = result <= 0 ? -1 : 0;
		goto _SUBR_END;
	}
#endif
}


static void my_VALUE_class_read(CLASS *class, VALUE *value, char *addr, CTYPE ctype, void *ref)
{
	VALUE_class_read_inline(class, value, addr, ctype, ref);
}

static void _SUBR_comp(ushort code)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL, &&__OBJECT
		};

	//static void *test[] = { &&__EQ, &&__NE, &&__GT, &&__LE, &&__LT, &&__GE };

	char NO_WARNING(result);
	VALUE *P1;
	VALUE *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	goto *jump[code & 0x1F];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	result = P1->_integer.value == P2->_integer.value;
	goto __END;
	
__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	result = P1->_long.value == P2->_long.value;
	goto __END;

__DATE:

	VALUE_conv(P1, T_DATE);
	VALUE_conv(P2, T_DATE);

	result = DATE_comp_value(P1, P2) == 0;
	goto __END;

__NULL:

	if (P2->type == T_NULL)
	{
		result = VALUE_is_null(P1);
		goto __END_RELEASE;
	}
	else if (P1->type == T_NULL)
	{
		result = VALUE_is_null(P2);
		goto __END_RELEASE;
	}

__STRING:

	VALUE_conv_string(P1);
	VALUE_conv_string(P2);

	if (P1->_string.len != P2->_string.len)
		result = 0;
	else
		result = STRING_equal_same(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len);
	
	RELEASE_STRING(P1);
	RELEASE_STRING(P2);
	goto __END;

__SINGLE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	result = P1->_float.value == P2->_float.value;
	goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	result = P1->_pointer.value == P2->_pointer.value;
	goto __END;

__OBJECT:

	result = OBJECT_comp_value(P1, P2) == 0;
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __END_RELEASE;

__VARIANT:

	{
		bool variant = FALSE;
		TYPE type;
	
		if (TYPE_is_variant(P1->type))
		{
			VARIANT_undo(P1);
			variant = TRUE;
		}

		if (TYPE_is_variant(P2->type))
		{
			VARIANT_undo(P2);
			variant = TRUE;
		}

		type = Max(P1->type, P2->type);

		if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
			type = T_OBJECT;
		else if (TYPE_is_object(type))
			THROW(E_TYPE, "Object", TYPE_get_name(Min(P1->type, P2->type)));

		if (!variant)
			*PC |= type;

		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number, Date or String", TYPE_get_name(code & 0x1F));

__END_RELEASE:

	RELEASE(P1);
	RELEASE(P2);

__END:

	P1->type = T_BOOLEAN;
	SP--;
	P1->_boolean.value = -result;
}

static void _SUBR_compn(ushort code)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL, &&__OBJECT
		};

	//static void *test[] = { &&__EQ, &&__NE, &&__GT, &&__LE, &&__LT, &&__GE };

	char NO_WARNING(result);
	VALUE *P1;
	VALUE *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	goto *jump[code & 0x1F];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	result = P1->_integer.value == P2->_integer.value;
	goto __END;
	
__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	result = P1->_long.value == P2->_long.value;
	goto __END;

__DATE:

	VALUE_conv(P1, T_DATE);
	VALUE_conv(P2, T_DATE);

	result = DATE_comp_value(P1, P2) == 0;
	goto __END;

__NULL:

	if (P2->type == T_NULL)
	{
		result = VALUE_is_null(P1);
		goto __END_RELEASE;
	}
	else if (P1->type == T_NULL)
	{
		result = VALUE_is_null(P2);
		goto __END_RELEASE;
	}

__STRING:

	VALUE_conv_string(P1);
	VALUE_conv_string(P2);

	if (P1->_string.len != P2->_string.len)
		result = 0;
	else
		result = STRING_equal_same(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len);
	
	RELEASE_STRING(P1);
	RELEASE_STRING(P2);
	goto __END;

__SINGLE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	result = P1->_float.value == P2->_float.value;
	goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	result = P1->_pointer.value == P2->_pointer.value;
	goto __END;

__OBJECT:

	result = OBJECT_comp_value(P1, P2) == 0;
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __END_RELEASE;

__VARIANT:

	{
		bool variant = FALSE;
		TYPE type;
	
		if (TYPE_is_variant(P1->type))
		{
			VARIANT_undo(P1);
			variant = TRUE;
		}

		if (TYPE_is_variant(P2->type))
		{
			VARIANT_undo(P2);
			variant = TRUE;
		}

		type = Max(P1->type, P2->type);

		if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
			type = T_OBJECT;
		else if (TYPE_is_object(type))
			THROW(E_TYPE, "Object", TYPE_get_name(Min(P1->type, P2->type)));

		if (!variant)
			*PC |= type;

		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number, Date or String", TYPE_get_name(code & 0x1F));

__END_RELEASE:

	RELEASE(P1);
	RELEASE(P2);

__END:

	P1->type = T_BOOLEAN;
	SP--;

	P1->_boolean.value = result - 1; // ? 0 : -1;
}

static void my_VALUE_class_constant(CLASS *class, VALUE *value, int ind)
{
	VALUE_class_constant_inline(class, value, ind);
}

#define MANAGE_VARIANT(_func) \
({ \
	type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	VARIANT_undo(P1); \
	VARIANT_undo(P2); \
	\
	if (TYPE_is_string(P1->type)) \
		VALUE_conv_float(P1); \
	\
	if (TYPE_is_string(P2->type)) \
		VALUE_conv_float(P2); \
	\
	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type)) \
		type = T_NULL; \
	else \
		type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		(_func)(code | type); \
		VALUE_conv_variant(P1); \
		return; \
	} \
})


static void _SUBR_add(ushort code)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__DATE
    };

  TYPE type;
  VALUE *P1, *P2;

  P1 = SP - 2;
  P2 = P1 + 1;

  type = code & 0x0F;
  goto *jump[type];

__BOOLEAN:
	
	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value | P2->_integer.value; goto __END;

__BYTE:
	
	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value + P2->_integer.value); goto __END;

__SHORT:
	
	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value + P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value += P2->_integer.value; goto __END;

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	P1->_long.value += P2->_long.value; goto __END;

__DATE:
__FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	P1->_float.value += P2->_float.value; goto __END;

__VARIANT:

	MANAGE_VARIANT(_SUBR_add);
  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

  SP--;
}

static void _SUBR_sub(ushort code)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__DATE
    };

  TYPE type;
  VALUE *P1, *P2;

  P1 = SP - 2;
  P2 = P1 + 1;

  type = code & 0x0F;
  goto *jump[type];

__BOOLEAN:
	
	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value ^ P2->_integer.value; goto __END;

__BYTE:
	
	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value - P2->_integer.value); goto __END;

__SHORT:
	
	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value - P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value -= P2->_integer.value; goto __END;

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	P1->_long.value -= P2->_long.value; goto __END;

__DATE:
__FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	P1->_float.value -= P2->_float.value; goto __END;

__VARIANT:

	MANAGE_VARIANT(_SUBR_sub);
  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

  SP--;
}

static void _SUBR_mul(ushort code)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__ERROR
    };

  TYPE type;
  VALUE *P1, *P2;

  P1 = SP - 2;
  P2 = P1 + 1;

  type = code & 0x0F;
  goto *jump[type];

__BOOLEAN:
	
	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value & P2->_integer.value; goto __END;

__BYTE:
	
	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value * P2->_integer.value); goto __END;

__SHORT:
	
	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value * P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value *= P2->_integer.value; goto __END;

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	P1->_long.value *= P2->_long.value; goto __END;

__FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	P1->_float.value *= P2->_float.value; goto __END;

__VARIANT:

	MANAGE_VARIANT(_SUBR_mul);
  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

  SP--;
}

static void _SUBR_div(ushort code)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__ERROR
    };

  TYPE type;
  VALUE *P1, *P2;

  P1 = SP - 2;
  P2 = P1 + 1;

  type = code & 0x0F;
  goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:
__LONG:
__FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	P1->_float.value /= P2->_float.value;
	if (!finite(P1->_float.value))
	{
		if (P2->_float.value == 0.0)
			THROW(E_ZERO);
		else
			THROW(E_MATH);
	}
	goto __END;

__VARIANT:

	MANAGE_VARIANT(_SUBR_div);
  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

  SP--;
}
