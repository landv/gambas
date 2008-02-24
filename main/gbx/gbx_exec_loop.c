/***************************************************************************

  exec_loop.c

  The interpreter main loop

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

//#define DEBUG_PCODE 1

#if DEBUG_PCODE
#define PROJECT_EXEC
#include "gb_pcode_temp.h"
#endif

#define SUBR_beep EXEC_ILLEGAL


#define GET_XXX()   (((signed short)(code << 4)) >> 4)
#define GET_UXX()   (code & 0xFFF)
#define GET_7XX()   (code & 0x7FF)
#define GET_XX()    ((signed char)(code & 0xFF))
#define GET_3X()    (code & 0x3F)


static EXEC_FUNC SubrTable[] =
{
  /* 28 */  SUBR_comp,            SUBR_comp,            SUBR_comp,            SUBR_comp,
  /* 2C */  SUBR_comp,            SUBR_comp,            SUBR_near,            SUBR_case,
  /* 30 */  SUBR_add_,            SUBR_add_,            SUBR_add_,            SUBR_add_,
  /* 34 */  SUBR_neg_,            SUBR_quo,             SUBR_rem,             SUBR_pow,
  /* 38 */  SUBR_and_,            SUBR_and_,            SUBR_and_,            SUBR_not,
  /* 3C */  SUBR_cat,             SUBR_like,            SUBR_file,            SUBR_is,

  NULL,            /* 00 */
  NULL,            /* 01 */
  NULL,            /* 02 */
  NULL,            /* 03 */
  SUBR_space,      /* 04 */
  SUBR_string,     /* 05 */
  SUBR_trim,       /* 06 */
  SUBR_upper,      /* 07 */
  SUBR_lower,      /* 08 */
  SUBR_chr,        /* 09 */
  SUBR_asc,        /* 10 */
  SUBR_instr,      /* 11 */
  SUBR_instr,      /* 12 */
  SUBR_subst,      /* 13 */
  SUBR_replace,    /* 14 */
  SUBR_split,      /* 15 */
  SUBR_scan,       /* 16 */
  SUBR_strcomp,    /* 17 */
  SUBR_iconv,      /* 18 */
  SUBR_sconv,      /* 19 */
  SUBR_neg_,       /* 20 */
  SUBR_neg_,       /* 21 */
  SUBR_neg_,       /* 22 */
  SUBR_sgn,        /* 23 */
  SUBR_math,       /* 24 */
  SUBR_pi,         /* 25 */
  SUBR_round,      /* 26 */
  SUBR_randomize,  /* 27 */
  SUBR_rnd,        /* 28 */
  SUBR_min_max,    /* 29 */
  SUBR_min_max,    /* 30 */
  SUBR_if,         /* 31 */
  SUBR_choose,     /* 32 */
  SUBR_array,      /* 33 */
  SUBR_math2,      /* 34 */
  SUBR_is_chr,     /* 35 */
  SUBR_bit,        /* 36 */
  SUBR_is_type,    /* 37 */
  SUBR_type,       /* 38 */
  SUBR_conv,       /* 39 */
  SUBR_bin,        /* 40 */
  SUBR_hex,        /* 41 */
  SUBR_val,        /* 42 */
  SUBR_str,        /* 43 */
  SUBR_format,     /* 44 */
  SUBR_timer,      /* 45 */
  SUBR_now,        /* 46 */
  SUBR_year,       /* 47 */
  SUBR_week,       /* 48 */
  SUBR_date,       /* 49 */
  SUBR_time,       /* 50 */
  SUBR_date_op,    /* 51 */
  SUBR_eval,       /* 52 */
  SUBR_error,      /* 53 */
  SUBR_debug,      /* 54 */
  SUBR_wait,       /* 55 */
  SUBR_open,       /* 56 */
  SUBR_close,      /* 57 */
  SUBR_input,      /* 58 */
  SUBR_linput,     /* 59 */
  SUBR_print,      /* 60 */
  SUBR_read,       /* 61 */
  SUBR_write,      /* 62 */
  SUBR_flush,      /* 63 */
  SUBR_lock,       /* 64 */
  SUBR_inp_out,    /* 65 */
  SUBR_eof,        /* 66 */
  SUBR_lof,        /* 67 */
  SUBR_seek,       /* 68 */
  SUBR_kill,       /* 69 */
  SUBR_mkdir,      /* 70 */
  SUBR_rmdir,      /* 71 */
  SUBR_rename,     /* 72 */
  SUBR_copy,       /* 73 */
  SUBR_link,       /* 74 */
  SUBR_exist,      /* 75 */
  SUBR_access,     /* 76 */
  SUBR_stat,       /* 77 */
  SUBR_dfree,      /* 78 */
  SUBR_temp,       /* 79 */
  SUBR_isdir,      /* 80 */
  SUBR_dir,        /* 81 */
  SUBR_rdir,       /* 82 */
  SUBR_exec,       /* 83 */
  SUBR_alloc,      /* 84 */
  SUBR_free,       /* 85 */
  SUBR_realloc,    /* 86 */
  SUBR_strptr,     /* 87 */
  SUBR_sleep,      /* 88 */
  SUBR_varptr,     /* 89 */
  /* -> 95 */
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
    /* 28 =               */  &&_SUBR,
    /* 29 <>              */  &&_SUBR,
    /* 2A >               */  &&_SUBR,
    /* 2B <=              */  &&_SUBR,
    /* 2C <               */  &&_SUBR,
    /* 2D >=              */  &&_SUBR,
    /* 2E ==              */  &&_SUBR,
    /* 2F CASE            */  &&_SUBR,
    /* 30 +               */  &&_SUBR,
    /* 31 -               */  &&_SUBR,
    /* 32 *               */  &&_SUBR,
    /* 33 /               */  &&_SUBR,
    /* 34 NEG             */  &&_SUBR,
    /* 35 \               */  &&_SUBR,
    /* 36 MOD             */  &&_SUBR,
    /* 37 ^               */  &&_SUBR,
    /* 38 AND             */  &&_SUBR,
    /* 39 OR              */  &&_SUBR,
    /* 3A XOR             */  &&_SUBR,
    /* 3B NOT             */  &&_SUBR,
    /* 3C &               */  &&_SUBR,
    /* 3D LIKE            */  &&_SUBR,
    /* 3E &/              */  &&_SUBR,
    /* 3F                 */  &&_SUBR,
    /* 40 Left$           */  &&_SUBR_LEFT,
    /* 41 Mid$            */  &&_SUBR_MID,
    /* 42 Right$          */  &&_SUBR_RIGHT,
    /* 43 Len             */  &&_SUBR_LEN,
    /* 44 Space$          */  &&_SUBR_1,
    /* 45 String$         */  &&_SUBR,
    /* 46 Trim$           */  &&_SUBR_1,
    /* 47 UCase$          */  &&_SUBR_1,
    /* 48 LCase$          */  &&_SUBR_1,
    /* 49 Chr$            */  &&_SUBR_1,
    /* 4A Asc             */  &&_SUBR,
    /* 4B InStr           */  &&_SUBR,
    /* 4C RInStr          */  &&_SUBR,
    /* 4D Subst$          */  &&_SUBR,
    /* 4E Replace$        */  &&_SUBR,
    /* 4F Split           */  &&_SUBR,
    /* 50 Conv$           */  &&_SUBR,
    /* 51 Abs             */  &&_SUBR_1,
    /* 52 Int             */  &&_SUBR_1,
    /* 53 Fix             */  &&_SUBR_1,
    /* 54 Sgn             */  &&_SUBR_1,
    /* 55 Math            */  &&_SUBR_1,
    /* 56 Pi              */  &&_SUBR,
    /* 57 Round           */  &&_SUBR,
    /* 58 Randomize       */  &&_SUBR,
    /* 59 Rnd             */  &&_SUBR,
    /* 5A Min             */  &&_SUBR,
    /* 5B Max             */  &&_SUBR,
    /* 5C IIf             */  &&_SUBR,
    /* 5D Choose          */  &&_SUBR,
    /* 5E Array           */  &&_SUBR,
    /* 5F ATan2           */  &&_SUBR,
    /* 60 IsAscii         */  &&_SUBR_1,
    /* 61                 */  &&_SUBR,
    /* 62                 */  &&_SUBR,
    /* 63 BClr            */  &&_SUBR,
    /* 64                 */  &&_SUBR,
    /* 65                 */  &&_SUBR,
    /* 66                 */  &&_SUBR,
    /* 67                 */  &&_SUBR,
    /* 68                 */  &&_SUBR,
    /* 69                 */  &&_SUBR,
    /* 6A IsBoolean       */  &&_SUBR_1,
    /* 6B TypeOf          */  &&_SUBR_1,
    /* 6C CBool           */  &&_SUBR_1,
    /* 6D Bin$            */  &&_SUBR,
    /* 6E Hex$            */  &&_SUBR,
    /* 6F Val             */  &&_SUBR_1,
    /* 70 Str$            */  &&_SUBR_1,
    /* 71 Format$         */  &&_SUBR,
    /* 72 Timer           */  &&_SUBR,
    /* 73 Now             */  &&_SUBR,
    /* 74 Year            */  &&_SUBR_1,
    /* 75 Date            */  &&_SUBR,
    /* 76 Time            */  &&_SUBR,
    /* 77 DateAdd         */  &&_SUBR,
    /* 78 Error           */  &&_SUBR,
    /* 79 Eval            */  &&_SUBR,
    /* 7A Beep            */  &&_SUBR,
    /* 7B Wait            */  &&_SUBR,
    /* 7C Open            */  &&_SUBR,
    /* 7D Close           */  &&_SUBR,
    /* 7E Input           */  &&_SUBR,
    /* 7F LineInput       */  &&_SUBR,
    /* 80 Print           */  &&_SUBR,
    /* 81 Read            */  &&_SUBR,
    /* 82 Write           */  &&_SUBR,
    /* 83 Flush           */  &&_SUBR,
    /* 84 Lock            */  &&_SUBR,
    /* 85 InputFrom       */  &&_SUBR,
    /* 86 Eof             */  &&_SUBR,
    /* 87 Lof             */  &&_SUBR,
    /* 88 Seek            */  &&_SUBR,
    /* 89 Kill            */  &&_SUBR,
    /* 8A Mkdir           */  &&_SUBR,
    /* 8B Rmdir           */  &&_SUBR,
    /* 8C Rename          */  &&_SUBR,
    /* 8D Copy            */  &&_SUBR,
    /* 8E Link            */  &&_SUBR,
    /* 8F Exist           */  &&_SUBR,
    /* 90 Access          */  &&_SUBR,
    /* 91 Stat            */  &&_SUBR,
    /* 92 Temp$           */  &&_SUBR,
    /* 93 IsDir           */  &&_SUBR,
    /* 94 Dir             */  &&_SUBR,
    /* 95 RDir            */  &&_SUBR,
    /* 96 Exec            */  &&_SUBR,
    /* 97 Alloc           */  &&_SUBR,
    /* 98 Free            */  &&_SUBR,
    /* 99 Realloc         */  &&_SUBR,
    /* 9A StrPtr          */  &&_SUBR,
    /* 9B SConv$          */  &&_SUBR,
    /* 9C Debug           */  &&_SUBR,
    /* 9D Dfree           */  &&_SUBR,
    /* 9E Week            */  &&_SUBR,
    /* 9F Comp            */  &&_SUBR,
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

  register int NO_WARNING(ind);
  register ushort code;
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

