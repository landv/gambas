/***************************************************************************

  exec_push.c

  Instructions that push symbols, special functions and arrays on the
  stack.

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

#include "gb_common.h"
#include "gb_limit.h"
#include "gbx_exec.h"
#include "gb_pcode.h"
#include "gbx_api.h"

#include "gbx_string.h"
#include "gbx_array.h"
#include "gbx_c_collection.h"


void EXEC_push_unknown(ushort code)
{
  static void *jump[] = {
    /* 0 */ &&_PUSH_GENERIC,
    /* 1 */ &&_PUSH_CONSTANT,
    /* 2 */ &&_PUSH_VARIABLE,
    /* 3 */ &&_PUSH_STATIC_VARIABLE,
    /* 4 */ &&_PUSH_PROPERTY,
    /* 5 */ &&_PUSH_METHOD,
    /* 6 */ &&_PUSH_STATIC_METHOD,
    /* 7 */ &&_PUSH_VARIABLE_AUTO,
    /* 8 */ &&_PUSH_PROPERTY_AUTO,
    /* 9 */ &&_PUSH_METHOD_AUTO,
    /* 10 */ &&_PUSH_EXTERN
    };

  const char *name;
  int index;
  CLASS_DESC *desc;
  CLASS *class;
  OBJECT *object;
  char *addr;
  bool defined;
  VALUE *val;

  EXEC_object(&SP[-1], &class, &object, &defined);

  goto *jump[code & 0xF];


_PUSH_GENERIC:

  name = CP->load->unknown[PC[1]];

	// The first time we access a symbol, we must not be virtual to find it
	val = &SP[-1];
	
	if (defined && object && !VALUE_is_super(val))
  	index = CLASS_find_symbol(val->_object.class, name);
	else
  	index = CLASS_find_symbol(class, name);

  if (index == NO_SYMBOL)
  {
    //index = CLASS_find_symbol(class, name);

    if (class->special[SPEC_UNKNOWN] == NO_SYMBOL)
    {
			if (defined && object && !VALUE_is_super(val))
				class = val->_object.class;
      THROW(E_NSYMBOL, name, class->name);
		}

    goto _PUSH_UNKNOWN_METHOD;
  }
  
  desc = class->table[index].desc;

  switch (CLASS_DESC_get_type(desc))
  {
    case CD_CONSTANT:

      if (defined)
      {
				if ((PC[-1] & 0xF800) == C_PUSH_CLASS)
				{
					if (desc->constant.type == T_BOOLEAN)
					{
						PC[-1] = C_PUSH_MISC | (desc->constant.value._integer ? CPM_TRUE : CPM_FALSE);
						PC[0] = C_NOP;
						PC[1] = C_NOP;
						goto _PUSH_CONSTANT_2;
					}
					else if (desc->constant.type == T_BYTE || desc->constant.type == T_SHORT)
					{
						PC[-1] = C_PUSH_INTEGER;
						PC[0] = desc->constant.value._integer;
						PC[1] = (CODE_CONV << 8) | desc->constant.type;
						goto _PUSH_CONSTANT_2;
					}
					else if (desc->constant.type == T_INTEGER)
					{
						PC[-1] = C_PUSH_LONG;
						PC[0] = desc->constant.value._integer & 0xFFFF;
						PC[1] = desc->constant.value._integer >> 16;
						goto _PUSH_CONSTANT_2;
					}
				}
				
				*PC |= 1;
				PC[1] = index;
      }

      goto _PUSH_CONSTANT_2;

    case CD_VARIABLE:

      if (object == NULL)
      {
        if (!class->auto_create)
          THROW(E_DYNAMIC, class->name, name);
        object = EXEC_auto_create(class, TRUE);
        *PC |= 7;
      }
      else
      {
        if (defined) *PC |= 2;
      }

      if (defined)
        PC[1] = index;

      goto _PUSH_VARIABLE_2;

    case CD_STATIC_VARIABLE:

      if (object != NULL)
        THROW(E_STATIC, class->name, name);

      if (defined) *PC |= 3;

      if (defined)
        PC[1] = index;

      goto _PUSH_STATIC_VARIABLE_2;

    case CD_PROPERTY:
    case CD_PROPERTY_READ:

      if (object == NULL)
      {
        if (!class->auto_create)
          THROW(E_DYNAMIC, class->name, name);
        object = EXEC_auto_create(class, TRUE);
        *PC |= 8;
      }
      else
      {
        if (defined) *PC |= 4;
      }

      if (defined)
        PC[1] = index;

      goto _PUSH_PROPERTY_2;

    case CD_STATIC_PROPERTY:
    case CD_STATIC_PROPERTY_READ:

      if (object != NULL)
        THROW(E_STATIC, class->name, name);

      if (defined) *PC |= 4;

      if (defined)
        PC[1] = index;

      goto _PUSH_PROPERTY_2;

    case CD_METHOD:

      if (object == NULL)
      {
        if (!class->auto_create)
          THROW(E_DYNAMIC, class->name, name);
        object = EXEC_auto_create(class, TRUE);
        *PC |= 9;
      }
      else
      {
        if (defined) *PC |= 5;
      }

      if (defined)
        PC[1] = index;

      goto _PUSH_METHOD_2;

    case CD_STATIC_METHOD:

      if (object)
      {
        OBJECT_UNREF(object, "EXEC_push_unknown");
        object = NULL;
      }

      if (defined) *PC |= 6;

      if (defined)
        PC[1] = index;

      goto _PUSH_METHOD_2;

    case CD_EXTERN:

      if (object)
      {
        OBJECT_UNREF(object, "EXEC_push_unknown");
        object = NULL;
      }

      if (defined) *PC |= 10;

      if (defined)
        PC[1] = index;

      goto _PUSH_EXTERN_2;

    default:

      THROW(E_NSYMBOL, name, class->name);
  }


