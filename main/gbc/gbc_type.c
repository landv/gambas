/***************************************************************************

  gbc_type.c

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

#define __GBC_TYPE_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gbc_type.h"

#include "gbc_class.h"
#include "gbc_compile.h"


const char *TYPE_name[] = 
{
  "Void", "Boolean", "Byte", "Short", "Integer", "Long", "Single", "Float", "Date",
  "String", "CString", "Variant", "Array", "Pointer", "Class", "Null",
  "Object"
};


TYPE TYPE_make(TYPE_ID id, short value, int flag)
{
  TYPE type;

  TYPE_clear(&type);
  TYPE_set_id(&type, id);

  if (!(id == T_OBJECT || id == T_ARRAY || id == T_STRUCT))
		value = -1;
  
	TYPE_set_value(&type, value);
  TYPE_set_flag(&type, flag);

  return type;
}


size_t TYPE_sizeof(TYPE type)
{
  TYPE_ID id = TYPE_get_id(type);

  switch(id)
  {
    case T_BOOLEAN:
      return 1;

    case T_BYTE:
      return 1;

    case T_SHORT:
      return 2;

    case T_INTEGER:
      return 4;

    case T_LONG:
      return 8;

    case T_SINGLE:
      return 4;

    case T_FLOAT:
      return 8;

    case T_DATE:
      return 8;

    case T_STRING:
      return 4;

    case T_VARIANT:
      return 12;

    case T_OBJECT:
      return 4;
      
    case T_POINTER:
    	return 4;

    case T_ARRAY:
      {
        int i;
        size_t size;
        CLASS_ARRAY *array = &JOB->class->array[TYPE_get_value(type)];

        size = 1;
        for (i = 0; i < array->ndim; i++)
          size *= array->dim[i];

        size *= TYPE_sizeof(array->type);

        return (size + 3) & ~3;
      }

    default:
      ERROR_panic("TYPE_sizeof: bad type id");
  }
}



char *TYPE_get_desc(TYPE type)
{
  static char buf[256];

  TYPE_ID id;
  int value;

  id = TYPE_get_id(type);
  value = TYPE_get_value(type);

  if (id == T_ARRAY)
  {
    strcpy(buf, TYPE_name[JOB->class->array[value].type.t.id]);
    strcat(buf, "[]");
  }
  else
  {
    strcpy(buf, TYPE_name[id]);
  }

  return buf;
}


const char *TYPE_get_short_desc(TYPE type)
{
  static const char *name[] = {
    "",  "b", "i", "i", "i", "l", "g", "f", 
    "d", "s", "s", "p", "v", "?", "?", "?", 
    "o"
    };

  TYPE_ID id;

  id = TYPE_get_id(type);

  if (id == T_ARRAY)
    return "?";
  else
    return name[id];
}
