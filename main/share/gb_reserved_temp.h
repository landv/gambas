/***************************************************************************

  gb_reserved_temp.h

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

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_pcode.h"
#include "gb_type_common.h"
#include "gb_reserved.h"

/* If this file is modified, don't forget to update GAMBAS_PCODE_VERSION in acinclude.m4 if needed */

#include "gb_reserved_keyword.h"

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
  { "IsPointer",         37,  11, 1     },
  { "IsVariant",         37,  12, 1     },
  { "IsNull",            37,  15, 1     },
  { "IsObject",          37,  16, 1     },
  { "IsNumber",          37,  17, 1     },

  /*{ "Boolean?",          37,  1,  1     },
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
  { "Number?",           37,  17, 1     },*/

  { "TypeOf",            38,  0,  1     },

  { "CBool",             39,  1,  1     },   /* CODE_CONV */
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
	{ "CVar",              39, 12,  1     },
	{ "CVariant",          39, 12,  1     },

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
  { "RDir",              82,  0,  1,  4 },

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

  { "VarPtr",            89,  0,  1     },

  { ".Collection",       90,  0,  1, 63 },
  
  { "Tr",                91,  0,  1     },
  { "Tr$",               91,  0,  1     },
  
  { "Quote",             92,  0,  1     },
  { "Quote$",            92,  0,  1     },
  { "Shell",             92,  1,  1     },
  { "Shell$",            92,  1,  1     },
  { "Html",              92,  2,  1     },
  { "Html$",             92,  2,  1     },
  
  { "Unquote",           93,  0,  1     },
  { "Unquote$",          93,  0,  1     },

  { "Assign",            94,  0,  1,  2 },   /* CODE_ASSIGN */
  /*
  { "_EventOff",         94,  0,  0     },
  { "_EventOn",          95,  0,  0     },
  */

  { NULL }
};

TABLE *COMP_res_table;
TABLE *COMP_subr_table;

int SUBR_VarPtr;
int SUBR_Mid;
int SUBR_MidS;

static uchar _operator_table[256] = { 0 };

static int get_index(const char *subr_name)
{
  int index;

  if (TABLE_find_symbol(COMP_subr_table, subr_name, strlen(subr_name), NULL, &index))
    return index;
  else
    return NO_SYMBOL;
}

void RESERVED_init(void)
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
  printf("Reserved symbols table:\n");
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
  
  /* Subroutines table */

  TABLE_create(&COMP_subr_table, 0, TF_IGNORE_CASE);
  for (subr = &COMP_subr_info[0]; subr->name; subr++)
  {
    if (subr->max_param == 0)
      subr->max_param = subr->min_param;

    TABLE_add_symbol(COMP_subr_table, subr->name, strlen(subr->name), NULL, NULL);
  }

  #ifdef DEBUG
  printf("Subroutines table:\n");
  TABLE_print(COMP_subr_table, TRUE);
  #endif

	SUBR_VarPtr = get_index("VarPtr");
	SUBR_Mid = get_index("Mid");
	SUBR_MidS = get_index("Mid$");

  /* Table des constantes */

  /*
  TABLE_create(&COMP_const_table, 0, TF_IGNORE_CASE);
  for (cst = &COMP_const_info[0]; cst->name; cst++)
    TABLE_add_symbol(COMP_const_table, cst->name, strlen(cst->name), NULL, NULL);
  */
}


void RESERVED_exit(void)
{
  TABLE_delete(&COMP_res_table);
  TABLE_delete(&COMP_subr_table);
}


SUBR_INFO *SUBR_get(const char *subr_name)
{
  int index = get_index(subr_name);

	if (index == NO_SYMBOL)
    return NULL;
  else
    return &COMP_subr_info[index];
}


SUBR_INFO *SUBR_get_from_opcode(ushort opcode, ushort optype)
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

static uint hash(const char *key, int len)
{
	int i;
	uint h = 1;
	for (i = 0; i < len; i++)
		h = (h << 4) + (h ^ (key[i] & 0x1F));
	
	return h % 73;
}
	
