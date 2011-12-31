/***************************************************************************

  gbx_c_gambas.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_C_GAMBAS_C

#include "gbx_info.h"
#include "gbx_local.h"
#include "gbx_compare.h"
#include "gb_type_common.h"
#include "gb_file.h"
#include "gbx_date.h"

#ifndef GBX_INFO

#include "gb_error.h"
#include "gbx_api.h"
#include "gbx_class.h"
#include "gbx_event.h"
#include "gbx_exec.h"
#include "gbx_c_array.h"
#include "gbx_c_gambas.h"


static int nvararg(void)
{
	if (FP && FP->vararg)
		return BP - PP;
	else
		return 0;
}

BEGIN_PROPERTY(Param_Count)

	GB_ReturnInteger(nvararg());

END_PROPERTY


BEGIN_PROPERTY(Param_Max)

	GB_ReturnInteger(nvararg() - 1);

END_PROPERTY


BEGIN_METHOD(Param_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= nvararg())
		THROW(E_BOUND);

	TEMP = PP[index];
	GB_ReturnConvVariant();
	//VALUE_conv(&TEMP, T_VARIANT);

END_METHOD


BEGIN_PROPERTY(Param_All)

	GB_ARRAY all;
	int nparam = nvararg();
	int i;
	
	GB_ArrayNew(POINTER(&all), T_VARIANT, nparam);
	
	for (i = 0; i < nparam; i++)
	{
		VALUE_conv(&PP[i], T_VARIANT);
		GB_StoreVariant((GB_VARIANT *)&PP[i], GB_ArrayGet(all, i));
	}
	
	GB_ReturnObject(all);

END_PROPERTY


BEGIN_METHOD_VOID(Param_next)

	int *index = (int *)GB_GetEnum();

	if (*index >= nvararg())
		GB_StopEnum();
	else
	{
		TEMP = PP[*index];
		(*index)++;
		GB_ReturnConvVariant();
		//VALUE_conv(&TEMP, T_VARIANT);
	}

END_METHOD


BEGIN_PROPERTY(Param_Name)

	GB_ReturnConstZeroString(EXEC_unknown_name);

END_PROPERTY


/*BEGIN_PROPERTY(Param_Property)

	GB_ReturnBoolean(EXEC_unknown_property);

END_PROPERTY*/

BEGIN_PROPERTY(Param_EventName)

	GB_ReturnConstZeroString(EVENT_Name);

END_PROPERTY

#endif

