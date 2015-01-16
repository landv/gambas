/***************************************************************************

  gbx_class_info.c

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

#define __GBX_CLASS_INFO_C

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gbx_c_array.h"
#include "gambas.h"

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

	GB_METHOD("Split", "String[]", NULL, "(String)s[(Separators)s(Escape)s(IgnoreVoid)b(KeepEscape)b]"),
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
	GB_METHOD("Ceil", "f", NULL, "(Value)f"),
	GB_METHOD("Floor", "f", NULL, "(Value)f"),

	GB_METHOD("Atan2", "f", NULL, "(X)f(Y)f"),
	GB_METHOD("Ang", "f", NULL, "(X)f(Y)f"),
	GB_METHOD("Hyp", "f", NULL, "(X)f(Y)f"),
	GB_METHOD("Mag", "f", NULL, "(X)f(Y)f"),

	GB_METHOD("Sgn", "i", NULL, "(Value)v"),
	GB_METHOD("Fix", "v", NULL, "(Value)v"),

	GB_METHOD("Pi", "f", NULL, "[(Factor)f]"),

	GB_METHOD("Round", "f", NULL, "(Value)f[(Round)i]"),

	GB_METHOD("Rnd", "f", NULL, "[(From)f(To)f]"),

	GB_METHOD("Min", "v", NULL, "(Value)v(Value2)v"),
	GB_METHOD("Max", "v", NULL, "(Value)v(Value2)v"),

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

	GB_METHOD("IsBoolean", "b", NULL, "(Value)s"),
	GB_METHOD("IsInteger", "b", NULL, "(Value)s"),
	GB_METHOD("IsLong", "b", NULL, "(Value)s"),
	GB_METHOD("IsFloat", "b", NULL, "(Value)s"),
	GB_METHOD("IsDate", "b", NULL, "(Value)s"),
	GB_METHOD("IsNumber", "b", NULL, "(Value)s"),
	GB_METHOD("IsNull", "b", NULL, "(Value)v"),

	GB_METHOD("IsAscii", "b", NULL, "(String)s"),
	GB_METHOD("IsLetter", "b", NULL, "(String)s"),
	GB_METHOD("IsLower", "b", NULL, "(String)s"),
	GB_METHOD("IsUpper", "b", NULL, "(String)s"),
	GB_METHOD("IsLCase", "b", NULL, "(String)s"),
	GB_METHOD("IsUCase", "b", NULL, "(String)s"),
	GB_METHOD("IsDigit", "b", NULL, "(String)s"),
	GB_METHOD("IsHexa", "b", NULL, "(String)s"),
	GB_METHOD("IsSpace", "b", NULL, "(String)s"),
	GB_METHOD("IsBlank", "b", NULL, "(String)s"),
	GB_METHOD("IsPunct", "b", NULL, "(String)s"),

	GB_METHOD("TypeOf", "i", NULL, "(Value)v"),
	GB_METHOD("SizeOf", "i", NULL, "(Type)i"),

	GB_METHOD("CBool", "b", NULL, "(Value)v"),
	GB_METHOD("CBoolean", "b", NULL, "(Value)v"),
	GB_METHOD("CByte", "c", NULL, "(Value)v"),
	GB_METHOD("CShort", "h", NULL, "(Value)v"),
	GB_METHOD("CInt", "i", NULL, "(Value)v"),
	GB_METHOD("CInteger", "i", NULL, "(Value)v"),
	GB_METHOD("CLong", "l", NULL, "(Value)v"),
	GB_METHOD("CSingle", "g", NULL, "(Value)v"),
	GB_METHOD("CFloat", "f", NULL, "(Value)v"),
	GB_METHOD("CDate", "d", NULL, "(Value)v"),
	GB_METHOD("CStr", "s", NULL, "(Value)v"),
	GB_METHOD("CString", "s", NULL, "(Value)v"),
	GB_METHOD("CPointer", "p", NULL, "(Value)v"),
	GB_METHOD("CVariant", "v", NULL, "(Value)v"),

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

	GB_METHOD("Date", "d", NULL, "[(DateOrYear)v(Month)i(Day)i(Hour)i(Minute)i(Second)i(MilliSecond)i]"),
	GB_METHOD("Time", "d", NULL, "[(DateOrHour)v(Minute)i(Second)i(MilliSecond)i]"),
	GB_METHOD("Week", "i", NULL, "[(Date)d(Mode)i(Plain)b]"),

	GB_METHOD("DateAdd", "d", NULL, "(Date)d(Period)i(Count)i"),
	GB_METHOD("DateDiff", "i", NULL, "(Date1)d(Date2)d(Period)i"),

	GB_METHOD("Eval", "v", NULL, "(Expression)s[(Context)Collection;]"),

	GB_METHOD("Eof", "b", NULL, "[(File)Stream;]"),
	GB_METHOD("Lof", "l", NULL, "[(File)Stream;]"),
	GB_METHOD("Seek", "l", NULL, "(File)Stream;"),

	GB_METHOD("Exist", "b", NULL, "(Path)s[(FollowLink)b]"),
	GB_METHOD("Stat", "Stat", NULL, "(Path)s[(FollowLink)b]"),

	GB_METHOD("Temp$", "s", NULL, "[(Prefix)s]"),
	GB_METHOD("Temp", "s", NULL, "[(Prefix)s]"),

	GB_METHOD("IsDir", "b", NULL, "(Path)s"),

	GB_METHOD("Access", "b", NULL, "(Path)s[(Mode)i]"),

	GB_METHOD("Dir", "String[]", NULL, "(Path)s[(Pattern)s(Filter)i]"),
	GB_METHOD("RDir", "String[]", NULL, "(Path)s[(Pattern)s(Filter)i(FollowLink)b]"),
	
	GB_METHOD("DFree", "l", NULL, "(Path)s"),

	GB_METHOD("Alloc", "p", NULL, "(SizeOrString)v[(Count)i]"),
	GB_METHOD("Free", NULL, NULL, "(Pointer)p"),
	GB_METHOD("Realloc", "i", NULL, "(Pointer)p(Size)i[(Count)i]"),
	GB_METHOD("Str@", "s", NULL, "(Pointer)p"),
	GB_METHOD("String@", "s", NULL, "(Pointer)p"),
	GB_METHOD("VarPtr", "p", NULL, "(Variable)v"),
	
	GB_METHOD("MkBool", "s", NULL, "(Value)b"),
	GB_METHOD("MkBool$", "s", NULL, "(Value)b"),
	GB_METHOD("MkBoolean", "s", NULL, "(Value)b"),
	GB_METHOD("MkBoolean$", "s", NULL, "(Value)b"),
	GB_METHOD("MkByte", "s", NULL, "(Value)c"),
	GB_METHOD("MkByte$", "s", NULL, "(Value)c"),
	GB_METHOD("MkShort", "s", NULL, "(Value)h"),
	GB_METHOD("MkShort$", "s", NULL, "(Value)h"),
	GB_METHOD("MkInt", "s", NULL, "(Value)i"),
	GB_METHOD("MkInt$", "s", NULL, "(Value)i"),
	GB_METHOD("MkInteger", "s", NULL, "(Value)i"),
	GB_METHOD("MkInteger$", "s", NULL, "(Value)i"),
	GB_METHOD("MkLong", "s", NULL, "(Value)l"),
	GB_METHOD("MkLong$", "s", NULL, "(Value)l"),
	GB_METHOD("MkSingle", "s", NULL, "(Value)g"),
	GB_METHOD("MkSingle$", "s", NULL, "(Value)g"),
	GB_METHOD("MkFloat", "s", NULL, "(Value)f"),
	GB_METHOD("MkFloat$", "s", NULL, "(Value)f"),
	GB_METHOD("MkDate", "s", NULL, "(Value)d"),
	GB_METHOD("MkDate$", "s", NULL, "(Value)d"),
	GB_METHOD("MkPointer", "s", NULL, "(Value)p"),
	GB_METHOD("MkPointer$", "s", NULL, "(Value)p"),

	GB_METHOD("Swap", "s", NULL, "(String)s[(Endianness)i]"),
	GB_METHOD("Swap$", "s", NULL, "(String)s[(Endianness)i]"),

	GB_METHOD("Bool@", "b", NULL, "(Pointer)p"),
	GB_METHOD("Boolean@", "b", NULL, "(Pointer)p"),
	GB_METHOD("Byte@", "c", NULL, "(Pointer)p"),
	GB_METHOD("Short@", "h", NULL, "(Pointer)p"),
	GB_METHOD("Int@", "i", NULL, "(Pointer)p"),
	GB_METHOD("Integer@", "i", NULL, "(Pointer)p"),
	GB_METHOD("Long@", "l", NULL, "(Pointer)p"),
	GB_METHOD("Single@", "g", NULL, "(Pointer)p"),
	GB_METHOD("Float@", "f", NULL, "(Pointer)p"),
	GB_METHOD("Date@", "f", NULL, "(Pointer)p"),
	GB_METHOD("Pointer@", "p", NULL, "(Pointer)p"),

	GB_METHOD("Tr", "s", NULL, "(String)s"),
	GB_METHOD("Tr$", "s", NULL, "(String)s"),

	GB_METHOD("Quote", "s", NULL, "(String)s"),
	GB_METHOD("Quote$", "s", NULL, "(String)s"),
	
	GB_METHOD("Shell", "s", NULL, "(String)s"),
	GB_METHOD("Shell$", "s", NULL, "(String)s"),
	
	GB_METHOD("Html", "s", NULL, "(String)s"),
	GB_METHOD("Html$", "s", NULL, "(String)s"),

	GB_METHOD("Base64", "s", NULL, "(String)s"),
	GB_METHOD("Base64$", "s", NULL, "(String)s"),

	GB_METHOD("Url", "s", NULL, "(String)s"),
	GB_METHOD("Url$", "s", NULL, "(String)s"),

	GB_METHOD("Unquote", "s", NULL, "(String)s"),
	GB_METHOD("Unquote$", "s", NULL, "(String)s"),
	
	GB_METHOD("UnBase64", "s", NULL, "(String)s"),
	GB_METHOD("UnBase64$", "s", NULL, "(String)s"),

	GB_METHOD("FromBase64", "s", NULL, "(String)s"),
	GB_METHOD("FromBase64$", "s", NULL, "(String)s"),

	GB_METHOD("FromUrl", "s", NULL, "(String)s"),
	GB_METHOD("FromUrl$", "s", NULL, "(String)s"),

	GB_METHOD("Odd", "b", NULL, "(Value)i"),
	GB_METHOD("Even", "b", NULL, "(Value)i"),

	GB_METHOD("IsNan", "b", NULL, "(Value)f"),
	GB_METHOD("IsInf", "i", NULL, "(Value)f"),

	GB_METHOD("IsMissing", "b", NULL, "(Argument)?"),

	GB_END_DECLARE
};


extern GB_DESC NATIVE_GambasLanguage[];
extern GB_DESC NATIVE_Gambas[];
extern GB_DESC NATIVE_Param[];
extern GB_DESC NATIVE_Enum[];
extern GB_DESC NATIVE_Symbol[];
extern GB_DESC NATIVE_Class[];
extern GB_DESC NATIVE_Classes[];
extern GB_DESC NATIVE_Component[];
extern GB_DESC NATIVE_Components[];
extern GB_DESC NATIVE_Object[];
extern GB_DESC NATIVE_Collection[];
extern GB_DESC NATIVE_Error[];
extern GB_DESC NATIVE_Stream[];
extern GB_DESC NATIVE_StreamLines[];
extern GB_DESC NATIVE_StatPerm[];
extern GB_DESC NATIVE_Stat[];
extern GB_DESC NATIVE_File[];
extern GB_DESC NATIVE_AppEnv[];
extern GB_DESC NATIVE_AppArgs[];
extern GB_DESC NATIVE_App[];
extern GB_DESC NATIVE_System[];
extern GB_DESC NATIVE_User[];
extern GB_DESC NATIVE_ArrayBounds[];
extern GB_DESC NATIVE_Array[];
extern GB_DESC NATIVE_Process[];
extern GB_DESC NATIVE_BooleanArray[];
extern GB_DESC NATIVE_ByteArray[];
extern GB_DESC NATIVE_ShortArray[];
extern GB_DESC NATIVE_IntegerArray[];
extern GB_DESC NATIVE_SingleArray[];
extern GB_DESC NATIVE_FloatArray[];
extern GB_DESC NATIVE_DateArray[];
extern GB_DESC NATIVE_StringArray[];
extern GB_DESC NATIVE_ObjectArray[];
extern GB_DESC NATIVE_VariantArray[];
extern GB_DESC NATIVE_TemplateArray[];
extern GB_DESC NATIVE_TemplateArrayOfStruct[];
extern GB_DESC NATIVE_LongArray[];
extern GB_DESC NATIVE_PointerArray[];
extern GB_DESC NATIVE_String[];
extern GB_DESC TaskDesc[];
extern GB_DESC NATIVE_Timer[];
extern GB_DESC NATIVE_Observer[];
extern GB_DESC NATIVE_Proxy[];

GB_DESC *GB_CLASSES[] EXPORT =
{
	NATIVE_GambasLanguage,
	NATIVE_Gambas,
	NATIVE_Param,
	NATIVE_Enum,
	NATIVE_Symbol,
	NATIVE_Class,
	NATIVE_Classes,
	NATIVE_Component,
	NATIVE_Components,
	NATIVE_Object,
	NATIVE_Collection,
	NATIVE_Error,
	NATIVE_StreamLines,
	NATIVE_Stream,
	NATIVE_StatPerm,
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
	NATIVE_TemplateArray,
	NATIVE_TemplateArrayOfStruct,
	NATIVE_LongArray,
	NATIVE_PointerArray,
	NATIVE_String,
	TaskDesc,
	NATIVE_Timer,
	NATIVE_Observer,
	//NATIVE_Proxy,
	NULL
};

