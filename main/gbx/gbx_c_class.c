/***************************************************************************

  CClass.c

  The native class Class needed for introspection

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

#include "gbx_object.h"
#include "gbx_c_array.h"
#include "gbx_c_class.h"

static CLASS_DESC_SYMBOL *_current_symbol = NULL;

static CLASS_DESC *get_desc(CLASS *class, const char *name)
{
  int index;

  index = CLASS_find_symbol(class, name);

	//fprintf(stderr, "CLASS_find_symbol('%s', '%s') = %ld\n", class->name, name, index);

  if (index == NO_SYMBOL)
  {
    GB_Error((char *)E_NSYMBOL, name, class->name);
    return NULL;
  }
  else
    return class->table[index].desc;
}

/**************************************************************************************************/

BEGIN_METHOD(component_get, GB_STRING name)

  const char *name = GB_ToZeroString(ARG(name));
  COMPONENT *comp;

  comp = COMPONENT_find(name);
  GB_ReturnObject(comp);

END_METHOD


BEGIN_PROPERTY(component_count)

  GB_ReturnInt(COMPONENT_count);

END_PROPERTY


BEGIN_METHOD_VOID(component_next)

  COMPONENT **plib = (COMPONENT **)GB_GetEnum();

  *plib = COMPONENT_next(*plib);
  if (*plib == NULL)
    GB_StopEnum();
  else
    GB_ReturnObject(*plib);

END_METHOD


BEGIN_PROPERTY(component_name)

  GB_ReturnConstZeroString(OBJECT(COMPONENT)->name);

END_PROPERTY


BEGIN_PROPERTY(component_path)

  GB_ReturnString(COMPONENT_path);

END_PROPERTY


BEGIN_PROPERTY(component_user_path)

  GB_ReturnString(COMPONENT_user_path);

END_PROPERTY


BEGIN_METHOD(component_load, GB_STRING name)

  const char *name = GB_ToZeroString(ARG(name));
  COMPONENT *comp;

  comp = COMPONENT_find(name);
  if (!comp)
    comp = COMPONENT_create(name);

  COMPONENT_load(comp);

  GB_ReturnObject(comp);

END_METHOD


BEGIN_METHOD_VOID(component_unload)

  COMPONENT_unload(OBJECT(COMPONENT));

END_METHOD




/**************************************************************************************************/

BEGIN_METHOD(class_get, GB_STRING name)

  const char *name = GB_ToZeroString(ARG(name));
  CLASS *class = NULL;

  if (name != NULL)
    class = CLASS_look(name, LENGTH(name));

  if (class == NULL)
  {
    GB_Error("Unknown class '&1'", name);
    return;
  }

  if (!class->state)
  {
    GB_Error("Class is not loaded");
    return;
  }

  GB_ReturnObject(class);

END_METHOD


BEGIN_METHOD(class_load, GB_STRING name)

  CLASS *class;

  class = CLASS_get(GB_ToZeroString(ARG(name)));
  GB_ReturnObject(class);

END_METHOD


BEGIN_PROPERTY(class_name)

  GB_ReturnConstZeroString(OBJECT(CLASS)->name);

END_PROPERTY


BEGIN_METHOD_VOID(class_next)

  TABLE *table = CLASS_get_table();
  int *index = (int *)GB_GetEnum();
  CLASS *class;

  for(;;)
  {
    if (*index >= TABLE_count(table))
    {
      GB_StopEnum();
      break;
    }

    class = ((CLASS_SYMBOL *)TABLE_get_symbol(table, *index))->class;
    (*index)++;

    if (class->state)
    {
      GB_ReturnObject(class);
      break;
    }
  }

END_METHOD


BEGIN_PROPERTY(class_count)

  GB_ReturnInt(CLASS_count());

END_PROPERTY


BEGIN_PROPERTY(class_object_count)

  GB_ReturnInt(OBJECT(CLASS)->count);

END_PROPERTY


BEGIN_PROPERTY(class_hidden)

  GB_ReturnBoolean(*(OBJECT(CLASS)->name) == '.');

END_PROPERTY


BEGIN_PROPERTY(class_native)

  GB_ReturnBoolean(CLASS_is_native(OBJECT(CLASS)));

END_PROPERTY


BEGIN_PROPERTY(class_component)

  GB_ReturnObject(OBJECT(CLASS)->component);

END_PROPERTY


BEGIN_PROPERTY(class_parent)

  GB_ReturnObject(OBJECT(CLASS)->parent);

END_PROPERTY



/**************************************************************************************************/

BEGIN_PROPERTY(class_symbols_count)

  CLASS *class = OBJECT(CLASS);
	int i, ndesc;
  CLASS_DESC *old = NULL;
  CLASS_DESC *desc;

	ndesc = class->n_desc;
	for (i = 0; i < class->n_desc; i++)
	{
		desc = class->table[class->table[i].sort].desc;
		if (!desc || desc == old)
			ndesc--;
		old = desc;
	}

  GB_ReturnInt(ndesc);