_PUSH_CONSTANT:

  desc = class->table[PC[1]].desc;

_PUSH_CONSTANT_2:

  VALUE_read(&SP[-1], (void *)&desc->constant.value, desc->constant.type);
  goto _FIN_DEFINED;


_PUSH_VARIABLE_AUTO:

  object = EXEC_auto_create(class, TRUE);

_PUSH_VARIABLE:

  desc = class->table[PC[1]].desc;

_PUSH_VARIABLE_2:

  addr = (char *)object + desc->variable.offset;
  goto _READ_PROPERTY;


_PUSH_STATIC_VARIABLE:

  desc = class->table[PC[1]].desc;

_PUSH_STATIC_VARIABLE_2:

  addr = (char *)desc->variable.class->stat + desc->variable.offset;
  goto _READ_PROPERTY;


_READ_PROPERTY:

  VALUE_read(&SP[-1], (void *)addr, desc->property.type);
  goto _FIN_DEFINED;


_PUSH_PROPERTY_AUTO:

  object = EXEC_auto_create(class, TRUE);

_PUSH_PROPERTY:

  desc = class->table[PC[1]].desc;

_PUSH_PROPERTY_2:

  if (desc->property.native)
  {
    if (EXEC_call_native(desc->property.read, object, desc->property.type, NULL))
      PROPAGATE();

    SP[-1] = TEMP;
    goto _FIN_DEFINED;
  }
  else
  {
    EXEC.class = desc->property.class;
    EXEC.object = object;
    EXEC.drop = FALSE;
    EXEC.nparam = 0;
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->property.read;

    EXEC_function_keep();

    SP[-1] = *RP;
    RP->type = T_VOID;
    goto _FIN_DEFINED_NO_BORROW;
  }


_PUSH_STATIC_METHOD:

  if (object)
  {
    OBJECT_UNREF(object, "EXEC_push_unknown");
    object = NULL;
  }

  goto _PUSH_METHOD;

_PUSH_METHOD_AUTO:

  object = EXEC_auto_create(class, TRUE);

_PUSH_METHOD:

  index = PC[1];
  desc = class->table[index].desc;

_PUSH_METHOD_2:

  //printf("PUSH_METHOD: %d %s\n", index, desc->method.name);

  SP--;
  SP->type = T_FUNCTION;
  SP->_function.class = class;
  SP->_function.object = object;
  /*SP->_function.function = (int)&desc->method;*/

  if (FUNCTION_is_native(&desc->method))
    SP->_function.kind = FUNCTION_NATIVE;
  else
    SP->_function.kind =  FUNCTION_PUBLIC;

  SP->_function.index = index;

  SP->_function.defined = defined;

  SP++;

  goto _FIN;


_PUSH_EXTERN:

  if (object)
  {
    OBJECT_UNREF(object, "EXEC_push_unknown");
    object = NULL;
  }

  index = PC[1];
  desc = class->table[index].desc;

_PUSH_EXTERN_2:

  //printf("PUSH_METHOD: %d %s\n", index, desc->method.name);

  SP--;
  SP->type = T_FUNCTION;
  SP->_function.class = class;
  SP->_function.object = object;
  SP->_function.kind = FUNCTION_EXTERN;
  SP->_function.index = (int)desc->ext.exec;
  SP->_function.defined = defined;
  SP++;

  goto _FIN;

_PUSH_UNKNOWN_METHOD:

  SP--;
  SP->type = T_FUNCTION;
  SP->_function.class = class;
  SP->_function.object = object;
  SP->_function.kind = FUNCTION_UNKNOWN;
  SP->_function.index = PC[1];
  SP->_function.defined = FALSE;
  SP++;

  //OBJECT_REF(&object, "EXEC_push_unknown: UNKNOWN");

  goto _FIN;


_FIN_DEFINED:

  BORROW(&SP[-1]);

_FIN_DEFINED_NO_BORROW:

  if (!defined)
    VALUE_conv(&SP[-1], T_VARIANT);

  /* sp[-1] contenait l'objet et a ���ras� Il faut donc le d���encer
     nous-m�e. Sauf si c'est un appel de m�hode statique (cf. plus haut) */

  OBJECT_UNREF(object, "EXEC_push_unknown");

_FIN:

  PC++;
}

void EXEC_push_array(void)
{
  CLASS *class;
  OBJECT *object;
  GET_NPARAM(np);
  int dim[MAX_ARRAY_DIM];
  int i;
  void *data;
  boolean defined;
  VALUE *val;
  ARRAY_DESC *desc;

  val = &SP[-np];
  np--;


  if (val->type == T_ARRAY)
  {
    for (i = 1; i <= np; i++)
    {
      VALUE_conv(&val[i], T_INTEGER);
      dim[i - 1] = val[i]._integer.value;
    }

    SP -= np + 1;

		desc = (ARRAY_DESC *)SP->_array.class->load->array[SP->_array.index];
    data = ARRAY_get_address(desc, SP->_array.addr, np, dim);

    VALUE_read(SP, data, CLASS_ctype_to_type(SP->_array.class, desc->type));

    PUSH();
  }
  else
  {
    EXEC_object(val, &class, &object, &defined);
    /*RELEASE(SP);*/

    /* Ex�ution de la m�hode sp�iale _get */

    if (EXEC_special(SPEC_GET, class, object, np, FALSE))
      THROW(E_NARRAY, class->name);

    OBJECT_UNREF(object, "EXEC_push_array");
    SP--;
    SP[-1] = SP[0];

    if (!defined)
    	VALUE_conv(&SP[-1], T_VARIANT);
  }
}
