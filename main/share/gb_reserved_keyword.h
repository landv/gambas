/***************************************************************************

  gb_reserved_keyword.h

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

#ifndef __GB_RESERVED_KEYWORD_H
#define __GB_RESERVED_KEYWORD_H

COMP_INFO COMP_res_info[] =
{
	{ "" },

	{ "Boolean",      RSF_TYPE,                 T_BOOLEAN,  8     },
	{ "Byte",         RSF_TYPE,                 T_BYTE,     8     },
	{ "Date",         RSF_TYPE,                 T_DATE,     8     },
	{ "Single",       RSF_TYPE,                 T_SINGLE,   8     },
	{ "Float",        RSF_TYPE,                 T_FLOAT,    8     },
	{ "Integer",      RSF_TYPE,                 T_INTEGER,  8     },
	{ "Long",         RSF_TYPE,                 T_LONG,     8     },
	{ "Short",        RSF_TYPE,                 T_SHORT,    8     },
	{ "String",       RSF_TYPE,                 T_STRING,   8     },
	{ "Variant",      RSF_TYPE,                 T_VARIANT,  8     },
	{ "Object",       RSF_TYPE,                 T_OBJECT,   8     },
	{ "Pointer",      RSF_TYPE,                 T_POINTER,  8     },
	{ "Class",        RSF_IDENT,                0,          2     },
	{ "Function",     RSF_IDENT,                0,          4     },
	{ "Struct",       RSF_PREV,                 0,          3     },
	{ "Const",        RSF_IDENT,                0,          5     },
	{ "Private",      RSF_IDENT|RSF_PUB,                          },
	{ "Public",       RSF_IDENT|RSF_PUB,                          },
	{ "Static",       RSF_PUB                                     },
	{ "Fast",         RSF_PUB                                     },
	{ "Property",     RSF_IDENT                                   },
	{ "Event",        RSF_IDENT|RSF_EVENT                         },
	{ "Inherits",     RSF_CLASS|RSF_AS                            },
	{ "Implements"                                                },
	{ "Export"                                                    },
	{ "As",           RSF_AS                                      },
	{ "Of"                                                        },
	{ "Dim",          RSF_IDENT,                                  },
	{ "New",          RSF_AS ,                  0,          1     },
	{ "Procedure",    RSF_IDENT,                0,          4     },
	{ "Sub",          RSF_IDENT,                0,          4     },
	{ "Return"                                                    },
	{ "Optional"                                                  },
	{ "Output"                                                    },
	{ "Do"                                                        },
	{ "Loop"                                                      },
	{ "While"                                                     },
	{ "Until"                                                     },
	{ "Repeat"                                                    },
	{ "Wend"                                                      },
	{ "If"                                                        },
	{ "Then"                                                      },
	{ "Else"                                                      },
	{ "Endif"                                                     },
	{ "End"                                                       },
	{ "For"                                                       },
	{ "To"                                                        },
	{ "DownTo"                                                    },
	{ "From"                                                      },
	{ "Step"                                                      },
	{ "Next"                                                      },
	{ "Select"                                                    },
	{ "Case"                                                      },
	{ "Exit"                                                      },
	{ "Break"                                                     },
	{ "Continue"                                                  },
	{ "Goto"                                                      },
	{ "GoSub"                                                     },
	{ "On"                                                        },
	{ "Me",           0,                        0,          1     },
	{ "Last",         0,                        0,          1     },
	{ "Try"                                                       },
	{ "Finally"                                                   },
	{ "Catch"                                                     },
	{ "With"                                                      },
	{ "True"                                                      },
	{ "False"                                                     },
	{ "Swap"                                                      },
	{ "Null"                                                      },
	{ "Extern",       RSF_IDENT,                0,          5     },
	{ "Each"                                                      },
	{ "In"                                                        },
	{ "Default"                                                   },
	{ "Stop"                                                      },
	{ "Quit"                                                      },
	{ "Raise",        RSF_IDENT|RSF_EVENT                         },
	{ "Error"                                                     },
	{ "Super",        0,                        0,          1     },
	{ "Enum",         0,                        0,          6     },
	{ "Let"                                                       },
	{ "+Inf"                                                      },
	{ "-Inf"                                                      },
	{ "Use"                                                       },

	{ "Print"                                                     },
	{ "Input"                                                     },
	{ "Read",         RSF_PREV,                 0,          7     },
	{ "Write"                                                     },
	{ "Open"                                                      },
	{ "Close"                                                     },
	{ "Seek"                                                      },
	{ "Append"                                                    },
	{ "Create"                                                    },
	{ "Binary"                                                    },
	{ "Line"                                                      },
	{ "Flush"                                                     },
	{ "Exec"                                                      },
	{ "Shell"                                                     },
	{ "Wait"                                                      },
	{ "Sleep"                                                     },
	{ "Kill"                                                      },
	{ "Move"                                                      },
	{ "Copy"                                                      },
	{ "Inc"                                                       },
	{ "Dec"                                                       },
	{ "Mkdir"                                                     },
	{ "Rmdir"                                                     },
	{ "Watch"                                                     },
	{ "Link"                                                      },
	{ "Lock"                                                      },
	{ "Unlock"                                                    },
	{ "Library"                                                   },
	{ "Debug"                                                     },
	{ "Pipe"                                                      },
	{ "Randomize"                                                 },
	{ "ByRef"                                                     },
	{ "Memory"                                                    },
	{ "Chmod"                                                     },
	{ "Chown"                                                     },
	{ "Chgrp"                                                     },
	
	{ "#If"                                                       },
	{ "#Else"                                                     },
	{ "#Endif"                                                    },
	{ "#Const"                                                    },
	{ "#Line"                                                     },

	{ ":",            RSF_NONE,                 OP_COLON,     0,                       },  // Use for the immediate collection syntax
	{ ";"                                                                              },
	{ ","                                                                              },
	{ "..."                                                                            },
	{ "#"                                                                              },
	{ "@"                                                                              },
	{ "?"                                                                              },
	{ "{"                                                                              },
	{ "}"                                                                              },
	{ "=",            RSF_OP2S,                 OP_EQUAL,     0,   4,    C_EQ          },
	{ "==",           RSF_OP2S,                 OP_NEAR,      0,   4,    C_NEAR        },
	{ "(",            RSF_OPP,                  OP_LBRA,      0,   12                  },
	{ ")",                                                                             },
	{ ".",            RSF_OP2|RSF_POINT,        OP_PT,        0,   20                  },
	{ "!",            RSF_OP2|RSF_POINT,        OP_EXCL,      0,   20                  },
	{ "+",            RSF_OP2,                  OP_PLUS,      0,   5,    C_ADD         },
	{ "-",            RSF_OP2,                  OP_MINUS,     0,   5,    C_SUB         },
	{ "*",            RSF_OP2,                  OP_STAR,      0,   6,    C_MUL         },
	{ "/",            RSF_OP2,                  OP_SLASH,     0,   6,    C_DIV         },
	{ "^",            RSF_OP2S,                 OP_FLEX,      0,   7,    C_POW         },
	{ "&",            RSF_OPN,                  OP_AMP,       0,   9,    C_CAT         },
	{ "&/",           RSF_OPN,                  OP_FILE,      0,   8,    C_FILE        },
	{ ">",            RSF_OP2S,                 OP_GT,        0,   4,    C_GT          },
	{ "<",            RSF_OP2S,                 OP_LT,        0,   4,    C_LT          },
	{ ">=",           RSF_OP2S,                 OP_GE,        0,   4,    C_GE          },
	{ "<=",           RSF_OP2S,                 OP_LE,        0,   4,    C_LE          },
	{ "<>",           RSF_OP2S,                 OP_NE,        0,   4,    C_NE          },
	{ "[",            RSF_OPP,                  OP_LSQR,      0,   12                  },
	{ "]",            RSF_NONE,                 OP_RSQR                                },  // Use for the immediate array syntax
	{ "And",          RSF_OP2SM,                OP_AND,       0,   2,    C_AND         },
	{ "Or",           RSF_OP2SM,                OP_OR,        0,   2,    C_OR          },
	{ "Not",          RSF_OP1,                  OP_NOT,       0,   10,   C_NOT         },
	{ "Xor",          RSF_OP2SM,                OP_XOR,       0,   2,    C_XOR         },
	{ "\\",           RSF_OP2S,                 OP_DIV,       0,   6,    C_QUO         },
	{ "Div",          RSF_OP2S,                 OP_DIV,       0,   6,    C_QUO         },
	{ "%",            RSF_OP2S,                 OP_MOD,       0,   6,    C_REM         },
	{ "Mod",          RSF_OP2S,                 OP_MOD,       0,   6,    C_REM         },
	{ "Is",           RSF_OP2|RSF_AS|RSF_NOT,   OP_IS,        0,   11,   C_IS,   0     },
	{ "",             RSF_OP2|RSF_AS,           OP_IS,        0,   11,   C_IS,   1     },
	{ "Like",         RSF_OP2S|RSF_NOT,         OP_LIKE,      0,   4,    C_LIKE, 0     },
	{ "",             RSF_OP2S,                 OP_LIKE,      0,   4,    C_LIKE, 4     }, // NOT LIKE
	{ "Begins",       RSF_OP2S|RSF_NOT,         OP_LIKE,      0,   4,    C_LIKE, 1     },
	{ "",             RSF_OP2S,                 OP_LIKE,      0,   4,    C_LIKE, 5     }, // NOT BEGINS
	{ "Ends",         RSF_OP2S|RSF_NOT,         OP_LIKE,      0,   4,    C_LIKE, 2     },
	{ "",             RSF_OP2S,                 OP_LIKE,      0,   4,    C_LIKE, 6     }, // NOT ENDS
	{ "Match",        RSF_OP2S|RSF_NOT,         OP_LIKE,      0,   4,    C_LIKE, 3     },
	{ "",             RSF_OP2S,                 OP_LIKE,      0,   4,    C_LIKE, 7     }, // NOT MATCH

	{ "+=",           RSF_ASGN,                 RS_PLUS                                },
	{ "-=",           RSF_ASGN,                 RS_MINUS                               },
	{ "*=",           RSF_ASGN,                 RS_STAR                                },
	{ "/=",           RSF_ASGN,                 RS_SLASH                               },
	{ "\\=",          RSF_ASGN,                 RS_BSLASH                              },
	{ "%=",           RSF_ASGN,                 RS_PERCENT                             },
	{ "&=",           RSF_ASGN,                 RS_AMP                                 },
	{ "&/=",          RSF_ASGN,                 RS_FILE                                },
	{ "^=",           RSF_ASGN,                 RS_FLEX                                },

	{ NULL }
};

SUBR_INFO COMP_subr_info[] =
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

	{ "Lower$",             7,  1,  1     },
	{ "Lower",              7,  1,  1     },
	{ "LCase$",             7,  1,  1     },
	{ "LCase",              7,  1,  1     },
	
	// 8 is available

	{ "Chr$",               9,  0,  1     },
	{ "Chr",                9,  0,  1     },

	{ "Asc",               10,  0,  1,  2 },

	{ "InStr",             11,  0,  2,  4 },

	{ "RInStr",            12,  0,  2,  4 },   /* CODE_RINSTR */

	{ "Subst$",            13,  0,  1, 63 },
	{ "Subst",             13,  0,  1, 63 },

	{ "Replace$",          14,  0,  3,  4 },
	{ "Replace",           14,  0,  3,  4 },

	{ "Split",             15,  0,  1,  5 },
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
	{ "Atnh",              24, 19,  1     },
	{ "ATanh",             24, 19,  1     },
	{ "Exp2",              24, 20,  1     },
	{ "Exp10",             24, 21,  1     },
	{ "Log2",              24, 22,  1     },
	{ "Cbr",               24, 23,  1     },
	{ "Expm",              24, 24,  1     },
	{ "Logp",              24, 25,  1     },
	{ "Floor",             24, 26,  1     },
	{ "Ceil",              24, 27,  1     },

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

	{ ".Array",            33,  0,  0, 63 },   /* Needed for Eval("[...]") */

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
	{ "IsInteger",         37,  4,  1     },
	{ "IsLong",            37,  5,  1     },
	{ "IsFloat",           37,  7,  1     },
	{ "IsDate",            37,  8,  1     },
	{ "IsNumber",          37,  14, 1     },
	{ "IsNull",            37,  15, 1     },

	{ "TypeOf",            38,  0,  1     },
	{ "SizeOf",            38,  1,  1     },

	{ "CBool",             39,  1,  1     },  // CODE_CONV
	{ "CBoolean",          39,  1,  1     },
	{ "CByte",             39,  2,  1     },
	{ "CShort",            39,  3,  1     },
	{ "CInt",              39,  4,  1     },
	{ "CInteger",          39,  4,  1     },
	{ "CLong",             39,  5,  1     },
	{ "CSingle",           39,  6,  1     },
	{ "CFloat",            39,  7,  1     },
	{ "CDate",             39,  8,  1     },
	{ "CStr",              39,  9,  1     },
	{ "CString",           39,  9,  1     },
	{ "CPointer",          39, 11,  1     },
	{ "CVariant",          39, 12,  1     },

	{ "Bin$",              40,  0,  1,  2 },  // CODE_BIN
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

	{ "Date",              49,  0,  0,  7 },
	{ "Time",              50,  0,  0,  4 },

	{ "DateAdd",           51,  0,  3     },
	{ "DateDiff",          51,  1,  3     },

	{ "Eval",              52,  0,  1,  2 },