END_PROPERTY


BEGIN_METHOD_VOID(class_symbols_next)

  CLASS *class = OBJECT(CLASS);
  int *index = (int *)GB_GetEnum();
  CLASS_DESC_SYMBOL *cds;

	cds = CLASS_get_next_sorted_symbol(class, index);
	if (!cds)
		GB_StopEnum();
	else
	  GB_ReturnConstString(cds->name, cds->len);
/*
	for(;;)
	{
		if (*index >= class->n_desc)
		{
			GB_StopEnum();
			return;
		}

	  sort = class->table[*index].sort;
	  if (*index > 0)
	  	old = class->table[class->table[*index - 1].sort].desc;
  	(*index)++;
  	if (class->table[sort].desc && class->table[sort].desc != old)
  	{
  		fprintf(stderr, "%.*s: %p\n", class->table[sort].len, class->table[sort].name, class->table[sort].desc);
  		break;
		}
	}

  GB_ReturnConstString(class->table[sort].name, class->table[sort].len);*/

END_METHOD


BEGIN_METHOD(class_symbol_get, GB_STRING name)

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


BEGIN_PROPERTY(class_instance)

  GB_ReturnObject(OBJECT(CLASS)->instance);

END_PROPERTY



/**************************************************************************************************/

BEGIN_PROPERTY(symbol_name)

  GB_ReturnConstString(_current_symbol->name, _current_symbol->len);

END_PROPERTY


BEGIN_PROPERTY(symbol_kind)

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


BEGIN_PROPERTY(symbol_static)

  CLASS_DESC_SYMBOL *cds = _current_symbol;

  GB_ReturnBoolean(index(CD_STATIC_LIST, CLASS_DESC_get_type(cds->desc)) != NULL);

END_PROPERTY


BEGIN_PROPERTY(symbol_hidden)

  CLASS_DESC_SYMBOL *cds = _current_symbol;

  GB_ReturnBoolean(*cds->name == '_');

END_PROPERTY


BEGIN_PROPERTY(symbol_read_only)

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


BEGIN_PROPERTY(symbol_type)

  CLASS_DESC_SYMBOL *cds = _current_symbol;

  GB_ReturnConstZeroString(TYPE_to_string(cds->desc->property.type)); /* Valable pour tout symbole */

END_PROPERTY


BEGIN_PROPERTY(symbol_signature)

  CLASS_DESC_SYMBOL *cds = _current_symbol;

  GB_ReturnConstZeroString(CLASS_DESC_get_signature(cds->desc));

END_METHOD


BEGIN_PROPERTY(symbol_value)

  CLASS_DESC *desc = _current_symbol->desc;

  if (CLASS_DESC_get_type(desc) != CD_CONSTANT)
  {
    GB_ReturnNull();
    return;
  }

  if (desc->constant.type == T_STRING)
    GB_ReturnConstZeroString(desc->constant.value._string);
  else
    GB_ReturnPtr(desc->constant.type, (void *)&desc->constant.value);

END_PROPERTY




/**************************************************************************************************/


BEGIN_METHOD(object_get_property, GB_OBJECT object; GB_STRING property)

  const char *name;
  CLASS_DESC *desc;
  CLASS *class;
  char type;
  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  if (OBJECT_is_class(object))
  {
    class = (CLASS *)object;
    object = NULL;
  }
  else
  {
    class = OBJECT_class(object);
  }

  name = GB_ToZeroString(ARG(property));
  desc = get_desc(class, name);

  if (!desc)
    return;

  type = CLASS_DESC_get_type(desc);

  if (type == CD_PROPERTY || type == CD_PROPERTY_READ)
  {
    if (!object)
    {
      GB_Error((char *)E_DYNAMIC, class->name, name);
      return;
    }
  }
  else if (type == CD_STATIC_PROPERTY || type == CD_STATIC_PROPERTY_READ)
  {
    if (object)
    {
      GB_Error((char *)E_STATIC, class->name, name);
      return;
    }
  }
  else
  {
    GB_Error((char *)E_NPROPERTY, class->name, name);
    return;
  }

  if (desc->property.native)
  {
    if (EXEC_call_native(desc->property.read, object, desc->property.type, 0))
    {
    	GAMBAS_Error = TRUE;
      return;
		}
  }
  else
  {
    EXEC.class = desc->property.class;
    EXEC.object = object;
    EXEC.drop = FALSE;
    EXEC.nparam = 0;
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->property.read;
    //EXEC.func = &class->load->func[(long)desc->property.read];

    EXEC_function_keep();

    TEMP = *RP;
    UNBORROW(RP);
    RP->type = T_VOID;
  }

END_METHOD


