/***************************************************************************

  gb_reserved_temp.h

  Template for reserved keywords table and subroutines table.

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

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_pcode.h"
#include "gb_type_common.h"
#include "gb_reserved.h"

/* If this file is modified, don't forget to update GAMBAS_PCODE_VERSION in acinclude.m4 */

PUBLIC COMP_INFO COMP_res_info[] =
{
  { "" },

  { "Boolean",      RSF_TYPE,     T_BOOLEAN         },
  { "Byte",         RSF_TYPE,     T_BYTE            },
  { "Date",         RSF_TYPE,     T_DATE            },
  { "Single",       RSF_TYPE,     T_SINGLE          },
  { "Float",        RSF_TYPE,     T_FLOAT           },
  { "Integer",      RSF_TYPE,     T_INTEGER         },
  { "Long",         RSF_TYPE,     T_LONG            },
  { "Short",        RSF_TYPE,     T_SHORT           },
  { "String",       RSF_TYPE,     T_STRING          },
  { "Variant",      RSF_TYPE,     T_VARIANT         },
  { "Object",       RSF_TYPE,     T_OBJECT          },
  { "Pointer",      RSF_TYPE,     T_POINTER         },
  { "CLASS"                                         },
  { "FUNCTION",     RSF_ILF                         },
  { "STRUCT"                                        },
  { "CONST"                                         },
  { "PRIVATE",      RSF_ILD                         },
  { "PUBLIC",       RSF_ILD                         },
  { "STATIC"                                        },
  { "PROPERTY",     RSF_ILD                         },
  { "EVENT",        RSF_ILE|RSF_ILF                 },
  { "INHERITS",     RSF_ILD|RSF_ILT                 },
  { "IMPLEMENTS"                                    },
  { "EXPORT"                                        },
  { "AS",           RSF_ILT                         },
  { "OF"                                            },
  { "DIM",          RSF_ILD                         },
  { "NEW",          RSF_ILT                         },
  { "PROCEDURE",    RSF_ILF                         },
  { "SUB",          RSF_ILF                         },
  { "RETURN"                                        },
  { "OPTIONAL"                                      },
  { "OUTPUT"                                        },
  { "DO"                                            },
  { "LOOP"                                          },
  { "WHILE"                                         },
  { "UNTIL"                                         },
  { "REPEAT"                                        },
  { "WEND"                                          },
  { "IF"                                            },
  { "THEN"                                          },
  { "ELSE"                                          },
  { "ENDIF"                                         },
  { "END"                                           },
  { "FOR"                                           },
  { "TO"                                            },
  { "FROM"                                          },
  { "STEP"                                          },
  { "NEXT"                                          },
  { "SELECT"                                        },
  { "CASE"                                          },
  { "EXIT"                                          },
  { "BREAK"                                         },
  { "CONTINUE"                                      },
  { "GOTO"                                          },
  { "ME"                                            },
  { "LAST"                                          },
  { "TRY"                                           },
  { "FINALLY"                                       },
  { "CATCH"                                         },
  { "WITH"                                          },
  { "TRUE"                                          },
  { "FALSE"                                         },
  { "SWAP"                                          },
  { "NULL"                                          },
  { "EXTERN"                                        },
  { "EACH"                                          },
  { "IN"                                            },
  { "DEFAULT"                                       },
  { "STOP"                                          },
  { "QUIT"                                          },
  { "RAISE",        RSF_ILE|RSF_ILF                 },
  { "ERROR"                                         },
  { "SUPER"                                         },
  { "ENUM"                                          },

  { "PRINT"                                         },
  { "INPUT"                                         },
  { "READ",         RSF_ILD                         },
  { "WRITE"                                         },
  { "OPEN"                                          },
  { "CLOSE"                                         },
  { "SEEK"                                          },
  { "APPEND"                                        },
  { "CREATE"                                        },
  { "BINARY"                                        },
  { "LINE"                                          },
  { "FLUSH"                                         },
  { "EXEC"                                          },
  { "SHELL"                                         },
  { "WAIT"                                          },
  { "SLEEP"                                         },
  { "KILL"                                          },
  { "MOVE"                                          },
  { "COPY"                                          },
  { "INC"                                           },
  { "DEC"                                           },
  { "MKDIR"                                         },
  { "RMDIR"                                         },
  { "WATCH"                                         },
  { "LINK"                                          },
  { "LOCK"                                          },
  { "UNLOCK"                                        },
  { "LIBRARY"                                       },
  { "DEBUG"                                         },
  { "PIPE"																					},
  { "RANDOMIZE"																			},

  { ":"                                                                   },
  { ";"                                                                   },
  { ","                                                                   },
  { "..."                                                                 },
  { "#"                                                                   },
  { "@"                                                                   },
  { "?"                                                                   },
  { "{"                                                                   },
  { "}"                                                                   },
  { "=",            RSF_OP2S,               OP_EQUAL,     4,    C_EQ      },
  { "==",           RSF_OP2S,               OP_NEAR,      4,    C_NEAR    },
  { "(",            RSF_OPP,                OP_LBRA,      12              },
  { ")",                                                                  },
  { ".",            RSF_OP2|RSF_INF,        OP_PT,        20              },
  { "!",            RSF_OP2|RSF_INF,        OP_EXCL,      20              },
  { "+",            RSF_OP2,                OP_PLUS,      5,    C_ADD     },
  { "-",            RSF_OP2,                OP_MINUS,     5,    C_SUB     },
  { "*",            RSF_OP2,                OP_STAR,      6,    C_MUL     },
  { "/",            RSF_OP2,                OP_SLASH,     6,    C_DIV     },
  { "^",            RSF_OP2S,               OP_FLEX,      7,    C_POW     },
  { "&",            RSF_OPN,                OP_AMP,       9,    C_CAT     },
  { ">",            RSF_OP2S,               OP_GT,        4,    C_GT      },
  { "<",            RSF_OP2S,               OP_LT,        4,    C_LT      },
  { ">=",           RSF_OP2S,               OP_GE,        4,    C_GE      },
  { "<=",           RSF_OP2S,               OP_LE,        4,    C_LE      },
  { "<>",           RSF_OP2S,               OP_NE,        4,    C_NE      },
  { "[",            RSF_OPP,                OP_LSQR,      12              },
  { "]",            RSF_NONE,               OP_RSQR                       },
  { "AND",          RSF_OP2SM,              OP_AND,       2,    C_AND     },
  { "OR",           RSF_OP2SM,              OP_OR,        2,    C_OR      },
  { "NOT",          RSF_OP1,                OP_NOT,       10,   C_NOT     },
  { "XOR",          RSF_OP2SM,              OP_XOR,       2,    C_XOR     },
  { "\\",           RSF_OP2S,               OP_DIV,       6,    C_QUO     },
  { "DIV",          RSF_OP2S,               OP_DIV,       6,    C_QUO     },
  { "MOD",          RSF_OP2S,               OP_MOD,       6,    C_REM     },
  { "IS",           RSF_OP2|RSF_ILT,        OP_IS,        11,   C_IS      },
  { "LIKE",         RSF_OP2S,               OP_LIKE,      4,    C_LIKE    },
  { "&/",           RSF_OPN,                OP_FILE,      8,    C_FILE    },

  { "+=",           RSF_ASGN,               RS_PLUS                       },
  { "-=",           RSF_ASGN,               RS_MINUS                      },
  { "*=",           RSF_ASGN,               RS_STAR                       },
  { "/=",           RSF_ASGN,               RS_SLASH                      },
  { "\\=",          RSF_ASGN,               RS_BSLASH                     },
  { "&=",           RSF_ASGN,               RS_AMP                        },
  { "&/=",          RSF_ASGN,               RS_FILE                       },

  { NULL }
};


