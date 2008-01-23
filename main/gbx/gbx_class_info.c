/***************************************************************************

  class_info.c

  Information about internal component

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

#define __GBX_CLASS_INFO_C

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gambas.h"

/*#include "gbx_c_gambas.h"
#include "gbx_c_class.h"
#include "gbx_c_error.h"
#include "gbx_c_collection.h"
#include "gbx_c_list.h"
#include "gbx_c_file.h"
#include "gbx_c_application.h"
#include "gbx_c_array.h"
#include "gbx_c_process.h"
#include "gbx_c_subcollection.h"
#include "gbx_c_string.h"*/

static GB_DESC NATIVE_GambasLanguage[] =
{
  GB_DECLARE(".", 0),

  GB_METHOD("Left$", "s", NULL, "(String)s[(Length)i]"),
  GB_METHOD("Left", "s", NULL, "(String)s[(Length)i]"),

  GB_METHOD("Mid$", "s", NULL, "(String)s[(Pos)i(Length)i]"),
  GB_METHOD("Mid", "s", NULL, "(String)s[(Pos)i(Length)i]"),

  GB_METHOD("Right$", "s", NULL, "(String)s[(Length)i]"),
  GB_METHOD("Right", "s", NULL, "(String)s[(Length)i]"),

  GB_METHOD("Len", "i", NULL, "(String)s"),

  GB_METHOD("Space$", "s", NULL, "(Length)i"),
  GB_METHOD("Space", "s", NULL, "(Length)i"),

  GB_METHOD("String$", "s", NULL, "(Length)i(Pattern)s"),
  GB_METHOD("String", "s", NULL, "(Length)i(Pattern)s"),

  GB_METHOD("Trim$", "s", NULL, "(String)s"),
  GB_METHOD("Trim", "s", NULL, "(String)s"),

  GB_METHOD("LTrim$", "s", NULL, "(String)s"),
  GB_METHOD("LTrim", "s", NULL, "(String)s"),

  GB_METHOD("RTrim$", "s", NULL, "(String)s"),
  GB_METHOD("RTrim", "s", NULL, "(String)s"),

  GB_METHOD("Upper$", "s", NULL, "(String)s"),
  GB_METHOD("Upper", "s", NULL, "(String)s"),
  GB_METHOD("UCase$", "s", NULL, "(String)s"),
  GB_METHOD("UCase", "s", NULL, "(String)s"),

  GB_METHOD("Lower$", "s", NULL, "(String)s"),
  GB_METHOD("Lower", "s", NULL, "(String)s"),
  GB_METHOD("LCase$", "s", NULL, "(String)s"),
  GB_METHOD("LCase", "s", NULL, "(String)s"),

  GB_METHOD("Chr$", "s", NULL, "(Code)i"),
  GB_METHOD("Chr", "s", NULL, "(Code)i"),

  GB_METHOD("Asc", "i", NULL, "(String)s[(Pos)i]"),

  GB_METHOD("Instr", "i", NULL, "(String)s(Pattern)s[(From)i(Mode)i]"),
  GB_METHOD("RInstr", "i", NULL, "(String)s(Pattern)s[(From)i(Mode)i]"),

  GB_METHOD("Subst$", "s", NULL, "(Pattern)s."),
  GB_METHOD("Subst", "s", NULL, "(Pattern)s."),

  GB_METHOD("Replace$", "s", NULL, "(String)s(Find)s(Replace)s[(Mode)i]"),
  GB_METHOD("Replace", "s", NULL, "(String)s(Find)s(Replace)s[(Mode)i]"),

  GB_METHOD("Split", "String[]", NULL, "(String)s[(Separator)s(Escape)s(IgnoreVoid)b]"),
  GB_METHOD("Scan", "String[]", NULL, "(String)s(Pattern)s"),

  GB_METHOD("Comp", "i", NULL, "(String1)s(String2)s[(Mode)i]"),

  GB_METHOD("Conv$", "s", NULL, "(String)s(From)s(To)s"),
  GB_METHOD("Conv", "s", NULL, "(String)s(From)s(To)s"),

  GB_METHOD("SConv$", "s", NULL, "(String)s"),
  GB_METHOD("SConv", "s", NULL, "(String)s"),

  GB_METHOD("DConv$", "s", NULL, "(String)s"),
  GB_METHOD("DConv", "s", NULL, "(String)s"),

  GB_METHOD("Abs", "v", NULL, "(Value)v"),
  GB_METHOD("Int", "v", NULL, "(Value)v"),

  GB_METHOD("Frac", "f", NULL, "(Value)f"),
  GB_METHOD("Log", "f", NULL, "(Value)f"),
  GB_METHOD("Exp", "f", NULL, "(Value)f"),
  GB_METHOD("Sqr", "f", NULL, "(Value)f"),
  GB_METHOD("Sin", "f", NULL, "(Value)f"),
  GB_METHOD("Cos", "f", NULL, "(Value)f"),
  GB_METHOD("Tan", "f", NULL, "(Value)f"),
  GB_METHOD("Atn", "f", NULL, "(Value)f"),
  GB_METHOD("ATan", "f", NULL, "(Value)f"),
  GB_METHOD("Asn", "f", NULL, "(Value)f"),
  GB_METHOD("ASin", "f", NULL, "(Value)f"),
  GB_METHOD("Acs", "f", NULL, "(Value)f"),
  GB_METHOD("ACos", "f", NULL, "(Value)f"),
  GB_METHOD("Deg", "f", NULL, "(Radians)f"),
  GB_METHOD("Rad", "f", NULL, "(Degrees)f"),
  GB_METHOD("Log10", "f", NULL, "(Value)f"),
  GB_METHOD("Sinh", "f", NULL, "(Value)f"),
  GB_METHOD("Cosh", "f", NULL, "(Value)f"),
  GB_METHOD("Tanh", "f", NULL, "(Value)f"),
  GB_METHOD("Asnh", "f", NULL, "(Value)f"),
  GB_METHOD("ASinh", "f", NULL, "(Value)f"),
  GB_METHOD("Acsh", "f", NULL, "(Value)f"),
  GB_METHOD("ACosh", "f", NULL, "(Value)f"),
  GB_METHOD("Atnh", "f", NULL, "(Value)f"),
  GB_METHOD("ATanh", "f", NULL, "(Value)f"),
  GB_METHOD("Exp2", "f", NULL, "(Value)f"),
  GB_METHOD("Exp10", "f", NULL, "(Value)f"),
  GB_METHOD("Log2", "f", NULL, "(Value)f"),
  GB_METHOD("Cbr", "f", NULL, "(Value)f"),
  GB_METHOD("Expm", "f", NULL, "(Value)f"),
  GB_METHOD("Logp", "f", NULL, "(Value)f"),

  GB_METHOD("Atan2", "f", NULL, "(X)f(Y)f"),
  GB_METHOD("Ang", "f", NULL, "(X)f(Y)f"),
  GB_METHOD("Hyp", "f", NULL, "(X)f(Y)f"),
  GB_METHOD("Mag", "f", NULL, "(X)f(Y)f"),

  GB_METHOD("Sgn", "i", NULL, "(Value)v"),
  GB_METHOD("Fix", "v", NULL, "(Value)v"),

  GB_METHOD("Pi", "f", NULL, "[(Factor)f]"),

  GB_METHOD("Round", "f", NULL, "(Value)f[(Round)i]"),

  GB_METHOD("Rnd", "f", NULL, "[(From)f(To)f]"),

  GB_METHOD("Min", "v", NULL, "(Value)v(Value2)v."),
  GB_METHOD("Max", "v", NULL, "(Value)v(Value2)v."),

  GB_METHOD("If", "v", NULL, "(Test)b(True)v(False)v"),
  GB_METHOD("IIf", "v", NULL, "(Test)b(True)v(False)v"),

  GB_METHOD("Choose", "v", NULL, "(Select)i[(Value)v.]"),

  GB_METHOD("BClr", "i", NULL, "(Value)i(Bit)i"),
  GB_METHOD("BSet", "i", NULL, "(Value)i(Bit)i"),
  GB_METHOD("BTst", "i", NULL, "(Value)i(Bit)i"),
  GB_METHOD("BChg", "i", NULL, "(Value)i(Bit)i"),

  GB_METHOD("Shl", "i", NULL, "(Value)i(Shift)i"),
  GB_METHOD("Shr", "i", NULL, "(Value)i(Shift)i"),
  GB_METHOD("Rol", "i", NULL, "(Value)i(Shift)i"),
  GB_METHOD("Ror", "i", NULL, "(Value)i(Shift)i"),

  GB_METHOD("IsBoolean", "b", NULL, "(Value)v"),
  GB_METHOD("IsByte", "b", NULL, "(Value)v"),
  GB_METHOD("IsShort", "b", NULL, "(Value)v"),
  GB_METHOD("IsInteger", "b", NULL, "(Value)v"),
  GB_METHOD("IsLong", "b", NULL, "(Value)v"),
  GB_METHOD("IsFloat", "b", NULL, "(Value)v"),
  GB_METHOD("IsDate", "b", NULL, "(Value)v"),
  GB_METHOD("IsString", "b", NULL, "(Value)v"),
  GB_METHOD("IsNull", "b", NULL, "(Value)v"),
  GB_METHOD("IsObject", "b", NULL, "(Value)v"),
  GB_METHOD("IsNumber", "b", NULL, "(Value)v"),

  GB_METHOD("Boolean?", "b", NULL, "(Value)v"),
  GB_METHOD("Byte?", "b", NULL, "(Value)v"),
  GB_METHOD("Short?", "b", NULL, "(Value)v"),
  GB_METHOD("Integer?", "b", NULL, "(Value)v"),
  GB_METHOD("Long?", "b", NULL, "(Value)v"),
  GB_METHOD("Float?", "b", NULL, "(Value)v"),
  GB_METHOD("Date?", "b", NULL, "(Value)v"),
  GB_METHOD("String?", "b", NULL, "(Value)v"),
  GB_METHOD("Null?", "b", NULL, "(Value)v"),
  GB_METHOD("Object?", "b", NULL, "(Value)v"),
  GB_METHOD("Number?", "b", NULL, "(Value)v"),

  GB_METHOD("IsAscii", "b", NULL, "(String)s"),
  GB_METHOD("IsLetter", "b", NULL, "(String)s"),
  GB_METHOD("IsLower", "b", NULL, "(String)s"),
  GB_METHOD("IsUpper", "b", NULL, "(String)s"),
  GB_METHOD("IsDigit", "b", NULL, "(String)s"),
  GB_METHOD("IsHexa", "b", NULL, "(String)s"),
  GB_METHOD("IsSpace", "b", NULL, "(String)s"),
  GB_METHOD("IsBlank", "b", NULL, "(String)s"),
  GB_METHOD("IsPunct", "b", NULL, "(String)s"),

  GB_METHOD("Ascii?", "b", NULL, "(String)s"),
  GB_METHOD("Letter?", "b", NULL, "(String)s"),
  GB_METHOD("Lower?", "b", NULL, "(String)s"),
  GB_METHOD("Upper?", "b", NULL, "(String)s"),
  GB_METHOD("Digit?", "b", NULL, "(String)s"),
  GB_METHOD("Hexa?", "b", NULL, "(String)s"),
  GB_METHOD("Space?", "b", NULL, "(String)s"),
  GB_METHOD("Blank?", "b", NULL, "(String)s"),
  GB_METHOD("Punct?", "b", NULL, "(String)s"),

  GB_METHOD("TypeOf", "i", NULL, "(Value)v"),

  GB_METHOD("CBool", "b", NULL, "(Value)v"),
  GB_METHOD("CByte", "c", NULL, "(Value)v"),
  GB_METHOD("CShort", "h", NULL, "(Value)v"),
  GB_METHOD("CInt", "i", NULL, "(Value)v"),
  GB_METHOD("CInteger", "i", NULL, "(Value)v"),
  GB_METHOD("CFloat", "f", NULL, "(Value)v"),
  GB_METHOD("CDate", "d", NULL, "(Value)v"),
  GB_METHOD("CStr", "s", NULL, "(Value)v"),
  GB_METHOD("CString", "s", NULL, "(Value)v"),

  GB_METHOD("Bin$", "s", NULL, "(Value)v[(Digits)i]"),
  GB_METHOD("Bin", "s", NULL, "(Value)v[(Digits)i]"),

  GB_METHOD("Hex$", "s", NULL, "(Value)v[(Digits)i]"),
  GB_METHOD("Hex", "s", NULL, "(Value)v[(Digits)i]"),

  GB_METHOD("Val", "v", NULL, "(String)s"),

  GB_METHOD("Str$", "s", NULL, "(Value)v"),
  GB_METHOD("Str", "s", NULL, "(Value)v"),

  GB_METHOD("Format$", "s", NULL, "(Value)v[(Format)s]"),
  GB_METHOD("Format", "s", NULL, "(Value)v[(Format)s]"),

  GB_METHOD("Timer", "f", NULL, NULL),
  GB_METHOD("Now", "d", NULL, NULL),

  GB_METHOD("Year", "i", NULL, "(Date)d"),
  GB_METHOD("Month", "i", NULL, "(Date)d"),
  GB_METHOD("Day", "i", NULL, "(Date)d"),
  GB_METHOD("Hour", "i", NULL, "(Date)d"),
  GB_METHOD("Minute", "i", NULL, "(Date)d"),
  GB_METHOD("Second", "i", NULL, "(Date)d"),
  GB_METHOD("WeekDay", "i", NULL, "(Date)d"),

  GB_METHOD("Date", "d", NULL, "[(Year)i(Month)i(Day)i(Hour)i(Minute)i(Second)i]"),
  GB_METHOD("Time", "d", NULL, "[(Hour)i(Minute)i(Second)i]"),
  GB_METHOD("Week", "i", NULL, "[(Date)d(Mode)i(Plain)b]"),

  GB_METHOD("DateAdd", "d", NULL, "(Date)d(Period)i(Interval)i"),
  GB_METHOD("DateDiff", "i", NULL, "(Date1)d(Date2)d(Period)i"),

  GB_METHOD("Eval", "v", NULL, "(Expression)s[(Context)Collection;]"),

  GB_METHOD("Eof", "b", NULL, "[(File)Stream;]"),
  GB_METHOD("Lof", "l", NULL, "[(File)Stream;]"),
  GB_METHOD("Seek", "l", NULL, "(File)Stream;"),

  GB_METHOD("Exist", "b", NULL, "(Path)s"),
  GB_METHOD("Stat", "Stat", NULL, "(Path)s[(FollowLink)b]"),

  GB_METHOD("Temp$", "s", NULL, "[(Prefix)s]"),
  GB_METHOD("Temp", "s", NULL, "[(Prefix)s]"),

  GB_METHOD("IsDir", "b", NULL, "(Path)s"),
  GB_METHOD("Dir?", "b", NULL, "(Path)s"),

  GB_METHOD("Dir", "String[]", NULL, "(Path)s[(Pattern)s(Filter)i]"),

  GB_METHOD("Access", "b", NULL, "(Path)s[(Mode)i]"),

  GB_METHOD("Alloc", "p", NULL, "(SizeOrString)v[(Count)i]"),
  GB_METHOD("Free", NULL, NULL, "(Pointer)p"),
  GB_METHOD("Realloc", "i", NULL, "(Pointer)p(Size)i[(Count)i]"),
  GB_METHOD("StrPtr", "s", NULL, "(Pointer)p"),

  GB_METHOD("RDir", "String[]", NULL, "(Path)s[(Pattern)s(Filter)i]"),

  GB_METHOD("DFree", "l", NULL, "(Path)s"),

  GB_END_DECLARE
};