#ifndef __EVAL_RESERVED_C
	{ ".Error",            53,  0,  0,  2 },
	{ ".Debug",            54,  0,  0     },

	{ ".Wait",             55,  0,  0,  1 },

	{ ".Open",             56,  0,  2     },
	{ ".OpenMemory",       56,  1,  2     },
	{ ".Close",            57,  0,  1     },
	{ ".Input",            58,  0,  0,  1 },
	{ ".LineInput",        59,  0,  1     },
	{ ".Print",            60,  0,  1, 63 },
	{ ".Read",             61,  0,  2,    },
	{ ".ReadBytes",        61,  1,  2,    },
	{ ".Write",            62,  0,  3,    },
	{ ".WriteBytes",       62,  1,  3,    },
	{ ".Flush",            63,  0,  1     },

	{ ".Lock",             64,  0,  1     },
	{ ".Unlock",           64,  1,  1     },
	{ ".LockWait",         64,  2,  2     },

	{ ".InputFrom",        65,  0,  1     },
	{ ".OutputTo",         65,  1,  1     },
	{ ".ErrorTo",          65,  2,  1     },
#endif
	{ "Eof",               66,  0,  0,  1 },
	{ "Lof",               67,  0,  0,  1 },
	{ "Seek",              68,  0,  1,  3 },