PUBLIC SUBR_INFO COMP_subr_info[] =
{
  { "Left$",              0,  0,  1,  2 },
  { "Left",               0,  0,  1,  2 },

  { "Mid$",               1,  0,  2,  3 },
  { "Mid",                1,  0,  2,  3 },

  { "Right$",             2,  0,  1,  2 },
  { "Right",              2,  0,  1,  2 },

  { "Len",                3,  0,  1     },

  { "Space$",             4,  0,  1     },
  { "Space",              4,  0,  1     },

  { "String$",            5,  0,  2     },
  { "String",             5,  0,  2     },

  { "Trim$",              6,  0,  1     },
  { "Trim",               6,  0,  1     },

  { "LTrim$",             6,  1,  1     },
  { "LTrim",              6,  1,  1     },

  { "RTrim$",             6,  2,  1     },
  { "RTrim",              6,  2,  1     },

  { "Upper$",             7,  0,  1     },
  { "Upper",              7,  0,  1     },
  { "UCase$",             7,  0,  1     },
  { "UCase",              7,  0,  1     },

  { "Lower$",             8,  0,  1     },
  { "Lower",              8,  0,  1     },
  { "LCase$",             8,  0,  1     },
  { "LCase",              8,  0,  1     },

  { "Chr$",               9,  0,  1     },
  { "Chr",                9,  0,  1     },

  { "Asc",               10,  0,  1,  2 },

  { "InStr",             11,  0,  2,  4 },

  { "RInStr",            12,  0,  2,  4 },   /* CODE_RINSTR */

  { "Subst$",            13,  0,  1, 63 },
  { "Subst",             13,  0,  1, 63 },

  { "Replace$",          14,  0,  3,  4 },
  { "Replace",           14,  0,  3,  4 },

  { "Split",             15,  0,  1,  4 },
  { "Scan",              16,  0,  2     },

  { "Comp",              17,  0,  2,  3 },

  { "Conv",              18,  0,  3     },
  { "Conv$",             18,  0,  3     },
  { "SConv",             19,  0,  1     },
  { "SConv$",            19,  0,  1     },
  { "DConv",             19,  1,  1     },
  { "DConv$",            19,  1,  1     },

  { "Abs",               20,  0,  1     },   /* CODE_ABS */
  { "Int",               21,  0,  1     },
  { "Fix",               22,  0,  1     },
  { "Sgn",               23,  0,  1     },

  { "Frac",              24,  1,  1     },
  { "Log",               24,  2,  1     },
  { "Exp",               24,  3,  1     },
  { "Sqr",               24,  4,  1     },
  { "Sin",               24,  5,  1     },
  { "Cos",               24,  6,  1     },
  { "Tan",               24,  7,  1     },
  { "Atn",               24,  8,  1     },
  { "ATan",              24,  8,  1     },
  { "Asn",               24,  9,  1     },
  { "ASin",              24,  9,  1     },
  { "Acs",               24, 10,  1     },
  { "ACos",              24, 10,  1     },
  { "Deg",               24, 11,  1     },
  { "Rad",               24, 12,  1     },
  { "Log10",             24, 13,  1     },
  { "Sinh",              24, 14,  1     },
  { "Cosh",              24, 15,  1     },
  { "Tanh",              24, 16,  1     },
  { "Asnh",              24, 17,  1     },
  { "ASinh",             24, 17,  1     },
  { "Acsh",              24, 18,  1     },
  { "ACosh",             24, 18,  1     },
  { "Atnh",              24, 21,  1     },
  { "ATanh",             24, 21,  1     },
  { "Exp2",              24, 20,  1     },
  { "Exp10",             24, 21,  1     },
  { "Log2",              24, 22,  1     },
  { "Cbr",               24, 23,  1     },
  { "Expm",              24, 24,  1     },
  { "Logp",              24, 25,  1     },

  { "Pi",                25,  0,  0,  1 },
  { "Round",             26,  0,  1,  2 },
#ifndef __EVAL_RESERVED_C
  { ".Randomize",        27,  0,  0,  1 },
#endif
  { "Rnd",               28,  0,  0,  2 },
  { "Min",               29,  0,  2,    },
  { "Max",               30,  0,  2,    },   /* CODE_MAX */

  { "If",                31,  0,  3,    },
  { "IIf",               31,  0,  3,    },
  { "Choose",            32,  0,  1, 63 },

  { ".Array",            33,  0,  1, 63 },   /* Needed for Eval("[...]") */

  { "ATan2",             34,  1,  2     },
  { "Atn2",              34,  1,  2     },
  { "Ang",               34,  2,  2     },
  { "Hyp",               34,  3,  2     },
  { "Mag",               34,  3,  2     },

  { "IsAscii",           35,  1,  1     },
  { "IsLetter",          35,  2,  1     },
  { "IsLCase",           35,  3,  1     },
  { "IsLower",           35,  3,  1     },
  { "IsUCase",           35,  4,  1     },
  { "IsUpper",           35,  4,  1     },
  { "IsDigit",           35,  5,  1     },
  { "IsHexa",            35,  6,  1     },
  { "IsSpace",           35,  7,  1     },
  { "IsBlank",           35,  8,  1     },
  { "IsPunct",           35,  9,  1     },

  { "Ascii?",            35,  1,  1     },
  { "Letter?",           35,  2,  1     },
  { "LCase?",            35,  3,  1     },
  { "Lower?",            35,  3,  1     },
  { "UCase?",            35,  4,  1     },
  { "Upper?",            35,  4,  1     },
  { "Digit?",            35,  5,  1     },
  { "Hexa?",             35,  6,  1     },
  { "Space?",            35,  7,  1     },
  { "Blank?",            35,  8,  1     },
  { "Punct?",            35,  9,  1     },

  { "BClr",              36,  1,  2     },
  { "BSet",              36,  2,  2     },
  { "BTst",              36,  3,  2     },
  { "BChg",              36,  4,  2     },
  { "Shl",               36,  5,  2     },
  { "Asl",               36,  5,  2     },
  { "Shr",               36,  6,  2     },
  { "Asr",               36,  6,  2     },
  { "Rol",               36,  7,  2     },
  { "Ror",               36,  8,  2     },
  { "Lsl",               36,  9,  2     },
  { "Lsr",               36, 10,  2     },

  { "IsBoolean",         37,  1,  1     },
  { "IsByte",            37,  2,  1     },
  { "IsShort",           37,  3,  1     },
  { "IsInteger",         37,  4,  1     },
  { "IsLong",            37,  5,  1     },
  { "IsSingle",          37,  6,  1     },
  { "IsFloat",           37,  7,  1     },
  { "IsDate",            37,  8,  1     },
  { "IsString",          37,  9,  1     },
  { "IsNull",            37,  15, 1     },
  { "IsObject",          37,  16, 1     },
  { "IsNumber",          37,  17, 1     },

  { "Boolean?",          37,  1,  1     },
  { "Byte?",             37,  2,  1     },
  { "Short?",            37,  3,  1     },
  { "Integer?",          37,  4,  1     },
  { "Long?",             37,  5,  1     },
  { "Single?",           37,  6,  1     },
  { "Float?",            37,  7,  1     },
  { "Date?",             37,  8,  1     },
  { "String?",           37,  9,  1     },
  { "Null?",             37,  15, 1     },
  { "Object?",           37,  16, 1     },
  { "Number?",           37,  17, 1     },

  { "TypeOf",            38,  0,  1     },

  { "CBool",             39,  1,  1     },
  { "CByte",             39,  2,  1     },
  { "CShort",            39,  3,  1     },
  { "CInt",              39,  4,  1     },
  { "CInteger",          39,  4,  1     },
  { "CLng",              39,  5,  1     },
  { "CLong",             39,  5,  1     },
  { "CSng",              39,  6,  1     },
  { "CSingle",           39,  6,  1     },
  { "CFlt",              39,  7,  1     },
  { "CFloat",            39,  7,  1     },
  { "CDate",             39,  8,  1     },
  { "CStr",              39,  9,  1     },
  { "CString",           39,  9,  1     },

  { "Bin$",              40,  0,  1,  2 },
  { "Bin",               40,  0,  1,  2 },

  { "Hex$",              41,  0,  1,  2 },
  { "Hex",               41,  0,  1,  2 },

  { "Val",               42,  0,  1     },

  { "Str$",              43,  0,  1     },
  { "Str",               43,  0,  1     },

  { "Format$",           44,  0,  1,  2 },
  { "Format",            44,  0,  1,  2 },

  { "Timer",             45,  0,  0     },

  { "Now",               46,  0,  0     },

  { "Year",              47,  1,  1     },
  { "Month",             47,  2,  1     },
  { "Day",               47,  3,  1     },
  { "Hour",              47,  4,  1     },
  { "Minute",            47,  5,  1     },
  { "Second",            47,  6,  1     },
  { "WeekDay",           47,  7,  1     },
  { "Week",              48,  0,  0,  3 },

  { "Date",              49,  0,  0,  6 },
  { "Time",              50,  0,  0,  3 },

  { "DateAdd",           51,  0,  3     },
  { "DateDiff",          51,  1,  3     },

  { "Eval",              52,  0,  1,  2 },

#ifndef __EVAL_RESERVED_C
  { ".Error",            53,  0,  0,  2 },
  { ".Debug",            54,  0,  0     },

  { ".Wait",             55,  0,  0,  1 },

  { ".Open",             56,  0,  2     },
  { ".Close",            57,  0,  1     },
  { ".Input",            58,  0,  0,  1 },
  { ".LineInput",        59,  0,  1     },
  { ".Print",            60,  0,  1, 63 },
  { ".Read",             61,  0,  1,  2 },
  { ".Write",            62,  0,  1,  2 },
  { ".Flush",            63,  0,  1     },

  { ".Lock",             64,  0,  1     },
  { ".Unlock",           64,  1,  1     },

  { ".InputFrom",        65,  0,  1     },
  { ".OutputTo",         65,  1,  1     },
  { ".ErrorTo",          65,  2,  1     },
#endif
  { "Eof",               66,  0,  0,  1 },
  { "Lof",               67,  0,  0,  1 },
  { "Seek",              68,  0,  1,  3 },
#ifndef __EVAL_RESERVED_C
  { ".Kill",             69,  0,  1     },
  { ".Mkdir",            70,  0,  1     },
  { ".Rmdir",            71,  0,  1     },
  { ".Move",             72,  0,  2     },
  { ".Copy",             73,  0,  2     },
  { ".Link",             74,  0,  2     },
#endif
  { "Exist",             75,  0,  1     },
  { "Access",            76,  0,  1,  2 },
  { "Stat",              77,  0,  1,  2 },
  { "Dfree",             78,  0,  1     },

  { "Temp",              79,  0,  0,  1 },
  { "Temp$",             79,  0,  0,  1 },

  { "IsDir",             80,  0,  1     },
  { "Dir?",              80,  0,  1     },

  { "Dir",               81,  0,  1,  3 },
  { "RDir",              82,  0,  1,  3 },

#ifndef __EVAL_RESERVED_C
  { ".Exec",             83,  0,  4     },
  { ".Shell",            83,  1,  4     },
#endif

  { "Alloc",             84,  0,  1,  2 },
  { "Free",              85,  0,  1     },
  { "Realloc",           86,  0,  2,  3 },
  { "StrPtr",            87,  0,  1     },

#ifndef __EVAL_RESERVED_C
  { ".Sleep",            88,  0,  1     },
#endif

  /*
  { "_EventOff",         94,  0,  0     },
  { "_EventOn",          95,  0,  0     },
  */

  { NULL }
};

