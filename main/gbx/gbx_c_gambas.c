/***************************************************************************

  gbx_c_gambas.c

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
#include "gbx_exec.h"

#include "gbx_event.h"
#include "gbx_c_gambas.h"


static int nvararg(void)
{
  if (FP && FP->vararg)
    return BP - PP;
  else
    return 0;
}

BEGIN_PROPERTY(CPARAM_count)

  GB_ReturnInteger(nvararg());

END_PROPERTY


BEGIN_PROPERTY(CPARAM_max)

  GB_ReturnInteger(nvararg() - 1);

END_PROPERTY


BEGIN_METHOD(CPARAM_get, GB_INTEGER index)

  int index = VARG(index);

  if (index < 0 || index >= nvararg())
    THROW(E_BOUND);

  TEMP = PP[index];
  //VALUE_conv(&TEMP, T_VARIANT);

END_METHOD


BEGIN_METHOD_VOID(CPARAM_next)

  int *index = (int *)GB_GetEnum();

  if (*index >= nvararg())
    GB_StopEnum();
  else
  {
    TEMP = PP[*index];
    (*index)++;
    //VALUE_conv(&TEMP, T_VARIANT);
  }

END_METHOD


BEGIN_PROPERTY(CUNKNOWN_name)

  GB_ReturnConstZeroString(EXEC.unknown);

END_PROPERTY


BEGIN_PROPERTY(CUNKNOWN_property)

  GB_ReturnBoolean(EXEC.property);

END_PROPERTY


void COBSERVER_attach(COBSERVER *this, void *parent, const char *name)
{
	EVENT_search(OBJECT_class(this->object), this->event, name, parent);	
}

void COBSERVER_detach(COBSERVER *this)
{
	if (this->event)
		FREE(&this->event, "COBSERVER_detach");
}

BEGIN_METHOD(COBSERVER_new, GB_OBJECT object; GB_BOOLEAN after)

	OBJECT *object;
	OBJECT_EVENT *ev;
	char *name;
	CLASS *class;
	COBSERVER *this;
	OBJECT *parent;
	
	object = (OBJECT *)VARG(object);
	if (GB_CheckObject(object))
		return;
	
	class = OBJECT_class(object);
	if (class->n_event == 0)
		return;
	
	name = EVENT_Name;
	if (!name || !*name)
		return;
	
	this = ((COBSERVER *)_object);
	parent = OBJECT_parent(this);
	if (!parent)
		return;
	
	ev = OBJECT_event(object);
  
  this->after = VARGOPT(after, FALSE);
  
	ALLOC_ZERO(&this->event, sizeof(ushort) * class->n_event, "COBSERVER_new");

	this->object = object;
	COBSERVER_attach(this, parent, name);
	//EVENT_search(class, this->event, name, parent);

  LIST_insert((void **)&ev->observer, this, &this->list);
  OBJECT_REF(this, "COBSERVER_new");

END_METHOD


BEGIN_METHOD_VOID(COBSERVER_free)

	COBSERVER_detach((COBSERVER *)_object);

END_METHOD

BEGIN_PROPERTY(gb_Text)

	ERROR_deprecated("gb.Text");
	GB_ReturnInteger(GB_COMP_NOCASE);

END_PROPERTY

#endif

GB_DESC NATIVE_Param[] =
{
  GB_DECLARE("Param", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Count", "i", CPARAM_count),
  GB_STATIC_PROPERTY_READ("Max", "i", CPARAM_max),
  GB_STATIC_PROPERTY_READ("Length", "i", CPARAM_count),

  GB_STATIC_PROPERTY_READ("Name", "s", CUNKNOWN_name),
  GB_STATIC_PROPERTY_READ("Property", "b", CUNKNOWN_property),

  GB_STATIC_METHOD("_get", "v", CPARAM_get, "(Index)i"),
  GB_STATIC_METHOD("_next", "v", CPARAM_next, NULL),

  //GB_STATIC_METHOD("Copy", "Variant[]", CPARAM_copy, "[(Start)i(Length)i]"),

  GB_END_DECLARE
};

GB_DESC NATIVE_Observer[] =
{
  GB_DECLARE("Observer", sizeof(COBSERVER)), 

  GB_METHOD("_new", NULL, COBSERVER_new, "(Object)o[(After)b]"),
  GB_METHOD("_free", NULL, COBSERVER_free, NULL),

  GB_END_DECLARE
};

/*
GB_DESC NATIVE_Unknown[] =
{
  GB_DECLARE("Unknown", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Name", "s", CUNKNOWN_name),
  GB_STATIC_PROPERTY_READ("Property", "b", CUNKNOWN_property),

  GB_END_DECLARE
};
*/

GB_DESC NATIVE_Gambas[] =
{
  GB_DECLARE("gb", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Binary", "i", GB_COMP_BINARY),
  GB_STATIC_PROPERTY_READ("Text", "i", gb_Text),
  GB_CONSTANT("IgnoreCase", "i", GB_COMP_NOCASE),
  GB_CONSTANT("Language", "i", GB_COMP_LANG),
	GB_CONSTANT("Like","i",GB_COMP_LIKE),
	GB_CONSTANT("Natural","i",GB_COMP_NATURAL),

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

  GB_CONSTANT("Standard", "i", LF_STANDARD),
  GB_CONSTANT("GeneralNumber", "i", LF_GENERAL_NUMBER),
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

  //GB_CONSTANT("User", "i", GB_STAT_USER),
  //GB_CONSTANT("Group", "i", GB_STAT_GROUP),
  //GB_CONSTANT("Other", "i", GB_STAT_OTHER),

  GB_CONSTANT("Sunday", "i", 0),
  GB_CONSTANT("Monday", "i", 1),
  GB_CONSTANT("Tuesday", "i", 2),
  GB_CONSTANT("Wednesday", "i", 3),
  GB_CONSTANT("Thursday", "i", 4),
  GB_CONSTANT("Friday", "i", 5),
  GB_CONSTANT("Saturday", "i", 6),

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