#ifndef __EVAL_RESERVED_C
	{ ".Kill",             69,  0,  1     },
	{ ".Mkdir",            69,  1,  1     },
	{ ".Rmdir",            69,  2,  1     },
#endif
	//{ ".Mkdir",            70,  0,  1     }, // The old Mkdir from 3.0
	{ "Even",              70,  1,  1     },
	{ "Odd",               70,  2,  1     },
	//{ ".Rmdir",            71,  0,  1     }, // The old Rmdir from 3.0
	{ "Rand",              71,  0,  1,  2 },
#ifndef __EVAL_RESERVED_C
	{ ".Move",             72,  0,  2     },
	{ ".Copy",             72,  1,  2     },
	{ ".Link",             72,  2,  2     },
	{ ".Chmod",            72,  3,  2     },
	{ ".Chown",            72,  4,  2     },
	{ ".Chgrp",            72,  5,  2     },
#endif
	{ "Swap",              73,  0,  1,  2 }, // at least one argument, because 73 is a deprecated Copy() too.
	{ "Swap$",             73,  0,  1,  2 },
	
	{ "IsNan",             74,  1,  1     },
	{ "IsInf",             74,  2,  1     },
	
	{ "Exist",             75,  0,  1,  2 },
	{ "Access",            76,  0,  1,  2 },
	{ "Stat",              77,  0,  1,  2 },
	{ "Dfree",             78,  0,  1     },

	{ "Temp",              79,  0,  0,  1 },
	{ "Temp$",             79,  0,  0,  1 },

	{ "IsDir",             80,  0,  1     },

	{ "Dir",               81,  0,  1,  3 },
	{ "RDir",              82,  0,  1,  4 },