BEGIN_METHOD(object_set_property, GB_OBJECT object; GB_STRING property; GB_VARIANT value)

  const char *name;
  CLASS_DESC *desc;
  CLASS *class;
  char type;
  void *object = VARG(object);
  VALUE *value = (VALUE *)ARG(value);

  if (GB_CheckObject(object))
    return;

  if (OBJECT_is_class(object))
  {
    class = (CLASS *)object;
    object = NULL;
  }
  else
  {
    class = OBJECT_class(object);
  }

  name = GB_ToZeroString(ARG(property));
  desc = get_desc(class, name);

  if (!desc)
    return;

  type = CLASS_DESC_get_type(desc);

  if (type == CD_PROPERTY)
  {
    if (!object)
    {
      GB_Error((char *)E_DYNAMIC, class->name, name);
      return;
    }
  }
  else if (type == CD_STATIC_PROPERTY)
  {
    if (object)
    {
      GB_Error((char *)E_STATIC, class->name, name);
      return;
    }
  }
  else if (type == CD_PROPERTY_READ  || type == CD_STATIC_PROPERTY_READ)
  {
    GB_Error((char *)E_NWRITE, class->name, name);
    return;
  }
  else
  {
    GB_Error((char *)E_NPROPERTY, class->name, name);
    return;
  }

  if (desc->property.native)
  {
    VALUE_conv(value, desc->property.type);

    if (EXEC_call_native(desc->property.write, object, 0, value))
    {
    	GAMBAS_Error = TRUE;
      return;
		}
  }
  else
  {
    *SP = *value;
    BORROW(SP);
    SP++;

    EXEC.class = desc->property.class;
    EXEC.object = object;
    EXEC.drop = FALSE;
    EXEC.nparam = 1;
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->property.write;
    //EXEC.func = &class->load->func[(long)desc->property.write];

    EXEC_function();

    /*VALUE_write(value, OBJECT_get_prop_addr(object, desc), desc->property.type);*/
  }

END_METHOD


BEGIN_METHOD(object_attach, GB_OBJECT object; GB_OBJECT parent; GB_STRING name)

  void *object = VARG(object);
  void *parent = VARG(parent);

  if (GB_CheckObject(object))
    return;

	if (!parent)
	{
		OBJECT_detach(object);
		return;
	}

  if (GB_CheckObject(parent))
    return;

  OBJECT_attach(object, parent, GB_ToZeroString(ARG(name)));

END_METHOD



