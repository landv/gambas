/***************************************************************************

  gbx_exec_pop.c

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
	if (defined && object && !VALUE_is_super(val))
  	index = CLASS_find_symbol(val->_object.class, name);
	else
  	index = CLASS_find_symbol(class, name);

  if (index == NO_SYMBOL)
  {
    if (class->special[SPEC_UNKNOWN] == NO_SYMBOL)
    {
			if (defined && object && !VALUE_is_super(val))
				class = val->_object.class;
			THROW(E_NSYMBOL, CLASS_get_name(class), name);
		}
		goto _POP_UNKNOWN_PROPERTY;
	}

  desc = class->table[index].desc;

  switch (CLASS_DESC_get_type(desc))
  {
    case CD_CONSTANT:

      THROW(E_NPROPERTY, CLASS_get_name(class), name);

    case CD_VARIABLE:

      if (object == NULL)
      {
        if (!class->auto_create)
          THROW(E_DYNAMIC, CLASS_get_name(class), name);
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
        THROW(E_DYNAMIC, CLASS_get_name(class), name);
      
      if (defined) 
			{
				*PC |= 6;
        PC[1] = index;
			}
			
			if (desc->variable.ctype.id == TC_STRUCT || desc->variable.ctype.id == TC_ARRAY)
				THROW(E_NWRITE, CLASS_get_name(class), name);
			
      goto _POP_STRUCT_FIELD_2;

    case CD_STATIC_VARIABLE:

      if (object)
        THROW(E_STATIC, CLASS_get_name(class), name);

      if (defined) *PC |= 2;

      if (defined)
        PC[1] = index;

      goto _POP_STATIC_VARIABLE_2;

    case CD_PROPERTY:

      if (object == NULL)
      {
        if (!class->auto_create)
          THROW(E_DYNAMIC, CLASS_get_name(class), name);
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

      if (object)
        THROW(E_STATIC, CLASS_get_name(class), name);

      if (defined) *PC |= 3;

      if (defined)
        PC[1] = index;

      goto _POP_PROPERTY_2;

    case CD_PROPERTY_READ:
    case CD_STATIC_PROPERTY_READ:

      THROW(E_NWRITE, CLASS_get_name(class), name);

    case CD_METHOD:
    case CD_STATIC_METHOD:

      THROW(E_NPROPERTY, CLASS_get_name(class), name);

    default:

      THROW(E_NSYMBOL, CLASS_get_name(class), name);
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
    if (EXEC_call_native(desc->property.write, object, 0, &SP[-2]))
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

	if (class->special[SPEC_PROPERTY] == NO_SYMBOL)
		goto _NOT_A_PROPERTY;
	
	EXEC_unknown_name = name;
	if (EXEC_special(SPEC_PROPERTY, class, class->property_static ? NULL : object, 0, FALSE))
		goto _NOT_A_PROPERTY;

	VALUE_conv_boolean(&SP[-1]);
	SP--;
	if (!SP->_boolean.value)
		goto _NOT_A_PROPERTY;
		
	EXEC_unknown_name = name;

	*SP = SP[-2];
	PUSH();

	EXEC_special(SPEC_UNKNOWN, class, class->unknown_static ? NULL : object, 1, TRUE);
	goto _FIN;
	
_NOT_A_PROPERTY:

	THROW(E_NPROPERTY, CLASS_get_name(class), name);

_FIN:

  RELEASE(&SP[-2]);
  OBJECT_UNREF(object);
  SP -= 2;
  PC++;
}

void EXEC_pop_array(ushort code)
{
  CLASS *class;
  OBJECT *object;
  GET_NPARAM(np);
  VALUE *val;
  VALUE swap;

  val = &SP[-np];
    
  EXEC_object(val, &class, &object);

	/* swap object and value to be inserted */
	VALUE_copy(&swap, &val[0]);
	VALUE_copy(&val[0], &val[-1]);
	VALUE_copy(&val[-1], &swap);

	if (EXEC_special(SPEC_PUT, class, object, np, TRUE))
		THROW(E_NARRAY, CLASS_get_name(class));

	POP(); /* free the object */
}