#ifndef __EVAL_RESERVED_C
	{ ".Exec",             83,  0,  4     },
	{ ".Shell",            83,  1,  4     },
#endif

	{ "Alloc",             84,  0,  1,  2 },
	{ "Free",              85,  0,  1     },
	{ "Realloc",           86,  0,  2,  3 },
	{ "Str@",              87,  0,  1,  2 },
	{ "String@",           87,  0,  1,  2 },

#ifndef __EVAL_RESERVED_C
	{ ".Sleep",            88,  0,  1     },
	{ ".Use",              88,  1,  1     },
	{ ".CheckExec",        88,  2,  1     },
#endif

	{ "VarPtr",            89,  0,  1     },
	{ "IsMissing",         89,  1,  1     },

	{ ".Collection",       90,  0,  1, 63 },
	
	{ "Tr",                91,  0,  1     },
	{ "Tr$",               91,  0,  1     },
	
	{ "Quote",             92,  0,  1     },
	{ "Quote$",            92,  0,  1     },
	{ "Shell",             92,  1,  1     },
	{ "Shell$",            92,  1,  1     },
	{ "Html",              92,  2,  1     },
	{ "Html$",             92,  2,  1     },
	{ "Base64",            92,  3,  1     },
	{ "Base64$",           92,  3,  1     },
	{ "Url",               92,  4,  1     },
	{ "Url$",              92,  4,  1     },

	{ "UnQuote",           93,  0,  1     },
	{ "UnQuote$",          93,  0,  1     },
	{ "UnBase64",          93,  1,  1     },
	{ "UnBase64$",         93,  1,  1     },
	{ "FromBase64",        93,  1,  1     },
	{ "FromBase64$",       93,  1,  1     },
	{ "FromUrl",           93,  2,  1     },
	{ "FromUrl$",          93,  2,  1     },

	{ "MkBool",            94,  1,  1     },
	{ "MkBool$",           94,  1,  1     },
	{ "MkBoolean",         94,  1,  1     },
	{ "MkBoolean$",        94,  1,  1     },
	{ "MkByte",            94,  2,  1     },
	{ "MkByte$",           94,  2,  1     },
	{ "MkShort",           94,  3,  1     },
	{ "MkShort$",          94,  3,  1     },
	{ "MkInt",             94,  4,  1     },
	{ "MkInt$",            94,  4,  1     },
	{ "MkInteger",         94,  4,  1     },
	{ "MkInteger$",        94,  4,  1     },
	{ "MkLong",            94,  5,  1     },
	{ "MkLong$",           94,  5,  1     },
	{ "MkSingle",          94,  6,  1     },
	{ "MkSingle$",         94,  6,  1     },
	{ "MkFloat",           94,  7,  1     },
	{ "MkFloat$",          94,  7,  1     },
	{ "MkDate",            94,  8,  1     },
	{ "MkDate$",           94,  8,  1     },
	{ "MkPointer",         94, 11,  1     },
	{ "MkPointer$",        94, 11,  1     },

	{ "Bool@",             95,  1,  1     },
	{ "Boolean@",          95,  1,  1     },
	{ "Byte@",             95,  2,  1     },
	{ "Short@",            95,  3,  1     },
	{ "Int@",              95,  4,  1     },
	{ "Integer@",          95,  4,  1     },
	{ "Long@",             95,  5,  1     },
	{ "Single@",           95,  6,  1     },
	{ "Float@",            95,  7,  1     },
	{ "Date@",             95,  8,  1     },
	{ "Pointer@",          95, 11,  1     },
	
	{ NULL }
};

#endif