GB_DESC NATIVE_Param[] =
{
	GB_DECLARE("Param", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("Count", "i", Param_Count),
	GB_STATIC_PROPERTY_READ("Max", "i", Param_Max),
	GB_STATIC_PROPERTY_READ("All", "Variant[]", Param_All),

	GB_STATIC_PROPERTY_READ("Name", "s", Param_Name),
	GB_STATIC_PROPERTY_READ("EventName", "s", Param_EventName),
	//GB_STATIC_PROPERTY_READ("Property", "b", Param_Property),

	GB_STATIC_METHOD("_get", "v", Param_get, "(Index)i"),
	GB_STATIC_METHOD("_next", "v", Param_next, NULL),

	//GB_STATIC_METHOD("Copy", "Variant[]", CPARAM_copy, "[(Start)i(Length)i]"),

	GB_END_DECLARE
};

GB_DESC NATIVE_Gambas[] =
{
	GB_DECLARE("gb", 0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("Binary", "i", GB_COMP_BINARY),
	GB_CONSTANT("IgnoreCase", "i", GB_COMP_NOCASE),
	GB_CONSTANT("Language", "i", GB_COMP_LANG),
	GB_CONSTANT("Like", "i", GB_COMP_LIKE),
	GB_CONSTANT("Natural", "i", GB_COMP_NATURAL),

	GB_CONSTANT("Ascent", "i", GB_COMP_ASCENT),
	GB_CONSTANT("Descent", "i", GB_COMP_DESCENT),

	/* BE CAREFUL ! These constants are used in the compiler */
	GB_CONSTANT("Null", "i", T_NULL),
	GB_CONSTANT("Boolean", "i", T_BOOLEAN),
	GB_CONSTANT("Byte", "i", T_BYTE),
	GB_CONSTANT("Short", "i", T_SHORT),
	GB_CONSTANT("Integer", "i", T_INTEGER),
	GB_CONSTANT("Long", "i", T_LONG),
	GB_CONSTANT("Float", "i", T_FLOAT),
	GB_CONSTANT("Single", "i", T_SINGLE),
	GB_CONSTANT("Date", "i", T_DATE),
	GB_CONSTANT("String", "i" , T_STRING),
	GB_CONSTANT("Pointer", "i" , T_POINTER),
	GB_CONSTANT("Function", "i" , T_FUNCTION),
	GB_CONSTANT("Variant", "i", T_VARIANT),
	GB_CONSTANT("Class", "i" , T_CLASS),
	GB_CONSTANT("Object", "i", T_OBJECT),

	GB_CONSTANT("File", "i", GB_STAT_FILE),
	GB_CONSTANT("Directory", "i", GB_STAT_DIRECTORY),
	GB_CONSTANT("Device", "i", GB_STAT_DEVICE),
	GB_CONSTANT("Pipe", "i", GB_STAT_PIPE),
	GB_CONSTANT("Socket", "i", GB_STAT_SOCKET),
	GB_CONSTANT("Link", "i", GB_STAT_LINK),

	GB_CONSTANT("NewLine", "s", "\n"),
	GB_CONSTANT("Tab", "s", "\t"),
	GB_CONSTANT("Cr", "s", "\r"),
	GB_CONSTANT("Lf", "s", "\n"),
	GB_CONSTANT("CrLf", "s", "\r\n"),

	GB_CONSTANT("Standard", "i", LF_STANDARD),
	GB_CONSTANT("GeneralNumber", "i", LF_GENERAL_NUMBER),
	GB_CONSTANT("ShortNumber", "i", LF_SHORT_NUMBER),
	GB_CONSTANT("Fixed", "i", LF_FIXED),
	GB_CONSTANT("Percent", "i", LF_PERCENT),
	GB_CONSTANT("Scientific", "i", LF_SCIENTIFIC),
	GB_CONSTANT("Currency", "i", LF_CURRENCY),
	GB_CONSTANT("International", "i", LF_INTERNATIONAL),
	GB_CONSTANT("GeneralDate", "i", LF_GENERAL_DATE),
	GB_CONSTANT("LongDate", "i", LF_LONG_DATE),
	GB_CONSTANT("MediumDate", "i", LF_MEDIUM_DATE),
	GB_CONSTANT("ShortDate", "i", LF_SHORT_DATE),
	GB_CONSTANT("LongTime", "i", LF_LONG_TIME),
	GB_CONSTANT("MediumTime", "i", LF_MEDIUM_TIME),
	GB_CONSTANT("ShortTime", "i", LF_SHORT_TIME),

	GB_CONSTANT("Read", "i", R_OK),
	GB_CONSTANT("Write", "i", W_OK),
	GB_CONSTANT("Exec", "i", X_OK),

	GB_CONSTANT("Sunday", "i", 0),
	GB_CONSTANT("Monday", "i", 1),
	GB_CONSTANT("Tuesday", "i", 2),
	GB_CONSTANT("Wednesday", "i", 3),
	GB_CONSTANT("Thursday", "i", 4),
	GB_CONSTANT("Friday", "i", 5),
	GB_CONSTANT("Saturday", "i", 6),

	GB_CONSTANT("Millisecond", "i", DP_MILLISECOND),
	GB_CONSTANT("Second", "i", DP_SECOND),
	GB_CONSTANT("Minute", "i", DP_MINUTE),
	GB_CONSTANT("Hour", "i", DP_HOUR),
	GB_CONSTANT("Day", "i", DP_DAY),
	GB_CONSTANT("WeekDay", "i", DP_WEEKDAY),
	GB_CONSTANT("Week", "i", DP_WEEK),
	GB_CONSTANT("Month", "i", DP_MONTH),
	GB_CONSTANT("Quarter", "i", DP_QUARTER),
	GB_CONSTANT("Year", "i", DP_YEAR),

	GB_CONSTANT("LittleEndian", "i", 0),
	GB_CONSTANT("BigEndian", "i", 1),

	GB_CONSTANT("Unix", "i", 0),
	GB_CONSTANT("Windows", "i", 1),
	GB_CONSTANT("Mac", "i", 2),

	GB_END_DECLARE
};