_NEXT3:

  PC++;

_NEXT2:

  PC++;

_NEXT:

  PC++;

/*-----------------------------------------------*/

_MAIN:

#if DEBUG_PCODE
    DEBUG_where();
    fprintf(stderr, "[%4d] ", (int)(intptr_t)(SP - (VALUE *)STACK_base));
    if (*PC >> 8)
      PCODE_dump(stderr, PC - FP->code, PC);
#endif

  code = *PC;
  goto *jump_table[code >> 8];

/*-----------------------------------------------*/

_SUBR:
_SUBR_1:

  EXEC_code = code;

  (*SubrTable[(code >> 8) - 0x28])();

_SUBR_END:

  if (PCODE_is_void(code))
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

	ind = GET_XX();

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

    if (val->type == T_VOID)
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
  goto _NEXT2;

/*-----------------------------------------------*/

_PUSH_CHAR:

  STRING_char_value(SP, (char)GET_XX());
  SP++;
  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_ME:

	/*SP->type = T_CLASS;
	SP->_class.class = (CLASS *)(GET_XX() & 3);*/

	#if 1
  if (GET_XX() & 1)
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
    if (OP)
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

	if (GET_XX() & 2)
	{
		// The used class must be in the stack, because it is tested by exec_push && exec_pop
		SP->_object.class = SP->_object.class->parent;
  	SP->_object.super = EXEC_super;
  	EXEC_super = SP;
	}
	#endif

  PUSH();
  goto _NEXT;