PUBLIC TABLE *COMP_res_table;
PUBLIC TABLE *COMP_subr_table;

static uchar _operator_table[256] = { 0 };

PUBLIC void RESERVED_init(void)
{
  COMP_INFO *info;
  SUBR_INFO *subr;
  int len;
  int i;
  
  /* Reserved words symbol table */
  
  TABLE_create(&COMP_res_table, 0, TF_IGNORE_CASE);
  for (info = &COMP_res_info[0], i = 0; info->name; info++, i++)
  {
    len = strlen(info->name);
    if (len == 1)
      _operator_table[(uint)*info->name] = i;
    
    TABLE_add_symbol(COMP_res_table, info->name, len, NULL, NULL);
  }
  
  #ifdef DEBUG
  printf("Table des mots r�erv�:\n");
  TABLE_print(COMP_res_table, TRUE);
  #endif

  #if 0
  HASH_TABLE_create(&_reserved_hash, sizeof(short), HF_NORMAL);
  index = 0;
  for (info = &COMP_res_info[0]; info->name; info++)
  {
    strcpy(buffer, info->name);
    len = strlen(buffer);
    for (i = 0; i < len; i++)
      buffer[i] = tolower(buffer[i]);
      
    *((short *)HASH_TABLE_insert(_reserved_hash, buffer, len)) = index;
    index++;
  }
  #endif
  
  /* Table des routines int�r�s */

  TABLE_create(&COMP_subr_table, 0, TF_IGNORE_CASE);
  for (subr = &COMP_subr_info[0]; subr->name; subr++)
  {
    if (subr->max_param == 0)
      subr->max_param = subr->min_param;

    TABLE_add_symbol(COMP_subr_table, subr->name, strlen(subr->name), NULL, NULL);
  }

  #ifdef DEBUG
  printf("Table des routines int�r�s:\n");
  TABLE_print(COMP_subr_table, TRUE);
  #endif

  /* Table des constantes */

  /*
  TABLE_create(&COMP_const_table, 0, TF_IGNORE_CASE);
  for (cst = &COMP_const_info[0]; cst->name; cst++)
    TABLE_add_symbol(COMP_const_table, cst->name, strlen(cst->name), NULL, NULL);
  */
}


