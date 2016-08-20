/***************************************************************************

  gb_pcode.h

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

#ifndef __GB_PCODE_H
#define __GB_PCODE_H

/* If this file is modified, don't forget to update GAMBAS_PCODE_VERSION in acinclude.m4 */

#define C_NOP                   0x0000

#define C_PUSH_QUICK            0xF000
#define C_PUSH_CONST            0xE000

#define C_POP_STATIC            0xD800
#define C_POP_DYNAMIC           0xD000
#define C_PUSH_STATIC           0xC800
#define C_PUSH_DYNAMIC          0xC000
#define C_PUSH_FUNCTION         0xB800
#define C_PUSH_CLASS            0xB000

#define C_ADD_QUICK             0xA000

#define C_PUSH_LOCAL            0x0100
#define C_PUSH_PARAM            0x0200
#define C_PUSH_ARRAY            0x0300
#define C_PUSH_UNKNOWN          0x0400

#define C_PUSH_EXTERN           0x0500
#define C_BYREF                 0x0600
#define C_PUSH_EVENT            0x0700
#define C_QUIT                  0x0800

#define C_POP_LOCAL             0x0900
#define C_POP_PARAM             0x0A00
#define C_POP_ARRAY             0x0B00
#define C_POP_UNKNOWN           0x0C00

#define C_POP_OPTIONAL          0x0D00
#define C_POP_CTRL              0x0E00

#define C_BREAK                 0x0F00

#define C_RETURN                0x1000

#define C_PUSH_INTEGER          0x1100
#define C_PUSH_LONG             0x1200
#define C_PUSH_CHAR             0x1300
#define C_PUSH_MISC             0x1400
#define C_PUSH_ME               0x1500

#define CPM_NULL         0
#define CPM_VOID         1
#define CPM_FALSE        2
#define CPM_TRUE         3
#define CPM_LAST         4
#define CPM_STRING       5
#define CPM_PINF         6
#define CPM_MINF         7
#define CPM_COMPLEX      8
#define CPM_VARGS        9
#define CPM_DROP_VARGS   10

#define C_TRY                   0x1600
#define C_END_TRY               0x1700
#define C_CATCH                 0x1800

#define C_DUP                   0x1900
#define C_DROP                  0x1A00
#define C_NEW                   0x1B00

#define C_CALL                  0x1C00
#define C_CALL_QUICK            0x1D00
#define C_CALL_SLOW             0x1E00
#define C_ON                    0x1F00

#define C_JUMP                  0x2000
#define C_JUMP_IF_TRUE          0x2100
#define C_JUMP_IF_FALSE         0x2200
#define C_GOSUB                 0x2300

#define C_JUMP_FIRST            0x2400
#define C_JUMP_NEXT             0x2500
#define C_FIRST                 0x2600
#define C_NEXT                  0x2700

#define C_EQ                    0x2800
#define C_NE                    0x2900
#define C_GT                    0x2A00
#define C_LE                    0x2B00
#define C_LT                    0x2C00
#define C_GE                    0x2D00
#define C_NEAR                  0x2E00
#define C_CASE                  0x2F00

#define C_ADD                   0x3000
#define C_SUB                   0x3100
#define C_MUL                   0x3200
#define C_DIV                   0x3300
#define C_NEG                   0x3400
#define C_QUO                   0x3500
#define C_REM                   0x3600
#define C_POW                   0x3700
#define C_AND                   0x3800
#define C_OR                    0x3900
#define C_XOR                   0x3A00
#define C_NOT                   0x3B00
#define C_CAT                   0x3C00
#define C_LIKE                  0x3D00
#define C_FILE                  0x3E00
#define C_IS                    0x3F00


#define CODE_FIRST_SUBR 0x40
#define CODE_LAST_SUBR  0x9F

#define CODE_CALL_VARIANT   0x80
//#define CODE_CALL_VOID      0x40

#define CODE_STATIC    0x0800
#define CODE_FUNCTION  0x0400

#define CODE_NEW_ARRAY  0x40
#define CODE_NEW_EVENT  0x80

#define CODE_RINSTR      (CODE_FIRST_SUBR + 12)
#define CODE_ABS         (CODE_FIRST_SUBR + 20)
#define CODE_MAX         (CODE_FIRST_SUBR + 30)
#define CODE_CONV        (CODE_FIRST_SUBR + 39)
#define CODE_BIN         (CODE_FIRST_SUBR + 40)


typedef
  ushort PCODE;

#define PCODE_is(pcode, value)  (((pcode) & 0xFF00) == (value))
#define PCODE_get_nparam(pcode) ((pcode) & 0x3F)

#define PCODE_is_variant(pcode)  ((pcode) & CODE_CALL_VARIANT)
#define PCODE_is_void(pcode)    ((pcode) & CODE_CALL_VOID)

#define PCODE_is_breakpoint(pcode) PCODE_is(pcode, C_BREAK)

#define PCODE_BREAKPOINT(num) ((PCODE)(C_BREAK | (num)))

#ifndef NO_CODE_DUMP
short PCODE_dump(FILE *out, ushort addr, PCODE *code);
#endif

#endif /* */
