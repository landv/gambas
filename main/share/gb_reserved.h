/***************************************************************************

  gb_reserved.h

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

#ifndef __GB_RESERVED_H
#define __GB_RESERVED_H

#include "gb_common.h"
#include "gb_table.h"

enum
{
	RSF_NONE   = 0x0000,
	RSF_OP     = 0x0001,
	RSF_TYPE   = 0x0002,
	RSF_ASGN   = 0x0004,
	RSF_NOT    = 0x0008,    // operator can have NOT before it
	
	RSF_N_ARY  = 0x0000,
	RSF_UNARY  = 0x0010,
	RSF_BINARY = 0x0020,
	RSF_POST   = 0x0030,
	RSF_ONLY   = 0x0040,
	
	RSF_OPN    = 0x0001,
	RSF_OP1    = 0x0011,
	RSF_OP2    = 0x0021,
	RSF_OPP    = 0x0031,
	RSF_OP2S   = 0x0061,
	RSF_OP2SM  = 0x00A1,

	RSF_POINT  = 0x0100,    // last pattern is a point or an exclamation mark
	RSF_IDENT  = 0x0200,    // last pattern waits for an identifier
	RSF_CLASS  = 0x0400,    // last pattern waits for a class
	RSF_AS     = 0x0800,    // last pattern waits for a datatype
	RSF_PREV   = 0x1000,    // last pattern use the flags of the last last pattern
	RSF_EVENT  = 0x2000,    // last pattern waits for an event name
	RSF_PUB    = 0x4000,    // last pattern is PUBLIC, PRIVATE or STATIC
	
	RSF_IMASK  = 0xFF00
};

#define RES_is_operator(value) (COMP_res_info[value].flag & RSF_OP)
#define RES_is_type(value) (COMP_res_info[value].flag & RSF_TYPE)
#define RES_is_assignment(value) (COMP_res_info[value].flag & RSF_ASGN)
#define RES_is_only(value) (COMP_res_info[value].flag & RSF_ONLY)
#define RES_get_ident_flag(value) (COMP_res_info[value].flag & RSF_IMASK)
#define RES_get_read_switch(value) (COMP_res_info[value].read_switch)

#define RES_priority(_res) (COMP_res_info[_res].priority)

#define RES_is_unary(value) ((COMP_res_info[value].flag & 0x30) == RSF_UNARY)
#define RES_is_binary(value) ((COMP_res_info[value].flag & 0x30) == RSF_BINARY)
#define RES_is_n_ary(value) ((COMP_res_info[value].flag & 0x30) == RSF_N_ARY)
#define RES_is_post(value) ((COMP_res_info[value].flag & 0x30) == RSF_POST)

#define RES_get_type(_res) (COMP_res_info[_res].value)
#define RES_get_assignment_operator(_res) (COMP_res_info[_res].value)

#define RES_can_have_not_before(value) (COMP_res_info[value].flag & RSF_NOT)

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
		RS_FAST,
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
		RS_DOWNTO,
		RS_FROM,
		RS_STEP,
		RS_NEXT,
		RS_SELECT,
		RS_CASE,
		RS_EXIT,
		RS_BREAK,
		RS_CONTINUE,
		RS_GOTO,
		RS_GOSUB,
		RS_ON,
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
		RS_PINF,
		RS_MINF,
		RS_USE,

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
		RS_CHMOD,
		RS_CHOWN,
		RS_CHGRP,
		
		RS_P_IF,
		RS_P_ELSE,
		RS_P_ENDIF,
		RS_P_CONST,
		RS_P_LINE,

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
		RS_FILE,
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
		RS_PERCENT,
		RS_MOD,
		RS_IS,
		RS_NOT_IS,
		RS_LIKE,
		RS_NOT_LIKE,
		RS_BEGINS,
		RS_NOT_BEGINS,
		RS_ENDS,
		RS_NOT_ENDS,

		RS_PLUS_EQ,
		RS_MINUS_EQ,
		RS_STAR_EQ,
		RS_SLASH_EQ,
		RS_DIV_EQ,
		RS_MOD_EQ,
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
	OP_FILE  ,
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
	OP_LIKE
};

typedef
	struct {
		const char *name;
		short flag;
		unsigned char value;
		unsigned char read_switch;
		unsigned short priority;
		short code;
		short subcode;
		void (*func)();
		}
	COMP_INFO;

typedef
	struct {
		const char *name;
		ushort opcode;
		ushort optype;
		short min_param;
		short max_param;
		}
	SUBR_INFO;

#ifndef __RESERVED_C

EXTERN COMP_INFO COMP_res_info[];
EXTERN SUBR_INFO COMP_subr_info[];

//EXTERN TABLE *COMP_res_table;
//EXTERN TABLE *COMP_subr_table;

EXTERN int SUBR_VarPtr;
EXTERN int SUBR_IsMissing;
EXTERN int SUBR_Mid;
EXTERN int SUBR_MidS;

#endif

void RESERVED_init(void);
void RESERVED_exit(void);

int RESERVED_find_word(const char *word, int len);
int RESERVED_find_subr(const char *word, int len);

SUBR_INFO *SUBR_get(const char *subr_name);
SUBR_INFO *SUBR_get_from_opcode(ushort opcode, ushort optype);

#endif