PUBLIC void RESERVED_exit(void)
{
  TABLE_delete(&COMP_res_table);
  TABLE_delete(&COMP_subr_table);
}


PUBLIC SUBR_INFO *SUBR_get(const char *subr_name)
{
  int index;

  if (!TABLE_find_symbol(COMP_subr_table, subr_name, strlen(subr_name), NULL, &index))
    return NULL;
  else
    return &COMP_subr_info[index];
}


PUBLIC SUBR_INFO *SUBR_get_from_opcode(ushort opcode, ushort optype)
{
  SUBR_INFO *si;

  for (si = COMP_subr_info; si->name; si++)
  {
    if (si->opcode == opcode)
    {
      if (si->min_param != si->max_param)
        return si;
      else if (si->optype == optype || si->optype == 0)
        return si;
    }
  }

  /*ERROR_panic("SUBR_get_from_opcode: SUBR not found !");*/
  return NULL;
}

PUBLIC int RESERVED_find_word(const char *word, int len)
{
  //const char *res1 = ":;, #@?{}= ().!+-*/^&><   []    \\";
  //const char *p;
  int ind;
      
  switch (len)
  {
    case 1:
      ind = _operator_table[(uint)*word];
      if (ind)
        return ind;
      break;
      
    default:
      if (TABLE_find_symbol(COMP_res_table, word, len, NULL, &ind))
        return ind;
      break;
  }
  
  return -1;
}
