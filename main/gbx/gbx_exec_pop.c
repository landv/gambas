/***************************************************************************

  gbx_exec_pop.c

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
#include "gb_limit.h"
#include "gbx_exec.h"

#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_c_array.h"
#include "gbx_c_collection.h"
#include "gbx_api.h"
#include "gbx_struct.h"


void EXEC_pop_unknown(void)
{
  static void *jump[] = {
    /* 0 */ &&_POP_GENERIC,
    /* 1 */ &&_POP_VARIABLE,
    /* 2 */ &&_POP_STATIC_VARIABLE,
    /* 3 */ &&_POP_PROPERTY,
    /* 4 */ &&_POP_VARIABLE_AUTO,
    /* 5 */ &&_POP_PROPERTY_AUTO,
		/* 6 */ &&_POP_STRUCT_FIELD
    };

  const char *name;
  int index;
  CLASS_DESC *desc;
  CLASS *class;
  OBJECT *object;
  char *addr;
  bool defined;
  VALUE *val;

  defined = EXEC_object(&SP[-1], &class, &object);

  /*printf("> exec_pop_unknown: SP = %p -> %p\n", SP, SP->_string.addr);*/

  goto *jump[*PC & 0xF];


_POP_GENERIC:

  name = CP->load->unknown[PC[1]];

	// The first time we access a symbol, we must not be virtual to find it
	val = &SP[-1];
	if (LIKELY(defined && object && !VALUE_is_super(val)))
  	index = CLASS_find_symbol(val->_object.class, name);
	else
  	index = CLASS_find_symbol(class, name);

  if (UNLIKELY(index == NO_SYMBOL))
  {
    if (class->special[SPEC_UNKNOWN] == NO_SYMBOL)
    {
			if (defined && object && !VALUE_is_super(val))
				class = val->_object.class;
			THROW(E_NSYMBOL, name, class->name);
		}
		goto _POP_UNKNOWN_PROPERTY;
	}

  desc = class->table[index].desc;

  switch (CLASS_DESC_get_type(desc))
  {
    case CD_CONSTANT:

      THROW(E_NPROPERTY, class->name, name);

    case CD_VARIABLE:

      if (object == NULL)
      {
        if (UNLIKELY(!class->auto_create))
          THROW(E_DYNAMIC, class->name, name);
        object = EXEC_auto_create(class, TRUE);
        *PC |= 4;
      }
      else
      {
        if (defined) *PC |= 1;
      }

      if (defined)
        PC[1] = index;

      goto _POP_VARIABLE_2;

    case CD_STRUCT_FIELD:

      if (object == NULL)
        THROW(E_DYNAMIC, class->name, name);
      
      if (defined) 
			{
				*PC |= 6;
        PC[1] = index;
			}
			
      goto _POP_STRUCT_FIELD_2;

    case CD_STATIC_VARIABLE:

      if (UNLIKELY(object != NULL))
        THROW(E_STATIC, class->name, name);

      if (defined) *PC |= 2;

      if (defined)
        PC[1] = index;

      goto _POP_STATIC_VARIABLE_2;

    case CD_PROPERTY:

      if (object == NULL)
      {
        if (UNLIKELY(!class->auto_create))
          THROW(E_DYNAMIC, class->name, name);
        object = EXEC_auto_create(class, TRUE);
        *PC |= 5;
      }
      else
      {
        if (defined) *PC |= 3;
      }

      if (defined)
        PC[1] = index;

      goto _POP_PROPERTY_2;

    case CD_STATIC_PROPERTY:

      if (UNLIKELY(object != NULL))
        THROW(E_STATIC, class->name, name);

      if (defined) *PC |= 3;

      if (defined)
        PC[1] = index;

      goto _POP_PROPERTY_2;

    case CD_PROPERTY_READ:
    case CD_STATIC_PROPERTY_READ:

      THROW(E_NWRITE, class->name, name);

    case CD_METHOD:
    case CD_STATIC_METHOD:

      THROW(E_NPROPERTY, class->name, name);

    default:

      THROW(E_NSYMBOL, name, class->name);
  }


_POP_VARIABLE_AUTO:

  object = EXEC_auto_create(class, TRUE);

_POP_VARIABLE:

  desc = class->table[PC[1]].desc;

_POP_VARIABLE_2:

  addr = (char *)object + desc->variable.offset;
  VALUE_write(&SP[-2], (void *)addr, desc->variable.type);
  goto _FIN;


_POP_STATIC_VARIABLE:

  desc = class->table[PC[1]].desc;

_POP_STATIC_VARIABLE_2:

  addr = (char *)desc->variable.class->stat + desc->variable.offset;
  VALUE_write(&SP[-2], (void *)addr, desc->variable.type);
  goto _FIN;


_POP_STRUCT_FIELD:

  desc = class->table[PC[1]].desc;