int RESERVED_find_word(const char *word, int len)
{
  int ind;
  
	if (len == 1)
	{
		ind = _operator_table[(uint)*word];
		if (ind)
			return ind;
		else
			return -1;
	}
	
  /*if (TABLE_find_symbol(COMP_res_table, word, len, NULL, &ind))
    return ind;
	else
		return -1;*/

  static void *jump[] = {
    &&__00,     &&__01,     &&__02,     &&__03,     &&__04,     &&__05,     &&__06,     &&__07, 
    &&__08,     &&__09,     &&__10,     &&__11,     &&__12,     &&__13,     &&__14,     &&__15, 
    &&__16,     &&__17,     &&__18,     &&__19,     &&__20,     &&__21,     &&__22,     &&__23, 
    &&__24,     &&__25,     &&__26,     &&__27,     &&__28,     &&__29,     &&__30,     &&__31, 
    &&__32,     &&__33,     &&__34,     &&__35,     &&__36,     &&__37,     &&__38,     &&__39, 
    &&__40,     &&__41,     &&__42,     &&__43,     &&__44,     &&__45,     &&__46,     &&__47, 
    &&__48,     &&__49,     &&__50,     &&__51,     &&__52,     &&__53,     &&__54,     &&__55, 
    &&__56,     &&__57,     &&__58,     &&__59,     &&__60,     &&__61,     &&__62,     &&__63, 
    &&__64,     &&__65,     &&__66,     &&__67,     &&__68,     &&__69,     &&__70,     &&__71, 
    &&__72, 
  };

  goto *jump[hash(word, len)];

__00:
  if (len == 4 && tolower(word[0]) == 'p' && tolower(word[1]) == 'i' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' ) return 106;
  if (len == 2 && word[0] == '*' && word[1] == '=' ) return 152;
  return -1;
__01:
  if (len == 3 && tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'c' ) return 97;
  return -1;
__02:
  if (len == 4 && tolower(word[0]) == 'c' && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 'e' ) return 51;
  return -1;
__03:
  if (len == 4 && tolower(word[0]) == 't' && tolower(word[1]) == 'r' && tolower(word[2]) == 'u' && tolower(word[3]) == 'e' ) return 62;
  if (len == 4 && tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'u' && tolower(word[3]) == 'm' ) return 75;
  return -1;
__04:
  if (len == 5 && tolower(word[0]) == 'e' && tolower(word[1]) == 'r' && tolower(word[2]) == 'r' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' ) return 73;
  return -1;
__05:
  return -1;
__06:
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'c' && tolower(word[3]) == 'k' ) return 102;
  return -1;
__07:
  return -1;
__08:
  return -1;
__09:
  return -1;
__10:
  if (len == 5 && tolower(word[0]) == 'e' && tolower(word[1]) == 'v' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' ) return 21;
  return -1;
__11:
  if (len == 5 && tolower(word[0]) == 'b' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'k' ) return 53;
  if (len == 2 && word[0] == '-' && word[1] == '=' ) return 151;
  return -1;
__12:
  if (len == 5 && tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'r' && tolower(word[4]) == 't' ) return 8;
  return -1;
__13:
  if (len == 2 && word[0] == '&' && word[1] == '=' ) return 155;
  return -1;
__14:
  if (len == 5 && tolower(word[0]) == 'c' && tolower(word[1]) == 'l' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 's' ) return 13;
  if (len == 9 && tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e' && tolower(word[5]) == 'd' && tolower(word[6]) == 'u' && tolower(word[7]) == 'r' && tolower(word[8]) == 'e' ) return 29;
  return -1;
__15:
  if (len == 5 && tolower(word[0]) == 's' && tolower(word[1]) == 'u' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' ) return 74;
  return -1;
__16:
  if (len == 8 && tolower(word[0]) == 'o' && tolower(word[1]) == 'p' && tolower(word[2]) == 't' && tolower(word[3]) == 'i' && tolower(word[4]) == 'o' && tolower(word[5]) == 'n' && tolower(word[6]) == 'a' && tolower(word[7]) == 'l' ) return 32;
  return -1;
__17:
  if (len == 4 && tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' ) return 3;
  if (len == 7 && tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'g' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r' ) return 6;
  return -1;
__18:
  if (len == 5 && tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 's' && tolower(word[4]) == 't' ) return 16;
  if (len == 4 && tolower(word[0]) == 'n' && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && tolower(word[3]) == 't' ) return 49;
  return -1;
__19:
  if (len == 4 && tolower(word[0]) == 'b' && tolower(word[1]) == 'y' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' ) return 2;
  return -1;
__20:
  if (len == 6 && tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'u' && tolower(word[4]) == 'c' && tolower(word[5]) == 't' ) return 15;
  if (len == 6 && tolower(word[0]) == 'p' && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 'l' && tolower(word[4]) == 'i' && tolower(word[5]) == 'c' ) return 18;
  return -1;
__21:
  if (len == 5 && tolower(word[0]) == 'f' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' ) return 5;
  if (len == 5 && tolower(word[0]) == 'r' && tolower(word[1]) == 'a' && tolower(word[2]) == 'i' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' ) return 72;
  return -1;
__22:
  if (len == 7 && tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 'a' && tolower(word[4]) == 'u' && tolower(word[5]) == 'l' && tolower(word[6]) == 't' ) return 69;
  return -1;
__23:
  if (len == 2 && word[0] == '=' && word[1] == '=' ) return 120;
  return -1;
__24:
  if (len == 4 && tolower(word[0]) == 'n' && tolower(word[1]) == 'u' && tolower(word[2]) == 'l' && tolower(word[3]) == 'l' ) return 65;
  if (len == 4 && tolower(word[0]) == 'e' && tolower(word[1]) == 'a' && tolower(word[2]) == 'c' && tolower(word[3]) == 'h' ) return 67;
  return -1;
__25:
  if (len == 6 && tolower(word[0]) == 'b' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'r' && tolower(word[5]) == 'y' ) return 86;
  return -1;
__26:
  if (len == 5 && tolower(word[0]) == 'w' && tolower(word[1]) == 'h' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' ) return 36;
  return -1;
__27:
  if (len == 8 && tolower(word[0]) == 'f' && tolower(word[1]) == 'u' && tolower(word[2]) == 'n' && tolower(word[3]) == 'c' && tolower(word[4]) == 't' && tolower(word[5]) == 'i' && tolower(word[6]) == 'o' && tolower(word[7]) == 'n' ) return 14;
  if (len == 3 && tolower(word[0]) == 'x' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r' ) return 141;
  if (len == 2 && word[0] == '&' && word[1] == '/' ) return 149;
  return -1;
__28:
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'k' ) return 101;
  return -1;
__29:
  if (len == 5 && tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'f' ) return 43;
  return -1;
__30:
  if (len == 5 && tolower(word[0]) == 's' && tolower(word[1]) == 'l' && tolower(word[2]) == 'e' && tolower(word[3]) == 'e' && tolower(word[4]) == 'p' ) return 92;
  if (len == 2 && tolower(word[0]) == 'i' && tolower(word[1]) == 's' ) return 145;
  return -1;
__31:
  if (len == 5 && tolower(word[0]) == 'c' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h' ) return 60;
  return -1;
__32:
  if (len == 6 && tolower(word[0]) == 's' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' ) return 4;
  return -1;
__33:
  if (len == 4 && tolower(word[0]) == 's' && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'p' ) return 64;
  if (len == 7 && tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'b' && tolower(word[3]) == 'r' && tolower(word[4]) == 'a' && tolower(word[5]) == 'r' && tolower(word[6]) == 'y' ) return 104;
  return -1;
__34:
  if (len == 6 && tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 't' && tolower(word[4]) == 'i' && tolower(word[5]) == 'c' ) return 19;
  if (len == 3 && tolower(word[0]) == 's' && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' ) return 30;
  if (len == 6 && tolower(word[0]) == 'c' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' ) return 85;
  if (len == 3 && word[0] == '&' && word[1] == '/' && word[2] == '=' ) return 156;
  return -1;
__35:
  if (len == 2 && tolower(word[0]) == 'm' && tolower(word[1]) == 'e' ) return 56;
  if (len == 4 && tolower(word[0]) == 'm' && tolower(word[1]) == 'o' && tolower(word[2]) == 'v' && tolower(word[3]) == 'e' ) return 94;
  if (len == 4 && tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' && tolower(word[3]) == 's' ) return 148;
  return -1;
__36:
  if (len == 5 && tolower(word[0]) == 'm' && tolower(word[1]) == 'k' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r' ) return 98;
  return -1;
__37:
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' ) return 7;
  if (len == 4 && tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'e' && tolower(word[3]) == 'p' ) return 48;
  if (len == 5 && tolower(word[0]) == 'b' && tolower(word[1]) == 'y' && tolower(word[2]) == 'r' && tolower(word[3]) == 'e' && tolower(word[4]) == 'f' ) return 108;
  if (len == 3 && tolower(word[0]) == 'n' && tolower(word[1]) == 'o' && tolower(word[2]) == 't' ) return 140;
  return -1;
__38:
  if (len == 7 && tolower(word[0]) == 'v' && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'a' && tolower(word[5]) == 'n' && tolower(word[6]) == 't' ) return 10;
  if (len == 6 && tolower(word[0]) == 'o' && tolower(word[1]) == 'b' && tolower(word[2]) == 'j' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 't' ) return 11;
  if (len == 2 && word[0] == '<' && word[1] == '=' ) return 134;
  if (len == 2 && word[0] == '\\' && word[1] == '=' ) return 154;
  return -1;
__39:
  if (len == 8 && tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' && tolower(word[4]) == 'e' && tolower(word[5]) == 'r' && tolower(word[6]) == 't' && tolower(word[7]) == 'y' ) return 20;
  if (len == 3 && tolower(word[0]) == 'f' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r' ) return 45;
  if (len == 5 && tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' ) return 77;
  if (len == 3 && tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 'c' ) return 96;
  return -1;
__40:
  if (len == 2 && tolower(word[0]) == 'a' && tolower(word[1]) == 's' ) return 25;
  if (len == 3 && tolower(word[0]) == 'a' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' ) return 138;
  return -1;
__41:
  if (len == 4 && tolower(word[0]) == 'g' && tolower(word[1]) == 'o' && tolower(word[2]) == 't' && tolower(word[3]) == 'o' ) return 55;
  if (len == 2 && tolower(word[0]) == 'i' && tolower(word[1]) == 'n' ) return 68;
  if (len == 2 && word[0] == '<' && word[1] == '>' ) return 135;
  return -1;
__42:
  if (len == 4 && tolower(word[0]) == 'q' && tolower(word[1]) == 'u' && tolower(word[2]) == 'i' && tolower(word[3]) == 't' ) return 71;
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'e' ) return 87;
  return -1;
__43:
  return -1;
__44:
  if (len == 7 && tolower(word[0]) == 'p' && tolower(word[1]) == 'o' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r' ) return 12;
  if (len == 4 && tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'd' ) return 79;
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'k' && tolower(word[3]) == 'e' ) return 146;
  return -1;
__45:
  if (len == 6 && tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' ) return 9;
  if (len == 3 && tolower(word[0]) == 'd' && tolower(word[1]) == 'i' && tolower(word[2]) == 'm' ) return 27;
  if (len == 2 && word[0] == '/' && word[1] == '=' ) return 153;
  return -1;
__46:
  if (len == 6 && tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'a' && tolower(word[5]) == 't' ) return 38;
  if (len == 4 && tolower(word[0]) == 't' && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n' ) return 41;
  if (len == 3 && tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' ) return 44;
  if (len == 6 && tolower(word[0]) == 'a' && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n' && tolower(word[5]) == 'd' ) return 84;
  return -1;
__47:
  if (len == 5 && tolower(word[0]) == 'f' && tolower(word[1]) == 'l' && tolower(word[2]) == 'u' && tolower(word[3]) == 's' && tolower(word[4]) == 'h' ) return 88;
  return -1;
__48:
  if (len == 3 && word[0] == '.' && word[1] == '.' && word[2] == '.' ) return 113;
  return -1;
__49:
  if (len == 2 && tolower(word[0]) == 'i' && tolower(word[1]) == 'f' ) return 40;
  if (len == 4 && tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'e' && tolower(word[3]) == 'c' ) return 89;
  return -1;
__50:
  if (len == 2 && tolower(word[0]) == 't' && tolower(word[1]) == 'o' ) return 46;
  return -1;
__51:
  if (len == 7 && tolower(word[0]) == 'b' && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n' ) return 1;
  if (len == 6 && tolower(word[0]) == 'b' && tolower(word[1]) == 'e' && tolower(word[2]) == 'g' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 's' ) return 147;
  return -1;
__52:
  return -1;
__53:
  if (len == 6 && tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 't' && tolower(word[3]) == 'u' && tolower(word[4]) == 'r' && tolower(word[5]) == 'n' ) return 31;
  if (len == 9 && tolower(word[0]) == 'r' && tolower(word[1]) == 'a' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd' && tolower(word[4]) == 'o' && tolower(word[5]) == 'm' && tolower(word[6]) == 'i' && tolower(word[7]) == 'z' && tolower(word[8]) == 'e' ) return 107;
  return -1;
__54:
  if (len == 6 && tolower(word[0]) == 'o' && tolower(word[1]) == 'u' && tolower(word[2]) == 't' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't' ) return 33;
  if (len == 2 && tolower(word[0]) == 'o' && tolower(word[1]) == 'r' ) return 139;
  return -1;
__55:
  if (len == 4 && tolower(word[0]) == 'k' && tolower(word[1]) == 'i' && tolower(word[2]) == 'l' && tolower(word[3]) == 'l' ) return 93;
  return -1;
__56:
  if (len == 6 && tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 't' ) return 24;
  if (len == 7 && tolower(word[0]) == 'f' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l' && tolower(word[6]) == 'y' ) return 59;
  if (len == 4 && tolower(word[0]) == 'w' && tolower(word[1]) == 'i' && tolower(word[2]) == 't' && tolower(word[3]) == 'h' ) return 61;
  if (len == 5 && tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' ) return 90;
  if (len == 4 && tolower(word[0]) == 'w' && tolower(word[1]) == 'a' && tolower(word[2]) == 'i' && tolower(word[3]) == 't' ) return 91;
  return -1;
__57:
  if (len == 8 && tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 'i' && tolower(word[6]) == 't' && tolower(word[7]) == 's' ) return 22;
  return -1;
__58:
  if (len == 4 && tolower(word[0]) == 'f' && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm' ) return 47;
  if (len == 5 && tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'b' && tolower(word[3]) == 'u' && tolower(word[4]) == 'g' ) return 105;
  if (len == 2 && word[0] == '+' && word[1] == '=' ) return 150;
  return -1;
__59:
  if (len == 3 && tolower(word[0]) == 'm' && tolower(word[1]) == 'o' && tolower(word[2]) == 'd' ) return 144;
  return -1;
__60:
  if (len == 3 && tolower(word[0]) == 'n' && tolower(word[1]) == 'e' && tolower(word[2]) == 'w' ) return 28;
  return -1;
__61:
  return -1;
__62:
  if (len == 5 && tolower(word[0]) == 'u' && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'i' && tolower(word[4]) == 'l' ) return 37;
  if (len == 4 && tolower(word[0]) == 'w' && tolower(word[1]) == 'e' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd' ) return 39;
  return -1;
__63:
  return -1;
__64:
  if (len == 4 && tolower(word[0]) == 'e' && tolower(word[1]) == 'l' && tolower(word[2]) == 's' && tolower(word[3]) == 'e' ) return 42;
  if (len == 8 && tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 'u' && tolower(word[7]) == 'e' ) return 54;
  if (len == 5 && tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 'p' && tolower(word[3]) == 'u' && tolower(word[4]) == 't' ) return 78;
  if (len == 6 && tolower(word[0]) == 'm' && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 'y' ) return 109;
  return -1;
__65:
  if (len == 6 && tolower(word[0]) == 's' && tolower(word[1]) == 'e' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 't' ) return 50;
  if (len == 4 && tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'i' && tolower(word[3]) == 't' ) return 52;
  if (len == 6 && tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 'n' ) return 66;
  return -1;
__66:
  if (len == 2 && tolower(word[0]) == 'o' && tolower(word[1]) == 'f' ) return 26;
  if (len == 5 && tolower(word[0]) == 'f' && tolower(word[1]) == 'a' && tolower(word[2]) == 'l' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' ) return 63;
  if (len == 4 && tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' ) return 70;
  if (len == 4 && tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'y' ) return 95;
  if (len == 5 && tolower(word[0]) == 'w' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h' ) return 100;
  return -1;
__67:
  if (len == 3 && tolower(word[0]) == 'l' && tolower(word[1]) == 'e' && tolower(word[2]) == 't' ) return 76;
  if (len == 5 && tolower(word[0]) == 'w' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 't' && tolower(word[4]) == 'e' ) return 80;
  if (len == 4 && tolower(word[0]) == 'o' && tolower(word[1]) == 'p' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n' ) return 81;
  if (len == 5 && tolower(word[0]) == 'r' && tolower(word[1]) == 'm' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r' ) return 99;
  return -1;
__68:
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 't' ) return 57;
  return -1;
__69:
  if (len == 5 && tolower(word[0]) == 'c' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' ) return 82;
  return -1;
__70:
  if (len == 7 && tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'v' && tolower(word[4]) == 'a' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' ) return 17;
  if (len == 10 && tolower(word[0]) == 'i' && tolower(word[1]) == 'm' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'm' && tolower(word[6]) == 'e' && tolower(word[7]) == 'n' && tolower(word[8]) == 't' && tolower(word[9]) == 's' ) return 23;
  if (len == 2 && tolower(word[0]) == 'd' && tolower(word[1]) == 'o' ) return 34;
  if (len == 3 && tolower(word[0]) == 'd' && tolower(word[1]) == 'i' && tolower(word[2]) == 'v' ) return 143;
  return -1;
__71:
  if (len == 4 && tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' ) return 35;
  if (len == 4 && tolower(word[0]) == 's' && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k' ) return 83;
  if (len == 6 && tolower(word[0]) == 'u' && tolower(word[1]) == 'n' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'c' && tolower(word[5]) == 'k' ) return 103;
  return -1;
__72:
  if (len == 3 && tolower(word[0]) == 't' && tolower(word[1]) == 'r' && tolower(word[2]) == 'y' ) return 58;
  if (len == 2 && word[0] == '>' && word[1] == '=' ) return 133;
  if (len == 2 && word[0] == '^' && word[1] == '=' ) return 157;
  return -1;
}
