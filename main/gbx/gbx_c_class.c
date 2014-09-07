/***************************************************************************

  gbx_c_class.c

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

#define __GBX_C_CLASS_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include <ctype.h>

#include "gb_common.h"
#include "gbx_api.h"
#include "gbx_component.h"
#include "gbx_project.h"
#include "gbx_class.h"
#include "gbx_class_desc.h"
#include "gbx_exec.h"
#include "gbx_event.h"
#include "gbx_object.h"
#include "gbx_c_array.h"
#include "gbx_c_observer.h"
#include "gbx_c_class.h"

static CLASS_DESC_SYMBOL *_current_symbol = NULL;

static void error(int code, CLASS *class, const char *name)
{
	GB_Error((char *)(intptr_t)code, CLASS_get_name(class), name);
}

//---- Components ---------------------------------------------------------

BEGIN_METHOD(Components_get, GB_STRING name)

	const char *name = GB_ToZeroString(ARG(name));
	COMPONENT *comp;

	comp = COMPONENT_find(name);
	GB_ReturnObject(comp);

END_METHOD

BEGIN_PROPERTY(Components_Count)

	GB_ReturnInt(COMPONENT_count);

END_PROPERTY

BEGIN_METHOD_VOID(Components_next)

	COMPONENT **plib = (COMPONENT **)GB_GetEnum();

	*plib = COMPONENT_next(*plib);
	if (*plib == NULL)
		GB_StopEnum();
	else
		GB_ReturnObject(*plib);

END_METHOD

BEGIN_PROPERTY(Component_Name)

	GB_ReturnConstZeroString(OBJECT(COMPONENT)->name);

END_PROPERTY

BEGIN_PROPERTY(Component_Path)

	GB_ReturnString(COMPONENT_path);

END_PROPERTY

BEGIN_METHOD(Component_Load, GB_STRING name)

	const char *name = GB_ToZeroString(ARG(name));
	COMPONENT *comp;

	comp = COMPONENT_find(name);
	if (!comp)
		comp = COMPONENT_create(name);

	COMPONENT_load(comp);

	GB_ReturnObject(comp);

END_METHOD

BEGIN_METHOD(Component_IsLoaded, GB_STRING name)

	const char *name = GB_ToZeroString(ARG(name));
	COMPONENT *comp;

	comp = COMPONENT_find(name);
	GB_ReturnBoolean(comp && comp->loaded);

END_METHOD


//---- Classes ------------------------------------------------------------

BEGIN_METHOD(Classes_get, GB_STRING name)

	const char *name = GB_ToZeroString(ARG(name));
	CLASS *class = NULL;

	if (name != NULL)
		class = CLASS_look(name, LENGTH(name));

	if (class == NULL)
	{
		GB_Error("Unknown class '&1'", name);
		return;
	}

	if (!CLASS_is_loaded(class))
	{
		GB_Error("Class is not loaded");
		return;
	}

	GB_ReturnObject(class);

END_METHOD


BEGIN_METHOD(Class_Load, GB_STRING name)

	CLASS *class;

	class = CLASS_get(GB_ToZeroString(ARG(name)));
	GB_ReturnObject(class);

END_METHOD


BEGIN_METHOD_VOID(Classes_next)

	CLASS **pcurrent = (CLASS **)GB_GetEnum();
	CLASS *class;

	class = *pcurrent;
	
	for(;;)
	{
		if (!class)
			class = CLASS_get_list();
		else
			class = class->next;
		
		if (!class)
		{
			GB_StopEnum();
			break;
		}

		if (CLASS_is_loaded(class))
		{
			GB_ReturnObject(class);
			break;
		}
		
	}

	*pcurrent = class;
	
END_METHOD


BEGIN_PROPERTY(Classes_count)

	GB_ReturnInt(CLASS_count());

END_PROPERTY


//---- Class --------------------------------------------------------------

BEGIN_PROPERTY(Class_Name)

	GB_ReturnConstZeroString(OBJECT(CLASS)->name);

END_PROPERTY


BEGIN_PROPERTY(Class_Count)

	GB_ReturnInt(OBJECT(CLASS)->count);

END_PROPERTY


BEGIN_PROPERTY(Class_Hidden)

	GB_ReturnBoolean(*(CLASS_get_name(OBJECT(CLASS))) == '.');

END_PROPERTY


BEGIN_PROPERTY(Class_Native)

	GB_ReturnBoolean(CLASS_is_native(OBJECT(CLASS)));

END_PROPERTY


BEGIN_PROPERTY(Class_Component)

	GB_ReturnObject(OBJECT(CLASS)->component);

END_PROPERTY


BEGIN_PROPERTY(Class_Parent)

	GB_ReturnObject(OBJECT(CLASS)->parent);

END_PROPERTY


BEGIN_PROPERTY(Class_Symbols)

	CLASS *class = OBJECT(CLASS);
	GB_ARRAY array;
	CLASS_DESC_SYMBOL *cds;
	int index = 0;
	
	GB_ArrayNew(&array, T_STRING, 0);
	
	for(;;)
	{
		cds = CLASS_get_next_sorted_symbol(class, &index);
		if (!cds)
			break;
		*((char **)GB_ArrayAdd(array)) = STRING_new(cds->name, cds->len);
	}
	
	GB_ReturnObject(array);

END_PROPERTY


BEGIN_METHOD(Class_get, GB_STRING name)

	CLASS *class = OBJECT(CLASS);
	CLASS_DESC_SYMBOL *cd = NULL;
	const char *name = GB_ToZeroString(ARG(name));

	if (name != NULL)
		cd = CLASS_get_symbol(class, name);

	if (cd == NULL)
	{
		GB_Error("Unknown symbol '&1'", name);
		return;
	}

	_current_symbol = cd;
	GB_ReturnObject(class);

END_METHOD


BEGIN_METHOD(Class_Exist, GB_STRING name)

	CLASS *class = OBJECT(CLASS);
	const char *name = GB_ToZeroString(ARG(name));

	GB_ReturnBoolean(name != NULL && CLASS_get_symbol(class, name) != NULL);

END_METHOD


BEGIN_PROPERTY(Class_Instance)

	GB_ReturnObject(OBJECT(CLASS)->instance);

END_PROPERTY


BEGIN_METHOD_VOID(Class_AutoCreate)

	CLASS *class = OBJECT(CLASS);
	
	if (!class->auto_create)
		GB_ReturnNull();
	else
		GB_ReturnObject(CLASS_auto_create(class, 0));

END_METHOD


BEGIN_METHOD(Class_New, GB_OBJECT params)

	CLASS *class = OBJECT(CLASS);
	GB_ARRAY params = VARGOPT(params, NULL);
	int i, np = 0;
	void *object;

	if (params)
		np = GB_ArrayCount(params);

	if (np)
	{
		STACK_check(np);

		for (i = 0; i < np; i++)
		{
			CARRAY_get_value(params, i, SP);
			PUSH();
		}
	}

	object = EXEC_create_object(class, np, NULL);
	OBJECT_UNREF_KEEP(object);
	GB_ReturnObject(object);

END_METHOD


//---- Symbol -------------------------------------------------------------

BEGIN_PROPERTY(Symbol_Name)

	GB_ReturnConstString(_current_symbol->name, _current_symbol->len);

END_PROPERTY


BEGIN_PROPERTY(Symbol_Kind)

	CLASS_DESC_SYMBOL *cds = _current_symbol;

	switch(CLASS_DESC_get_type(cds->desc))
	{
		case CD_VARIABLE:
		case CD_STATIC_VARIABLE:
			GB_ReturnInt(1);
			return;

		case CD_PROPERTY:
		case CD_PROPERTY_READ:
		case CD_STATIC_PROPERTY:
		case CD_STATIC_PROPERTY_READ:
			GB_ReturnInt(2);
			return;

		case CD_METHOD:
		case CD_STATIC_METHOD:
			GB_ReturnInt(3);
			return;

		case CD_EVENT:
			GB_ReturnInt(4);
			return;

		case CD_CONSTANT:
			GB_ReturnInt(5);
			return;

		default:
			GB_ReturnInt(0);
			return;
	}

END_PROPERTY


BEGIN_PROPERTY(Symbol_Static)

	CLASS_DESC_SYMBOL *cds = _current_symbol;

	GB_ReturnBoolean(index(CD_STATIC_LIST, CLASS_DESC_get_type(cds->desc)) != NULL);

END_PROPERTY


BEGIN_PROPERTY(Symbol_Hidden)

	CLASS_DESC_SYMBOL *cds = _current_symbol;

	GB_ReturnBoolean(*cds->name == '_');

END_PROPERTY


BEGIN_PROPERTY(Symbol_ReadOnly)

	CLASS_DESC_SYMBOL *cds = _current_symbol;

	switch (CLASS_DESC_get_type(cds->desc))
	{
		case CD_PROPERTY:
		case CD_STATIC_PROPERTY:
			GB_ReturnBoolean(FALSE);
			break;

		default:
			GB_ReturnBoolean(TRUE);
			break;
	}

END_PROPERTY


BEGIN_PROPERTY(Symbol_Type)

	CLASS_DESC_SYMBOL *cds = _current_symbol;

	GB_ReturnConstZeroString(TYPE_to_string(cds->desc->property.type)); /* Valable pour tout symbole */