_POP_STRUCT_FIELD_2:

	if (((CSTRUCT *)object)->ref)
		addr = (char *)((CSTATICSTRUCT *)object)->addr + desc->variable.offset;
	else
		addr = (char *)object + sizeof(CSTRUCT) + desc->variable.offset;
	
  VALUE_write(&SP[-2], (void *)addr, desc->variable.type);
  goto _FIN;


_POP_PROPERTY_AUTO:

  object = EXEC_auto_create(class, TRUE);

_POP_PROPERTY:

  desc = class->table[PC[1]].desc;

_POP_PROPERTY_2:

  VALUE_conv(&SP[-2], desc->property.type);

  if (desc->property.native)
  {
    if (UNLIKELY(EXEC_call_native(desc->property.write, object, 0, &SP[-2])))
      PROPAGATE();
  }
  else
  {
    *SP = SP[-2];
		PUSH();

    EXEC.class = desc->property.class;
    EXEC.object = object;
    EXEC.nparam = 1;
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->property.write;

    EXEC_function();
  }
	
	goto _FIN;

_POP_UNKNOWN_PROPERTY:

	EXEC_unknown_property = TRUE;
	EXEC_unknown_name = CP->load->unknown[PC[1]];

	*SP = SP[-2];
	PUSH();

	EXEC_special(SPEC_UNKNOWN, class, object, 1, TRUE);

_FIN:

  RELEASE(&SP[-2]);
  OBJECT_UNREF(object, "EXEC_pop_unknown");
  SP -= 2;
  PC++;
}


void EXEC_pop_array(ushort code)
{
	static const void *jump[] = { &&__POP_GENERIC, &&__POP_QUICK_ARRAY, && __POP_QUICK_COLLECTION, &&__POP_ARRAY };
	
  CLASS *class;
  OBJECT *object;
  GET_NPARAM(np);
  int i;
  void *data;
  bool defined;
  VALUE *val;
  VALUE swap;
	int fast;

  val = &SP[-np];
	goto *jump[((unsigned char)code) >> 6];

__POP_GENERIC:

	defined = EXEC_object(val, &class, &object);
	
	// The first time we access a symbol, we must not be virtual to find it
	if (LIKELY(defined && object && !VALUE_is_super(val)))
	{
		//fprintf(stderr, "%s -> %s\n", class->name, val->_object.class->name);
  	class = val->_object.class;
  }
	
	fast = 3;
	
	if (LIKELY(defined))
	{
		if (class->quick_array == CQA_ARRAY)
			fast = 1;
		else if (class->quick_array == CQA_COLLECTION)
			fast = 2;
	}

	*PC |= fast << 6;
	
	goto __POP_ARRAY_2;

__POP_QUICK_ARRAY:

  EXEC_object_fast(val, &class, &object);

	TYPE type = ((CARRAY *)object)->type;
	
	VALUE_copy(&swap, &val[0]);
	VALUE_copy(&val[0], &val[-1]);
	VALUE_copy(&val[-1], &swap);
	
	VALUE_conv(&val[0], type);
	VALUE_conv_integer(&val[1]);
	
	if (LIKELY(np == 2))
	{
		data = CARRAY_get_data((CARRAY *)object, val[1]._integer.value);
	}
	else
	{
		for (i = 2; i < np; i++)
			VALUE_conv_integer(&val[i]);
		
		data = CARRAY_get_data_multi((CARRAY *)object, (GB_INTEGER *)&val[1], np - 1);
	}
	if (UNLIKELY(data == NULL))
		PROPAGATE();
	VALUE_write(val, data, type);
	
	SP = val + 1;
	RELEASE_MANY(SP, 2);
	//OBJECT_UNREF(object, "EXEC_push_array");
	return;
	
__POP_QUICK_COLLECTION:

  EXEC_object_fast(val, &class, &object);

	VALUE_conv_variant(&val[-1]);
	VALUE_conv_string(&val[1]);
	
	if (UNLIKELY(GB_CollectionSet((GB_COLLECTION)object, val[1]._string.addr + val[1]._string.start, val[1]._string.len, (GB_VARIANT *)&val[-1])))
		PROPAGATE();
	
	RELEASE_MANY(SP, 3);
	return;
	
__POP_ARRAY:
    
  defined = EXEC_object(val, &class, &object);

__POP_ARRAY_2:

	/* swap object and value to be inserted */

	//swap = val[0];
	//val[0] = val[-1];
	//val[-1] = swap;
	VALUE_copy(&swap, &val[0]);
	VALUE_copy(&val[0], &val[-1]);
	VALUE_copy(&val[-1], &swap);

	if (UNLIKELY(EXEC_special(SPEC_PUT, class, object, np, TRUE)))
		THROW(E_NARRAY, class->name);

	POP(); /* free the object */
}