/*-----------------------------------------------*/

_PUSH_MISC:

  {
    static void *_jump[] =
      { &&__PUSH_NULL, &&__PUSH_VOID, &&__PUSH_FALSE, &&__PUSH_TRUE, &&__PUSH_LAST };

    goto *_jump[GET_XX()];

  __PUSH_NULL:

    SP->type = T_NULL;
    SP->_integer.value = 0;
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

  VALUE_conv(&SP[-1], T_BOOLEAN);
  SP--;
  if (SP->_boolean.value)
  {
    PC += (signed short)PC[1] + 2;
    goto _MAIN;
  }

  goto _NEXT2;

/*-----------------------------------------------*/

_JUMP_IF_FALSE:

  VALUE_conv(&SP[-1], T_BOOLEAN);
  SP--;
  if (SP->_boolean.value == 0)
  {
    PC += (signed short)PC[1] + 2;
    goto _MAIN;
  }

  goto _NEXT2;

/*-----------------------------------------------*/

_RETURN:

  if (GET_XX())
  {
    VALUE_conv(&SP[-1], FP->type);
    SP--;
    *RP = *SP;
		ERROR_clear();
    EXEC_leave(FALSE);
  }
  else
  {
    VALUE_default(RP, FP->type);
		ERROR_clear();
    EXEC_leave(FALSE);
  }

  if (PC == NULL)
    return;

  goto _NEXT;

/*-----------------------------------------------*/

_CALL:

  {
    static void *call_jump[] =
      { &&__CALL_NULL, &&__CALL_NATIVE, &&__CALL_PRIVATE, &&__CALL_PUBLIC,
        &&__CALL_EVENT, &&__CALL_EXTERN, &&__CALL_UNKNOWN, &&__CALL_CALL };

    register VALUE * NO_WARNING(val);

    ind = GET_3X();
    val = &SP[-(ind + 1)];

    if (!TYPE_is_function(val->type))
    {
      /*EXEC_object(val, &EXEC.class, (OBJECT **)&EXEC.object, &defined);
      EXEC.drop = PCODE_is_void(code);
      EXEC.nparam = ind;
	    EXEC.use_stack = FALSE;

      if (!defined)
        *PC |= CODE_CALL_VARIANT;*/

			bool defined;

      EXEC_object(val, &EXEC.class, (OBJECT **)&EXEC.object, &defined);
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

		if (!val->_function.defined)
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
    if (!EXEC.desc && !EXEC.object && EXEC.nparam == 1 && !EXEC.class->is_virtual)
    {
    	SP[-2] = SP[-1];
    	SP--;
			VALUE_conv(SP - 1, (TYPE)EXEC.class);
			goto _NEXT;
    }
    
    goto __CALL_SPEC;

  __CALL_SPEC:

    if (!EXEC.desc)
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
    static void *call_jump[] =
      { &&__CALL_NULL, &&__CALL_NATIVE_Q, &&__CALL_PRIVATE_Q, &&__CALL_PUBLIC_Q };

    register VALUE * NO_WARNING(val);

    ind = GET_3X();
    val = &SP[-(ind + 1)];

    EXEC.class = val->_function.class;
    EXEC.object = val->_function.object;
    EXEC.drop = PCODE_is_void(code);
    EXEC.nparam = ind;

    if (!val->_function.defined)
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
    static void *call_jump[] =
      { &&__CALL_NULL, &&__CALL_NATIVE_N, &&__CALL_PRIVATE_N, &&__CALL_PUBLIC_N };

    register VALUE * NO_WARNING(val);

    ind = GET_3X();
    val = &SP[-(ind + 1)];

    EXEC.class = val->_function.class;
    EXEC.object = val->_function.object;
    EXEC.drop = PCODE_is_void(code);
    EXEC.nparam = ind;
    EXEC.use_stack = TRUE;

    if (!val->_function.defined)
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
    static void *jn_jump[] = { &&_JN_START, &&_JN_NEXT_1, &&_JN_NEXT_2, &&_JN_NEXT_3, &&_JN_NEXT_4, &&_JN_NEXT_5, &&_JN_NEXT_6 };

    VALUE * NO_WARNING(end);
    VALUE * NO_WARNING(inc);
    register VALUE * NO_WARNING(val);
    
    end = &BP[PC[-1] & 0xFF];
    inc = end + 1;
    val = &BP[PC[2] & 0xFF];

    goto *jn_jump[GET_XX()];

  _JN_START:

    type = val->type;

		// The step value must stay negative, even if the loop variable is a byte

    if (TYPE_is_integer(type))
    {
    	VALUE_conv(&SP[-1], T_INTEGER);
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

    if (TYPE_is_integer(type))
    {
      if (inc->_integer.value > 0)
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

    if (val->_integer.value <= end->_integer.value)
      goto _NEXT3;

    goto _JN_END;

  _JN_NEXT_2:

    val->_integer.value += inc->_integer.value;

  _JN_TEST_2:

    if (val->_integer.value >= end->_integer.value)
      goto _NEXT3;

    goto _JN_END;

  _JN_NEXT_3:

    val->_float.value += inc->_float.value;

  _JN_TEST_3:

    if (val->_float.value <= end->_float.value)
      goto _NEXT3;

    goto _JN_END;

  _JN_NEXT_4:

    val->_float.value += inc->_float.value;

  _JN_TEST_4:

    if (val->_float.value >= end->_float.value)
      goto _NEXT3;

    goto _JN_END;

  _JN_NEXT_5:

    val->_long.value += inc->_long.value;

  _JN_TEST_5:

    if (val->_long.value <= end->_long.value)
      goto _NEXT3;

    goto _JN_END;

  _JN_NEXT_6:

    val->_long.value += inc->_long.value;

  _JN_TEST_6:

    if (val->_long.value >= end->_long.value)
      goto _NEXT3;

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

  if (EXEC_enum_next(code))
    goto _JUMP;
  else
    goto _NEXT2;

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
  SP->_function.index = GET_XX();
  SP->_function.defined = TRUE;

  //OBJECT_REF(OP, "exec_loop._PUSH_FUNCTION (FUNCTION)");
  SP++;

  goto _NEXT;

/*-----------------------------------------------*/

  {
    CLASS_VAR *var;
    char *addr;

_PUSH_DYNAMIC:

    var = &CP->load->dyn[GET_7XX()];

    if (OP == NULL)
      THROW(E_ILLEGAL);

    addr = &OP[var->pos];
    goto __READ;

_PUSH_STATIC:

    var = &CP->load->stat[GET_7XX()];
    addr = (char *)CP->stat + var->pos;
    goto __READ;

__READ:

    VALUE_class_read(CP, SP, addr, var->type);

    PUSH();
    goto _NEXT;


_POP_DYNAMIC:

    var = &CP->load->dyn[GET_7XX()];

    if (OP == NULL)
      THROW(E_ILLEGAL);

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

  VALUE_class_constant(CP, SP, GET_UXX());
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

  SUBR_add_quick(GET_XXX());

  goto _NEXT;

/*-----------------------------------------------*/

_TRY:

  //ERROR_clear();
  EP = SP;
  ET = EC;
  EC = PC + (signed short)PC[1] + 2;

  #if DEBUG_ERROR
  printf("TRY %p\n", EC);
  #endif

  goto _NEXT2;

/*-----------------------------------------------*/

_END_TRY:

  #if DEBUG_ERROR
  printf("END TRY %p\n", PC);
  #endif

  /* If EP was reset to null, then there was an error */
 	EXEC_got_error = (EP == NULL);
  EP = NULL;
  EC = ET;
  ET = NULL;
  goto _NEXT;

/*-----------------------------------------------*/

_CATCH:

  if (EC == NULL)
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

    ind = GET_XX();

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

  if (GET_XX() == 0)
    EXEC_quit();

  if (EXEC_debug && CP && CP->component == COMPONENT_main)
    DEBUG.Breakpoint(0);
	//else
	//	TRACE_backtrace(stderr);

  goto _NEXT;

/*-----------------------------------------------*/

_BYREF:

	if (PC == FP->code)
	{
		PC += GET_XX();
		goto _NEXT;
	}

	THROW(E_BYREF);

/*-----------------------------------------------*/

_ILLEGAL:

  THROW(E_ILLEGAL);

/*-----------------------------------------------*/

#define EXEC_code code

_SUBR_LEFT:

	{
		int val;
	
		SUBR_ENTER();
	
		if (!SUBR_check_string(PARAM))
		{
			if (NPARAM == 1)
				val = 1;
			else
			{
				VALUE_conv(&PARAM[1], T_INTEGER);
				val = PARAM[1]._integer.value;
			}
		
			if (val < 0)
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
	
		if (!SUBR_check_string(PARAM))
		{
			if (NPARAM == 1)
				val = 1;
			else
			{
				VALUE_conv(&PARAM[1], T_INTEGER);
				val = PARAM[1]._integer.value;
			}
		
			if (val < 0)
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
	
		SUBR_ENTER();
	
		if (SUBR_check_string(PARAM))
			goto _SUBR_MID_FIN;
	
		VALUE_conv(&PARAM[1], T_INTEGER);
		start = PARAM[1]._integer.value - 1;
	
		if (start < 0)
			THROW(E_ARG);
	
		if (start >= PARAM->_string.len)
		{
			RELEASE(PARAM);
			STRING_void_value(PARAM);
			goto _SUBR_MID_FIN;
		}
	
		if (NPARAM == 2)
			len = PARAM->_string.len;
		else
		{
			VALUE_conv(&PARAM[2], T_INTEGER);
			len = PARAM[2]._integer.value;
		}
	
		if (len < 0)
			len = Max(0, PARAM->_string.len - start + len);
	
		len = MinMax(len, 0, PARAM->_string.len - start);
	
		if (len == 0)
		{
			RELEASE(PARAM);
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
	
		if (SUBR_check_string(PARAM))
			len = 0;
		else
			len = PARAM->_string.len;
	
		RELEASE(PARAM);
	
		PARAM->type = T_INTEGER;
		PARAM->_integer.value = len;
	}
	goto _SUBR_END;
	
/*-----------------------------------------------*/

}