BEGIN_METHOD(object_detach, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  OBJECT_detach(object);

END_METHOD


BEGIN_METHOD(object_get_parent, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  GB_ReturnObject(OBJECT_parent(object));

END_METHOD


BEGIN_METHOD(object_class, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  GB_ReturnObject(OBJECT_class(object));

END_METHOD


BEGIN_METHOD(object_type, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  GB_ReturnConstZeroString(OBJECT_class(object)->name);

END_METHOD


BEGIN_METHOD(object_is, GB_OBJECT object; GB_STRING class)

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


BEGIN_METHOD(object_call, GB_OBJECT object; GB_STRING method; GB_OBJECT params)

  int i;
  int np = 0;
  char *name = GB_ToZeroString(ARG(method));
  void *object = VARG(object);
  GB_FUNCTION func;
  GB_ARRAY params = VARGOPT(params, NULL);
  CLASS *class;

  if (GB_CheckObject(object))
    return;

  class = OBJECT_class(object);

  if (GB_GetFunction(&func, object, name, NULL, NULL))
  {
    GB_Error((char *)E_NSYMBOL, name, class->name);
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

END_METHOD


BEGIN_METHOD(object_is_valid, GB_OBJECT object)

  GB_ReturnBoolean(OBJECT_is_valid(VARG(object)));

END_METHOD


BEGIN_METHOD(object_lock, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  OBJECT_lock(object, TRUE);

END_METHOD


BEGIN_METHOD(object_unlock, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  OBJECT_lock(object, FALSE);

END_METHOD


BEGIN_METHOD(object_is_locked, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

  GB_ReturnBoolean(OBJECT_is_locked(object));

END_METHOD


BEGIN_METHOD(object_count, GB_OBJECT object)

  void *object = VARG(object);

  if (GB_CheckObject(object))
    return;

	// We substract one because the object is referenced on the stack
  GB_ReturnInteger(OBJECT_count(object) - 1);

END_METHOD


BEGIN_METHOD(object_new, GB_STRING class; GB_OBJECT params)

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
	OBJECT_UNREF_KEEP(&object, "object_new");
	GB_ReturnObject(object);

END_METHOD


#endif


GB_DESC NATIVE_Symbol[] =
{
  GB_DECLARE(".Symbol", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Name", "s", symbol_name),
  GB_PROPERTY_READ("Kind", "i", symbol_kind),
  GB_PROPERTY_READ("Type", "s", symbol_type),
  GB_PROPERTY_READ("ReadOnly", "b", symbol_read_only),
  GB_PROPERTY_READ("Hidden", "b", symbol_hidden),
  GB_PROPERTY_READ("Signature", "s", symbol_signature),
  GB_PROPERTY_READ("Static", "b", symbol_static),
  GB_PROPERTY_READ("Value", "v", symbol_value),

  GB_END_DECLARE
};


GB_DESC NATIVE_ClassSymbols[] =
{
  GB_DECLARE(".ClassSymbols", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "s", class_symbols_next, NULL),
  GB_PROPERTY_READ("Count", "i", class_symbols_count),

  GB_END_DECLARE
};


GB_DESC NATIVE_Components[] =
{
  GB_DECLARE("Components", 0),  GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("Load", "Component", component_load, "(Name)s"),
  GB_STATIC_METHOD("_next", "Component", component_next, NULL),
  GB_STATIC_METHOD("_get", "Component", component_get, "(Name)s"),
  GB_STATIC_PROPERTY_READ("Count", "i", component_count),

  GB_END_DECLARE
};


GB_DESC NATIVE_Component[] =
{
  GB_DECLARE("Component", 0),  GB_VIRTUAL_CLASS(),

  //GB_STATIC_METHOD("_get", "Component", library_get, "(Name)s"),
  GB_STATIC_METHOD("Load", "Component", component_load, "(Name)s"),
  GB_STATIC_PROPERTY_READ("Path", "s", component_path),
  GB_STATIC_PROPERTY_READ("UserPath", "s", component_user_path),

  GB_PROPERTY_READ("Name", "s", component_name),

  GB_END_DECLARE
};


GB_DESC NATIVE_Classes[] =
{
  GB_DECLARE("Classes", 0),  GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_next", "Class", class_next, NULL),
  GB_STATIC_METHOD("_get", "Class", class_get, "(Name)s"),
  GB_STATIC_PROPERTY_READ("Count", "i", class_count),

  GB_END_DECLARE
};


GB_DESC NATIVE_Class[] =
{
  GB_DECLARE("Class", 0),  GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("Load", "Class", class_load, "(Name)s"),

  GB_METHOD("_get", ".Symbol", class_symbol_get, "(Name)s"),
  GB_PROPERTY_SELF("Symbols", ".ClassSymbols"),

  GB_PROPERTY_READ("Name", "s", class_name),
  GB_PROPERTY_READ("Hidden", "b", class_hidden),
  GB_PROPERTY_READ("Native", "b", class_native),
  GB_PROPERTY_READ("Parent", "Class", class_parent),
  GB_PROPERTY_READ("Component", "Component", class_component),
  GB_PROPERTY_READ("Count", "i", class_object_count),
  GB_PROPERTY_READ("Instance", "o", class_instance),

  /*GB_METHOD("New", "Object", class_new, "."),*/

  GB_CONSTANT("Variable", "i", 1),
  GB_CONSTANT("Property", "i", 2),
  GB_CONSTANT("Method", "i", 3),
  GB_CONSTANT("Event", "i", 4),
  GB_CONSTANT("Constant", "i", 5),

  GB_END_DECLARE
};


GB_DESC NATIVE_Object[] =
{
  GB_DECLARE("Object", 0),  GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("GetProperty", "v", object_get_property, "(Object)o(Property)s"),
  GB_STATIC_METHOD("SetProperty", NULL, object_set_property, "(Object)o(Property)s(Value)v"),
  GB_STATIC_METHOD("Attach", NULL, object_attach, "(Object)o(Parent)o(Name)s"),
  GB_STATIC_METHOD("Detach", NULL, object_detach, "(Object)o"),
  GB_STATIC_METHOD("Class", "Class", object_class, "(Object)o"),
  GB_STATIC_METHOD("Type", "s", object_type, "(Object)o"),
  GB_STATIC_METHOD("Is", "b", object_is, "(Object)o(Class)s"),
  GB_STATIC_METHOD("Parent", "o", object_get_parent, "(Object)o"),
  GB_STATIC_METHOD("Call", "v", object_call, "(Object)o(Method)s[(Arguments)Array;]"),
  GB_STATIC_METHOD("New", "o", object_new, "(Class)s[(Arguments)Array;]"),
  GB_STATIC_METHOD("IsValid", "b", object_is_valid, "(Object)o"),
  GB_STATIC_METHOD("Lock", NULL, object_lock, "(Object)o"),
  GB_STATIC_METHOD("Unlock", NULL, object_unlock, "(Object)o"),
  GB_STATIC_METHOD("IsLocked", "b", object_is_locked, "(Object)o"),
  GB_STATIC_METHOD("Count", "i", object_count, "(Object)o"),

  GB_END_DECLARE
};