EXTERN GB_DESC NATIVE_GambasLanguage[];
EXTERN GB_DESC NATIVE_Gambas[];
EXTERN GB_DESC NATIVE_Param[];
EXTERN GB_DESC NATIVE_Enum[];
EXTERN GB_DESC NATIVE_Symbol[];
EXTERN GB_DESC NATIVE_ClassSymbols[];
EXTERN GB_DESC NATIVE_Class[];
EXTERN GB_DESC NATIVE_Classes[];
EXTERN GB_DESC NATIVE_Component[];
EXTERN GB_DESC NATIVE_Components[];
EXTERN GB_DESC NATIVE_Object[];
EXTERN GB_DESC NATIVE_Collection[];
//EXTERN GB_DESC NATIVE_List[];
EXTERN GB_DESC NATIVE_Error[];
EXTERN GB_DESC NATIVE_Stream[];
EXTERN GB_DESC NATIVE_FilePerm[];
EXTERN GB_DESC NATIVE_Stat[];
EXTERN GB_DESC NATIVE_File[];
EXTERN GB_DESC NATIVE_AppEnv[];
EXTERN GB_DESC NATIVE_AppArgs[];
EXTERN GB_DESC NATIVE_App[];
EXTERN GB_DESC NATIVE_System[];
EXTERN GB_DESC NATIVE_User[];
EXTERN GB_DESC NATIVE_ArrayBounds[];
EXTERN GB_DESC NATIVE_Array[];
EXTERN GB_DESC NATIVE_Process[];
EXTERN GB_DESC NATIVE_BooleanArray[];
EXTERN GB_DESC NATIVE_ByteArray[];
EXTERN GB_DESC NATIVE_ShortArray[];
EXTERN GB_DESC NATIVE_IntegerArray[];
EXTERN GB_DESC NATIVE_SingleArray[];
EXTERN GB_DESC NATIVE_FloatArray[];
EXTERN GB_DESC NATIVE_DateArray[];
EXTERN GB_DESC NATIVE_StringArray[];
EXTERN GB_DESC NATIVE_ObjectArray[];
EXTERN GB_DESC NATIVE_VariantArray[];
EXTERN GB_DESC NATIVE_LongArray[];
EXTERN GB_DESC NATIVE_PointerArray[];
EXTERN GB_DESC NATIVE_SubCollection[];
EXTERN GB_DESC NATIVE_String[];
EXTERN GB_DESC NATIVE_Timer[];
EXTERN GB_DESC NATIVE_Quote[];
EXTERN GB_DESC NATIVE_Unquote[];
EXTERN GB_DESC NATIVE_Observer[];

