/***************************************************************************

  gb_reserved.h

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

#ifndef __GB_RESERVED_H
#define __GB_RESERVED_H

#include "gb_common.h"
#include "gb_table.h"

#define RSF_NONE          0x0
#define RSF_OP            0x1
#define RSF_TYPE          0x2
#define RSF_ASGN          0x4
#define RSF_N_ARY         0x00
#define RSF_UNARY         0x10
#define RSF_BINARY        0x20
#define RSF_POST          0x30
#define RSF_ONLY          0x40

#define RSF_OPN       0x01
#define RSF_OP1       0x11
#define RSF_OP2       0x21
#define RSF_OPP       0x31
//#define RSF_OPI       0x61
#define RSF_OP2S      0x61
#define RSF_OP2SM     0xA1

enum
{
	RSF_INF    = 0x0100,
	RSF_ILF    = 0x0200,   // last pattern waits for a function name
	RSF_ILD    = 0x0400,   // last pattern waits for an identifier
	RSF_ILDD   = 0x0800,   // last pattern waits for an identifier only if the previous one waits for an identifier too
	RSF_ILT    = 0x1000,   // last pattern waits for a datatype
	RSF_ILC    = 0x2000,   // last pattern waits for a class
	RSF_IMASK  = 0xFF00
};

#define RES_is_operator(value) (COMP_res_info[value].flag & RSF_OP)
#define RES_is_type(value) (COMP_res_info[value].flag & RSF_TYPE)
#define RES_is_assignment(value) (COMP_res_info[value].flag & RSF_ASGN)
#define RES_is_only(value) (COMP_res_info[value].flag & RSF_ONLY)
#define RES_get_ident_flag(value) (COMP_res_info[value].flag & RSF_IMASK)

#define RES_priority(_res) (COMP_res_info[_res].priority)

#define RES_is_unary(value) ((COMP_res_info[value].flag & 0x30) == RSF_UNARY)
#define RES_is_binary(value) ((COMP_res_info[value].flag & 0x30) == RSF_BINARY)
#define RES_is_n_ary(value) ((COMP_res_info[value].flag & 0x30) == RSF_N_ARY)
#define RES_is_post(value) ((COMP_res_info[value].flag & 0x30) == RSF_POST)

#define RES_get_type(_res) (COMP_res_info[_res].value)
#define RES_get_assignment_operator(_res) (COMP_res_info[_res].value)

typedef
  enum {
    RS_NONE,
    RS_BOOLEAN,
    RS_BYTE,
    RS_DATE,
    RS_SINGLE,
    RS_FLOAT,
    RS_INTEGER,
    RS_LONG,
    RS_SHORT,
    RS_STRING,
    RS_VARIANT,
    RS_OBJECT,
    RS_POINTER,
    RS_CLASS,
    RS_FUNCTION,
    RS_STRUCT,
    RS_CONST,
    RS_PRIVATE,
    RS_PUBLIC,
    RS_STATIC,
    RS_PROPERTY,
    RS_EVENT,
    RS_INHERITS,
    RS_IMPLEMENTS,
    RS_EXPORT,
    RS_AS,
    RS_OF,
    RS_DIM,
    RS_NEW,
    RS_PROCEDURE,
    RS_SUB,
    RS_RETURN,
    RS_OPTIONAL,
    RS_OUTPUT,
    RS_DO,
    RS_LOOP,
    RS_WHILE,
    RS_UNTIL,
    RS_REPEAT,
    RS_WEND,
    RS_IF,
    RS_THEN,
    RS_ELSE,
    RS_ENDIF,
    RS_END,
    RS_FOR,
    RS_TO,
    RS_FROM,
    RS_STEP,
    RS_NEXT,
    RS_SELECT,
    RS_CASE,
    RS_EXIT,
    RS_BREAK,
    RS_CONTINUE,
    RS_GOTO,
    RS_ME,
    RS_LAST,
    RS_TRY,
    RS_FINALLY,
    RS_CATCH,
    RS_WITH,
    RS_TRUE,
    RS_FALSE,
    RS_SWAP,
    RS_NULL,
    RS_EXTERN,
    RS_EACH,
    RS_IN,
    RS_DEFAULT,
    RS_STOP,
    RS_QUIT,
    RS_RAISE,
    RS_ERROR,
    RS_SUPER,
    RS_ENUM,
		RS_LET,

    RS_PRINT,
    RS_INPUT,
    RS_READ,
    RS_WRITE,
    RS_OPEN,
    RS_CLOSE,
    RS_SEEK,
    RS_APPEND,
    RS_CREATE,
    RS_BINARY,
    RS_LINE,
    RS_FLUSH,
    RS_EXEC,
    RS_SHELL,
    RS_WAIT,
    RS_SLEEP,
    RS_KILL,
    RS_MOVE,
    RS_COPY,
    RS_INC,
    RS_DEC,
    RS_MKDIR,
    RS_RMDIR,
    RS_WATCH,
    RS_LINK,
    RS_LOCK,
    RS_UNLOCK,
    RS_LIBRARY,
    RS_DEBUG,
    RS_PIPE,
    RS_RANDOMIZE,
    RS_BYREF,
		RS_MEMORY,

    RS_COLON,
    RS_SCOLON,
    RS_COMMA,
    RS_3PTS,
    RS_SHARP,
    RS_AT,
    RS_QUES,
    RS_LBRC,
    RS_RBRC,
    RS_EQUAL,
    RS_NEAR,
    RS_LBRA,
    RS_RBRA,
    RS_PT,
    RS_EXCL,
    RS_PLUS,
    RS_MINUS,
    RS_STAR,
    RS_SLASH,
    RS_FLEX,
    RS_AMP,
    RS_GT,
    RS_LT,
    RS_GE,
    RS_LE,
    RS_NE,
    RS_LSQR,
    RS_RSQR,
    RS_AND,
    RS_OR,
    RS_NOT,
    RS_XOR,
    RS_BSLASH,
    RS_DIV,
    RS_MOD,
    RS_IS,
    RS_LIKE,
    RS_BEGINS,
    RS_ENDS,
    RS_FILE,

    RS_PLUS_EQ,
    RS_MINUS_EQ,
    RS_STAR_EQ,
    RS_SLASH_EQ,
    RS_DIV_EQ,
    RS_AMP_EQ,
    RS_FILE_EQ,
    }
  RESERVED_ID;

enum
{
  OP_NONE  ,
  OP_COLON ,
  OP_EQUAL ,
  OP_NEAR  ,
  OP_LBRA  ,
  OP_RBRA  ,
  OP_PT    ,
  OP_EXCL  ,
  OP_COMMA ,
  OP_3PTS  ,
  OP_PLUS  ,
  OP_MINUS ,
  OP_STAR  ,
  OP_SLASH ,
  OP_FLEX  ,
  OP_AMP   ,
  OP_GT    ,
  OP_LT    ,
  OP_GE    ,
  OP_LE    ,
  OP_NE    ,
  OP_LSQR  ,
  OP_RSQR  ,
  OP_AND   ,
  OP_OR    ,
  OP_NOT   ,
  OP_XOR   ,
  OP_DIV   ,
  OP_MOD   ,
  OP_IS    ,
  OP_LIKE  ,
  OP_FILE
};

typedef
  struct {
    const char *name;
    short flag;
    short value;
    short priority;
    short code;
		short subcode;
    void (*func)();
    }
  PACKED
  COMP_INFO;

typedef
  struct {
    const char *name;
    ushort opcode;
    ushort optype;
    short min_param;
    short max_param;
    }
  PACKED
  SUBR_INFO;

/*
typedef
  struct {
    short index;
    char *name;
    }
  PACKED
  CONST_INFO;
*/

#ifndef __RESERVED_C

EXTERN COMP_INFO COMP_res_info[];
EXTERN SUBR_INFO COMP_subr_info[];

EXTERN TABLE *COMP_res_table;
EXTERN TABLE *COMP_subr_table;

EXTERN int SUBR_VarPtr;
EXTERN int SUBR_Mid;
EXTERN int SUBR_MidS;

#endif

void RESERVED_init(void);
void RESERVED_exit(void);

int RESERVED_find_word(const char *word, int len);

SUBR_INFO *SUBR_get(const char *subr_name);
SUBR_INFO *SUBR_get_from_opcode(ushort opcode, ushort optype);
//int SUBR_get_index(const char *subr_name);

#endif