END_PROPERTY


BEGIN_PROPERTY(Symbol_Signature)

	CLASS_DESC_SYMBOL *cds = _current_symbol;
	char *sign = CLASS_DESC_get_signature(cds->desc);

	if (sign)
	{
		STRING_free_later(sign);
		GB_ReturnString(sign);
	}
	else
		GB_ReturnVoidString();

END_METHOD


BEGIN_PROPERTY(Symbol_Value)

	CLASS_DESC *desc = _current_symbol->desc;

	if (CLASS_DESC_get_type(desc) != CD_CONSTANT)
	{
		GB_ReturnVariant(NULL);
		return;
	}

	if (desc->constant.type == T_STRING)
		GB_ReturnConstZeroString(desc->constant.value._string);
	else
		GB_ReturnPtr(desc->constant.type, (void *)&desc->constant.value);
	
	GB_ReturnConvVariant();

END_PROPERTY


//---- Object -------------------------------------------------------------

BEGIN_METHOD(Object_GetProperty, GB_OBJECT object; GB_STRING property)

	const char *name;
	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	name = GB_ToZeroString(ARG(property));

	GB_GetProperty(object, name);
	GB_ReturnConvVariant();

END_METHOD


BEGIN_METHOD(Object_SetProperty, GB_OBJECT object; GB_STRING property; GB_VARIANT value)

	const char *name;
	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	name = GB_ToZeroString(ARG(property));

	GB_SetProperty(object, name, (GB_VALUE *)ARG(value));