GB_DESC *GB_CLASSES[] EXPORT =
{
  NATIVE_GambasLanguage,
  NATIVE_Gambas,
  NATIVE_Param,
  NATIVE_Enum,
  NATIVE_Symbol,
  NATIVE_ClassSymbols,
  NATIVE_Class,
  NATIVE_Classes,
  NATIVE_Component,
  NATIVE_Components,
  NATIVE_Object,
  NATIVE_Collection,
  NATIVE_Error,
  NATIVE_Stream,
  NATIVE_FilePerm,
  NATIVE_Stat,
  NATIVE_File,
  NATIVE_AppEnv,
  NATIVE_AppArgs,
  NATIVE_App,
  NATIVE_System,
  NATIVE_User,
  NATIVE_ArrayBounds,
  NATIVE_Array,
  NATIVE_Process,
  NATIVE_BooleanArray,
  NATIVE_ByteArray,
  NATIVE_ShortArray,
  NATIVE_IntegerArray,
  NATIVE_SingleArray,
  NATIVE_FloatArray,
  NATIVE_DateArray,
  NATIVE_StringArray,
  NATIVE_ObjectArray,
  NATIVE_VariantArray,
  NATIVE_LongArray,
  NATIVE_PointerArray,
  NATIVE_SubCollection,
  NATIVE_String,
  NATIVE_Timer,
  NATIVE_Quote,
  NATIVE_Unquote,
  NATIVE_Observer,
  NULL
};