END_METHOD


BEGIN_METHOD(Object_Attach, GB_OBJECT object; GB_OBJECT parent; GB_STRING name)

	void *object = VARG(object);
	void *parent = VARG(parent);
	char *name = GB_ToZeroString(ARG(name));

	if (GB_CheckObject(object))
		return;

	if (!parent || !name || !*name)
	{
		OBJECT_detach(object);
		return;
	}

	if (GB_CheckObject(parent))
		return;

	OBJECT_attach(object, parent, name);
	
	/*if (OBJECT_is(object, CLASS_Observer))
		COBSERVER_attach((COBSERVER *)object, parent, GB_ToZeroString(ARG(name)));*/

END_METHOD


BEGIN_METHOD(Object_Detach, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	if (OBJECT_is(object, CLASS_Observer))
		COBSERVER_detach((COBSERVER *)object);
	
	OBJECT_detach(object);

END_METHOD


BEGIN_METHOD(Object_Parent, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	GB_ReturnObject(OBJECT_parent(object));

END_METHOD


BEGIN_METHOD(Object_Class, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	GB_ReturnObject(OBJECT_class(object));

END_METHOD


BEGIN_METHOD(Object_Type, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	GB_ReturnConstZeroString(OBJECT_class(object)->name);

END_METHOD


BEGIN_METHOD(Object_Is, GB_OBJECT object; GB_STRING class)

	void *object = VARG(object);
	CLASS *class = CLASS_look(STRING(class), LENGTH(class));

	if (GB_CheckObject(object))
		return;

	if (!class)
	{
		GB_ReturnBoolean(FALSE);
		return;
	}

	GB_ReturnBoolean(OBJECT_class(object) == class || CLASS_inherits(OBJECT_class(object), class));

END_METHOD


BEGIN_METHOD(Object_Call, GB_OBJECT object; GB_STRING method; GB_OBJECT params)

	int i;
	int np = 0;
	char *name = GB_ToZeroString(ARG(method));
	void *object = VARG(object);
	GB_FUNCTION func;
	GB_ARRAY params = VARGOPT(params, NULL);

	if (GB_CheckObject(object))
		return;
	
	if (GB_GetFunction(&func, object, name, NULL, NULL))
	{
		error(E_NSYMBOL, OBJECT_class(object), name);
		return;
	}

	if (params)
		np = GB_ArrayCount(params);

	if (np)
	{
		STACK_check(np);

		for (i = 0; i < np; i++)
		{
			CARRAY_get_value(params, i, SP);
			PUSH();
		}
	}

	GB_Call(&func, np, FALSE);
	
	if (TEMP.type != T_VOID)
		GB_ReturnConvVariant();

END_METHOD


BEGIN_METHOD(Object_IsValid, GB_OBJECT object)

	GB_ReturnBoolean(OBJECT_is_valid(VARG(object)));

END_METHOD


BEGIN_METHOD(Object_Lock, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	OBJECT_lock(object, TRUE);

END_METHOD


BEGIN_METHOD(Object_Unlock, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	OBJECT_lock(object, FALSE);

END_METHOD


BEGIN_METHOD(Object_IsLocked, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	GB_ReturnBoolean(OBJECT_is_locked(object));

END_METHOD


BEGIN_METHOD(Object_Count, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	// We substract one because the object is referenced on the stack
	GB_ReturnInteger(OBJECT_count(object) - 1);

END_METHOD


BEGIN_METHOD(Object_SizeOf, GB_OBJECT object)

	void *object = VARG(object);

	if (GB_CheckObject(object))
		return;

	GB_ReturnInteger(CLASS_sizeof(OBJECT_class(object)));

END_METHOD


BEGIN_METHOD(Object_New, GB_STRING class; GB_OBJECT params)

	CLASS *class = CLASS_find(GB_ToZeroString(ARG(class)));
	GB_ARRAY params = VARGOPT(params, NULL);
	int i, np = 0;
	void *object;

	if (!class)
	{
		GB_Error("Unknown class");
		return;
	}

	if (params)
		np = GB_ArrayCount(params);

	if (np)
	{
		STACK_check(np);

		for (i = 0; i < np; i++)
		{
			CARRAY_get_value(params, i, SP);
			PUSH();
		}
	}

	object = EXEC_create_object(class, np, NULL);
	OBJECT_UNREF_KEEP(object);
	GB_ReturnObject(object);

END_METHOD


BEGIN_PROPERTY(Object_Address)

	GB_ReturnPointer(VPROP(GB_OBJECT));

END_PROPERTY


BEGIN_METHOD(Object_CanRaise, GB_OBJECT object; GB_STRING name)

	void *object = VARG(object);
	CLASS *class;
	char *name;
	int index;

	if (!object)
	{
		GB_Error((char *)E_NULL);
		return;
	}

	class = OBJECT_class(object);
	name = GB_ToZeroString(ARG(name));
	index = GB_GetEvent(class, name);
	//fprintf(stderr, "Object.CanRaise: %s %d\n", name, index);
	if (index < 0)
		GB_ReturnBoolean(FALSE);
	else
		GB_ReturnBoolean(GB_CanRaise(object, index));
	
END_METHOD

#endif


//-------------------------------------------------------------------------

GB_DESC NATIVE_Symbol[] =
{
	GB_DECLARE_VIRTUAL(".Symbol"),

	GB_PROPERTY_READ("Name", "s", Symbol_Name),
	GB_PROPERTY_READ("Kind", "i", Symbol_Kind),
	GB_PROPERTY_READ("Type", "s", Symbol_Type),
	GB_PROPERTY_READ("ReadOnly", "b", Symbol_ReadOnly),
	GB_PROPERTY_READ("Hidden", "b", Symbol_Hidden),
	GB_PROPERTY_READ("Signature", "s", Symbol_Signature),
	GB_PROPERTY_READ("Static", "b", Symbol_Static),
	GB_PROPERTY_READ("Value", "v", Symbol_Value),

	GB_END_DECLARE
};

GB_DESC NATIVE_Components[] =
{
	GB_DECLARE_STATIC("Components"),

	GB_STATIC_METHOD("_next", "Component", Components_next, NULL),
	GB_STATIC_METHOD("_get", "Component", Components_get, "(Name)s"),
	GB_STATIC_PROPERTY_READ("Count", "i", Components_Count),

	GB_END_DECLARE
};

GB_DESC NATIVE_Component[] =
{
	GB_DECLARE("Component", sizeof(COMPONENT)),
	GB_NOT_CREATABLE(),

	//GB_STATIC_METHOD("_get", "Component", library_get, "(Name)s"),
	GB_STATIC_METHOD("Load", "Component", Component_Load, "(Name)s"),
	GB_STATIC_METHOD("IsLoaded", "b", Component_IsLoaded, "(Name)s"),
	GB_STATIC_PROPERTY_READ("Path", "s", Component_Path),

	GB_PROPERTY_READ("Name", "s", Component_Name),

	GB_END_DECLARE
};

GB_DESC NATIVE_Classes[] =
{
	GB_DECLARE_STATIC("Classes"),

	GB_STATIC_METHOD("_next", "Class", Classes_next, NULL),
	GB_STATIC_METHOD("_get", "Class", Classes_get, "(Name)s"),
	GB_STATIC_PROPERTY_READ("Count", "i", Classes_count),

	GB_END_DECLARE
};

GB_DESC NATIVE_Class[] =
{
	GB_DECLARE("Class", sizeof(CLASS)),
	GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("Load", "Class", Class_Load, "(Name)s"),

	GB_METHOD("_get", ".Symbol", Class_get, "(Symbol)s"),

	GB_PROPERTY_READ("Name", "s", Class_Name),
	GB_PROPERTY_READ("Hidden", "b", Class_Hidden),
	GB_PROPERTY_READ("Native", "b", Class_Native),
	GB_PROPERTY_READ("Parent", "Class", Class_Parent),
	GB_PROPERTY_READ("Component", "Component", Class_Component),
	GB_PROPERTY_READ("Count", "i", Class_Count),
	GB_PROPERTY_READ("Instance", "o", Class_Instance),
	GB_PROPERTY_READ("Symbols", "String[]", Class_Symbols),
	GB_METHOD("AutoCreate", "o", Class_AutoCreate, NULL),
	GB_METHOD("New", "o", Class_New, "[(Arguments)Array;]"),
	GB_METHOD("Exist", "b", Class_Exist, "(Symbol)s"),

	GB_CONSTANT("Variable", "i", 1),
	GB_CONSTANT("Property", "i", 2),
	GB_CONSTANT("Method", "i", 3),
	GB_CONSTANT("Event", "i", 4),
	GB_CONSTANT("Constant", "i", 5),

	GB_END_DECLARE
};

GB_DESC NATIVE_Object[] =
{
	GB_DECLARE_STATIC("Object"),

	GB_STATIC_METHOD("GetProperty", "v", Object_GetProperty, "(Object)o(Property)s"),
	GB_STATIC_METHOD("SetProperty", NULL, Object_SetProperty, "(Object)o(Property)s(Value)v"),
	GB_STATIC_METHOD("Attach", NULL, Object_Attach, "(Object)o(Parent)o(Name)s"),
	GB_STATIC_METHOD("Detach", NULL, Object_Detach, "(Object)o"),
	GB_STATIC_METHOD("Class", "Class", Object_Class, "(Object)o"),
	GB_STATIC_METHOD("Type", "s", Object_Type, "(Object)o"),
	GB_STATIC_METHOD("Is", "b", Object_Is, "(Object)o(Class)s"),
	GB_STATIC_METHOD("Parent", "o", Object_Parent, "(Object)o"),
	GB_STATIC_METHOD("Call", "v", Object_Call, "(Object)o(Method)s[(Arguments)Array;]"),
	GB_STATIC_METHOD("New", "o", Object_New, "(Class)s[(Arguments)Array;]"),
	GB_STATIC_METHOD("IsValid", "b", Object_IsValid, "(Object)o"),
	GB_STATIC_METHOD("Lock", NULL, Object_Lock, "(Object)o"),
	GB_STATIC_METHOD("Unlock", NULL, Object_Unlock, "(Object)o"),
	GB_STATIC_METHOD("IsLocked", "b", Object_IsLocked, "(Object)o"),
	GB_STATIC_METHOD("Count", "i", Object_Count, "(Object)o"),
	GB_STATIC_METHOD("SizeOf", "i", Object_SizeOf, "(Object)o"),
	GB_STATIC_METHOD("Address", "p", Object_Address, "(Object)o"),
	GB_STATIC_METHOD("CanRaise", "b", Object_CanRaise, "(Object)o(Event)s"),

	GB_END_DECLARE
};

