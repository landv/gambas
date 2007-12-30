/***************************************************************************

  value.c

  Datatype management routines. Conversions between each Gambas datatype,
  and conversions between Gambas datatypes and native datatypes.

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


#include <float.h>
#include <math.h>
#include <limits.h>

#include "gb_common.h"
#include "gb_common_case.h"

#include "gbx_math.h"
#include "gbx_type.h"

#include "gbx_array.h"
#include "gbx_string.h"
#include "gbx_number.h"
#include "gbx_object.h"
#include "gbx_variant.h"
#include "gbx_date.h"

#include "gbx_exec.h"
#include "gbx_local.h"
#include "gb_common_buffer.h"

#include "gbx_value.h"


/* This function must keep the datatype, as it is used for initializing local variables */

PUBLIC void VALUE_default(VALUE *value, TYPE type)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  value->type = type;

  if (TYPE_is_object(type))
    goto __OBJECT;
  else
    goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

  value->_integer.value = 0;
  return;

__LONG:

  value->_long.value = 0;
  return;

__SINGLE:
__FLOAT:
  value->_float.value = 0;
  return;

__STRING:
  value->_string.addr = NULL;
  value->_string.start = 0;
  value->_string.len = 0;
  return;

__VARIANT:
  value->_variant.vtype = T_NULL;
  return;

__DATE:
  value->_date.date = 0;
  value->_date.time = 0;
  return;

__VOID:
  return;

__OBJECT:
  value->_object.class = (CLASS *)type;
  value->_object.object = NULL;
  return;

__ARRAY:
__FUNCTION:
__CLASS:
__NULL:
  ERROR_panic("VALUE_default: Unknown default type");
}


PUBLIC void VALUE_convert(VALUE *value, TYPE type)
{
  static void *jump[16][16] =
  {
   /*             void       b          c          h          i          l          g          f          d          cs         s          v          array      func       class      n         */
   /* void   */ { &&__OK,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    },
   /* b      */ { &&__N,     &&__OK,    &&__b2c,   &&__b2h,   &&__TYPE,  &&__b2l,   &&__b2f,   &&__b2f,   &&__N,     &&__b2s,   &&__b2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* c      */ { &&__N,     &&__c2b,   &&__OK,    &&__c2h,   &&__TYPE,  &&__c2l,   &&__c2f,   &&__c2f,   &&__c2d,   &&__c2s,   &&__c2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* h      */ { &&__N,     &&__h2b,   &&__h2c,   &&__OK,    &&__TYPE,  &&__h2l,   &&__h2f,   &&__h2f,   &&__h2d,   &&__h2s,   &&__h2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* i      */ { &&__N,     &&__i2b,   &&__i2c,   &&__i2h,   &&__OK,    &&__i2l,   &&__i2f,   &&__i2f,   &&__i2d,   &&__i2s,   &&__i2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* l      */ { &&__N,     &&__l2b,   &&__l2c,   &&__l2h,   &&__l2i,   &&__OK,    &&__l2g,   &&__l2f,   &&__l2d,   &&__l2s,   &&__l2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* g      */ { &&__N,     &&__f2b,   &&__f2c,   &&__f2h,   &&__f2i,   &&__f2l,   &&__OK,    &&__TYPE,  &&__f2d,   &&__f2s,   &&__g2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* f      */ { &&__N,     &&__f2b,   &&__f2c,   &&__f2h,   &&__f2i,   &&__f2l,   &&__f2g,   &&__OK,    &&__f2d,   &&__f2s,   &&__f2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* d      */ { &&__N,     &&__d2b,   &&__d2c,   &&__d2h,   &&__d2i,   &&__d2l,   &&__d2f,   &&__d2f,   &&__OK,    &&__d2s,   &&__d2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__N,     },
   /* cs     */ { &&__N,     &&__s2b,   &&__s2c,   &&__s2h,   &&__s2i,   &&__s2l,   &&__s2f,   &&__s2f,   &&__s2d,   &&__OK,    &&__OK,    &&__s2v,   &&__N,     &&__N,     &&__N,     &&__N,     },
   /* s      */ { &&__N,     &&__s2b,   &&__s2c,   &&__s2h,   &&__s2i,   &&__s2l,   &&__s2f,   &&__s2f,   &&__s2d,   &&__OK,    &&__OK,    &&__s2v,   &&__N,     &&__N,     &&__N,     &&__N,     },
   /* v      */ { &&__N,     &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__OK,    &&__N,     &&__N,     &&__v2,    &&__v2,     },
   /* array  */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__OK,    &&__N,     &&__N,     &&__N,     },
   /* func   */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__OK,    &&__N,     &&__N,     },
   /* class  */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__2v,    &&__N,     &&__N,     &&__OK,    &&__N,     },
   /* null   */ { &&__N,     &&__n2b,   &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__n2d,   &&__n2s,   &&__n2s,   &&__2v,    &&__N,     &&__N,     &&__N,     &&__OK,    },
  };

  int len;
  char *addr;
  int index;
  CLASS *class;
  boolean test;

__CONV:

  if (type >> 4 || value->type >> 4)
    goto __OBJECT;
  else
    goto *jump[value->type][type];

__c2b:
__h2b:
__i2b:

  value->_integer.value = (value->_integer.value != 0) ? -1 : 0;
  goto __TYPE;

__l2b:

  value->_integer.value = (value->_long.value != 0) ? -1 : 0;
  goto __TYPE;

__f2b:

  value->_integer.value = (value->_float.value != 0) ? -1 : 0;
  goto __TYPE;

__d2b:
  value->_integer.value = (value->_date.date != 0 || value->_date.time != 0) ? -1 : 0;
  goto __TYPE;

__b2c:
__h2c:
__i2c:

  value->_integer.value = (unsigned char)value->_integer.value;
  goto __TYPE;

__l2c:

  value->_integer.value = (unsigned char)value->_long.value;
  goto __TYPE;

__f2c:

  value->_integer.value = (unsigned char)value->_float.value;
  goto __TYPE;

__b2h:
__c2h:
__i2h:

  value->_integer.value = (short)value->_integer.value;
  goto __TYPE;

__l2h:

  value->_integer.value = (short)value->_long.value;
  goto __TYPE;

__f2h:

  value->_integer.value = (short)value->_float.value;
  goto __TYPE;

__l2i:

  value->_integer.value = (int)value->_long.value;
  goto __TYPE;

__f2i:

  value->_integer.value = (int)value->_float.value;
  goto __TYPE;

__b2l:
__c2l:
__h2l:
__i2l:

  value->_long.value = (long long)value->_integer.value;
  goto __TYPE;

__f2l:

  value->_long.value = (long long)value->_float.value;
  goto __TYPE;

__l2g:

  value->_float.value = (float)value->_long.value;
  if (!finite(value->_float.value))
    THROW(E_OVERFLOW);
  goto __TYPE;

__f2g:

  value->_float.value = (float)value->_float.value;
  if (!finite(value->_float.value))
    THROW(E_OVERFLOW);
  goto __TYPE;

__b2f:
__c2f:
__h2f:
__i2f:

  value->_float.value = value->_integer.value;
  goto __TYPE;

__l2f:

  value->_float.value = value->_long.value;
  goto __TYPE;

__c2d:
__h2d:
__i2d:

  value->_date.date = Max(0, value->_integer.value);
  value->_date.time = 0;
  goto __TYPE;

__l2d:

  if (value->_long.value < 0)
    value->_date.date = 0;
  else if (value->_long.value > LONG_MAX)
    value->_date.date = LONG_MAX;
  else
    value->_date.date = (int)value->_long.value;

  value->_date.time = 0;
  goto __TYPE;

__f2d:

  index = (int)fix(value->_float.value);
  value->_date.time = (int)((value->_float.value - index) * 86400000.0 + 0.5);
  value->_date.date = index;
  goto __TYPE;

__d2c:
__d2h:
__d2i:

  value->_integer.value = value->_date.date;
  value->type = T_INTEGER;
  goto *jump[T_INTEGER][type];

__d2l:

  value->_long.value = value->_date.date;
  goto __TYPE;

__d2f:

  value->_float.value = (double)value->_date.date + (double)value->_date.time / 86400000.0;
  goto __TYPE;

__b2s:

  if (value->_boolean.value)
    STRING_char_value(value, 'T');
  else
    STRING_void_value(value);
  return;

__c2s:
__h2s:
__i2s:

  len = sprintf(COMMON_buffer, "%d", value->_integer.value);
  STRING_new_temp_value(value, COMMON_buffer, len);
  BORROW(value);
  return;

__l2s:

  len = sprintf(COMMON_buffer, "%lld", value->_long.value);
  STRING_new_temp_value(value, COMMON_buffer, len);
  BORROW(value);
  return;

__g2s:
__f2s:

  LOCAL_format_number(value->_float.value, LF_GENERAL_NUMBER, NULL, 0, &addr, &len, FALSE);
  STRING_new_temp_value(value, addr, len);
  BORROW(value);
  return;

__d2s:

  len = DATE_to_string(COMMON_buffer, value);
  STRING_new_temp_value(value, COMMON_buffer, len);
  BORROW(value);
  return;

__s2b:

  addr = value->_string.addr;
  value->_integer.value = ((addr != NULL) && (value->_string.len != 0)) ? -1 : 0;
  if (value->type == T_STRING)
    STRING_unref(&addr);
  goto __TYPE;

__s2c:
__s2h:
__s2i:

  addr = value->type == T_STRING ? value->_string.addr : NULL;

  if (NUMBER_from_string(NB_READ_INTEGER, value->_string.addr + value->_string.start, value->_string.len, value))
    goto __N;

  STRING_unref(&addr);
  goto *jump[T_INTEGER][type];

__s2l:

  addr = value->type == T_STRING ? value->_string.addr : NULL;

  if (NUMBER_from_string(NB_READ_LONG, value->_string.addr + value->_string.start, value->_string.len, value))
    goto __N;

  STRING_unref(&addr);
  return;

__s2f:

  addr = value->type == T_STRING ? value->_string.addr : NULL;

  if (NUMBER_from_string(NB_READ_FLOAT, value->_string.addr + value->_string.start, value->_string.len, value))
    goto __N;

  STRING_unref(&addr);
  return;

__s2d:

  addr = value->type == T_STRING ? value->_string.addr : NULL;

  if (DATE_from_string(value->_string.addr + value->_string.start, value->_string.len, value, FALSE))
    goto __N;

  STRING_unref(&addr);
  return;

__n2b:

  value->_integer.value = 0;
  goto __TYPE;

__n2d:

  DATE_void_value(value);
  return;

__n2s:

  STRING_void_value(value);
  return;

__v2:

  /* ATTENTION !
    VALUE_read sert normalement �lire un stockage en m�oire.
    On le d�ourne ici pour convertir le variant.
    Mais si le variant contient un T_STRING, � doit rester un T_STRING ! */

  index = value->_variant.vtype;

  if (index != T_NULL)
    VALUE_read(value, &value->_variant.value, value->_variant.vtype);
  value->type = index;

  goto __CONV;

__s2v:

  STRING_copy_from_value_temp(&addr, value);

  if (addr != value->_string.addr)
  {
    STRING_ref(addr);

    if (value->type == T_STRING)
      STRING_unref(&value->_string.addr);
  }

  *((char **)value->_variant.value) = addr;

  value->_variant.vtype = T_STRING;

  goto __TYPE;

__2v:

  /* VALUE_put ne fonctionne pas avec T_STRING ! */
  if (value->type != T_NULL)
    VALUE_put(value, &value->_variant.value, value->type);

  value->_variant.vtype = value->type;
  goto __TYPE;

__OBJECT:

  if (!TYPE_is_object(type))
  {
    if (type == T_BOOLEAN)
    {
      test = (value->_object.object != NULL);
      OBJECT_UNREF(&value->_object.object, "VALUE_convert");
      value->_boolean.value = test ? -1 : 0;
      goto __TYPE;
    }

    if (type == T_VARIANT)
      goto __2v;

    goto __N;
  }

  if (!TYPE_is_object(value->type))
  {
    if (value->type == T_NULL)
    {
      OBJECT_null(value, (CLASS *)type); /* marche aussi pour type = T_OBJECT */
      goto __TYPE;
    }

    if (value->type == T_VARIANT)
      goto __v2;

    if (value->type == T_CLASS)
    {
      class = value->_class.class;
      
			if (CLASS_is_virtual(class))
				THROW(E_VIRTUAL);
      
      CLASS_load(class);

      if (class->auto_create)
        value->_object.object = CLASS_auto_create(class, 0);
      else
        value->_object.object = class;

      OBJECT_REF(value->_object.object, "VALUE_convert");
      value->type = T_OBJECT;
      /* on continue... */
    }
    else
      goto __N;

  }

  if (value->type == T_OBJECT)
  {
    if (value->_object.object == NULL)
      goto __TYPE;

    class = OBJECT_class(value->_object.object);
    /* on continue */
  }
  else
    class = value->_object.class;

  if (CLASS_is_virtual(class))
    THROW(E_VIRTUAL);

  if (type == T_OBJECT)
    goto __TYPE;

__RETRY:

  if ((class == (CLASS *)type) || CLASS_inherits(class, (CLASS *)type))
    goto __TYPE;

  if (value->type != T_OBJECT && value->_object.object)
  {
    class = OBJECT_class(value->_object.object);
    value->type = T_OBJECT;
    goto __RETRY;
  }


  THROW(E_TYPE, TYPE_get_name(type), TYPE_get_name((TYPE)class));

__TYPE:

  value->type = type;

__OK:

  return;

__N:

  THROW(E_TYPE, TYPE_get_name(type), TYPE_get_name(value->type));

__NR:

  THROW(E_NRETURN);
}



PUBLIC void VALUE_write(VALUE *value, void *addr, TYPE type)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  char *str;

__CONV:

  if (TYPE_is_object(type))
    goto __OBJECT;
  else
    goto *jump[type];

__BOOLEAN:

  VALUE_conv(value, type);
  *((unsigned char *)addr) = (value->_boolean.value != 0 ? 255 : 0);
  return;

__BYTE:

  VALUE_conv(value, type);
  *((unsigned char *)addr) = (unsigned char)(value->_byte.value);
  return;

__SHORT:

  VALUE_conv(value, type);
  *((short *)addr) = (short)(value->_short.value);
  return;

__INTEGER:

  VALUE_conv(value, type);
  *((int *)addr) = value->_integer.value;
  return;

__LONG:

  VALUE_conv(value, type);
  *((long long *)addr) = value->_long.value;
  return;

__SINGLE:

  VALUE_conv(value, type);
  *((float *)addr) = (float)value->_float.value;
  return;

__FLOAT:

  VALUE_conv(value, type);
  *((double *)addr) = value->_float.value;
  return;

__DATE:

  VALUE_conv(value, type);
  ((int *)addr)[0] = value->_date.date;
  ((int *)addr)[1] = value->_date.time;
  return;

__STRING:

  /* Il faut faire l'affectation en deux temps au cas o
     value->_string.addr == *addr ! */

  VALUE_conv(value, type);

  STRING_copy_from_value_temp(&str, value);
  STRING_ref(str);
  STRING_unref((char **)addr);
  *((char **)addr) = str;
  return;

__OBJECT:

  VALUE_conv(value, type);

  OBJECT_REF(value->_object.object, "VALUE_write");
  OBJECT_UNREF(((void **)addr), "VALUE_write");
  *((void **)addr) = value->_object.object;
  return;

__CLASS:

  VALUE_conv(value, type);

  OBJECT_REF(value->_class.class, "VALUE_write");
  OBJECT_UNREF(((void **)addr), "VALUE_write");
  *((void **)addr) = value->_class.class;
  return;

__VARIANT:

  VARIANT_undo(value);

  type = value->type;
  if (type == T_CSTRING)
    type = T_STRING;

  VARIANT_clear((VARIANT *)addr);

  ((VARIANT *)addr)->type = type;

  /* Et si type ne fait pas partie des types valides pour cette fonction ?? */
  if (type == T_NULL)
    return;

  addr = ((VARIANT *)addr)->value;
  /*goto *jump[Min(T_OBJECT, type)];*/
  goto __CONV;

  /*
  {
    VARIANT *var = ((VARIANT *)addr);

    VARIANT_free(var);

    var->type = value->_variant.vtype;
    VARIANT_copy(value->_variant.value, var->value);

    if (TYPE_is_object(var->type))
      OBJECT_$ref(*((void **)var->value));

    return;
  }
  */

__VOID:
__ARRAY:
__FUNCTION:
__NULL:

  ERROR_panic("Bad type (%d) for VALUE_write", type);
}



PUBLIC void VALUE_read(VALUE *value, void *addr, TYPE type)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__CSTRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  char *str;

  value->type = type;

  if (TYPE_is_object(type))
    goto __OBJECT;
  else
    goto *jump[type];

__BOOLEAN:

  value->_boolean.value = (*((unsigned char *)addr) != 0) ? (-1) : 0;
  return;

__BYTE:

  value->_byte.value = *((unsigned char *)addr);
  return;

__SHORT:

  value->_short.value = *((short *)addr);
  return;

__INTEGER:

  value->_integer.value = *((int *)addr);
  return;

__LONG:

  value->_long.value = *((long long *)addr);
  return;

__SINGLE:

  value->_single.value = *((float *)addr);
  return;

__FLOAT:

  value->_float.value = *((double *)addr);
  return;

__DATE:

  value->_date.date = ((int *)addr)[0];
  value->_date.time = ((int *)addr)[1];
  return;

__STRING:

  str = *((char **)addr);

  value->type = T_STRING;
  value->_string.addr = str;
  value->_string.start = 0;
  /*value->_string.len = (str == NULL) ? 0 : strlen(str);*/
  value->_string.len = STRING_length(str);

  return;

__CSTRING:

  str = *((char **)addr);

  value->type = T_CSTRING;
  value->_string.addr = str;
  value->_string.start = 0;
  value->_string.len = (str == NULL) ? 0 : strlen(str);
  /*value->_string.len = STRING_length(str);*/

  return;

__OBJECT:

  value->_object.object = *((void **)addr);
  return;

__VARIANT:

  value->_variant.type = T_VARIANT;
  value->_variant.vtype = ((VARIANT *)addr)->type;

  /*if (value->_variant.vtype == T_STRING)
    value->_variant.vtype = T_STRING;
  else*/ if (value->_variant.vtype == T_VOID)
    value->_variant.vtype = T_NULL;

  VARIANT_copy(((VARIANT *)addr)->value, value->_variant.value);

  return;

__CLASS:

	value->_class.class = *((void **)addr);
	value->_class.super = NULL;
	return;

__VOID:
__ARRAY:
__FUNCTION:
__NULL:

  ERROR_panic("Bad type (%d) for VALUE_read", type);
}


/* Retourne TRUE s'il y a eu besoin d'allouer qqch */

/*
#define MAX_FREE_PTR 64

static int free_count = 0;
static void *free_ptr[MAX_FREE_PTR];


static void add_free_ptr(void *ptr)
{
  if (free_count >= MAX_FREE_PTR)
    ERROR_panic("VALUE_put: too many pointers to free");

  free_ptr[free_count] = ptr;
  free_count++;
}

PUBLIC void VALUE_put_free(void)
{
  int i;

  for (i = 0; i < free_count; i++)
    STRING_free((char **)&free_ptr[i]);

  free_count = 0;
}

PUBLIC void VALUE_put_forget(void)
{
  free_count = 0;
}
*/

PUBLIC void VALUE_put(VALUE *value, void *addr, TYPE type)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  VALUE_conv(value, type);

  if (TYPE_is_object(type))
    goto __OBJECT;
  else
    goto *jump[type];

__BOOLEAN:

  *((unsigned char *)addr) = (value->_boolean.value != 0 ? 255 : 0);
  return;

__BYTE:

  *((unsigned char *)addr) = (unsigned char)(value->_byte.value);
  return;

__SHORT:

  *((short *)addr) = (short)(value->_short.value);
  return;

__INTEGER:

  *((int *)addr) = value->_integer.value;
  return;

__LONG:

  *((long long *)addr) = value->_long.value;
  return;

__SINGLE:

  *((float *)addr) = (float)value->_float.value;
  return;

__FLOAT:

  *((double *)addr) = value->_float.value;
  return;

__DATE:

  /* Inverser au cas o value ~= addr */

  ((int *)addr)[1] = value->_date.time;
  ((int *)addr)[0] = value->_date.date;
  return;

__STRING:

  ((int *)addr)[0] = (int)(value->_string.addr + value->_string.start);
  ((int *)addr)[1] = value->_string.len;
  return;

__OBJECT:

  *((void **)addr) = value->_object.object;
  return;

__VARIANT:

  *((VARIANT *)addr) = *((VARIANT *)&value->_variant.vtype);
  return;

  /*
  if
  type = value->type;

  ((VARIANT *)addr)->type = type;
  addr = ((VARIANT *)addr)->value;
  goto __CONV;
  */

__CLASS:

  *((void **)addr) = value->_class.class;
  return;

__VOID:
__ARRAY:
__FUNCTION:
__NULL:

  ERROR_panic("Bad type (%d) for VALUE_put", type);
}


PUBLIC void VALUE_free(void *addr, TYPE type)
{
  if (type == T_STRING)
  {
    STRING_unref((char **)addr);
    *((char **)addr) = NULL;
  }
  else if (TYPE_is_object(type))
  {
    OBJECT_unref((void **)addr);
    *((void **)addr) = NULL;
  }
  else if (type == T_VARIANT)
  {
    VARIANT_free((VARIANT *)addr);
    ((VARIANT *)addr)->type = T_NULL;
  }
}



PUBLIC void VALUE_to_string(VALUE *value, char **addr, int *len)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

__CONV:

  if (TYPE_is_object(value->type))
    goto __OBJECT;
  else
    goto *jump[value->type];

__NULL:

  *addr = ""; /* Pour �re coh�ent avec print "" puisque Null == "" */
  *len = 0;
  return;

__BOOLEAN:

  if (value->_boolean.value)
  {
    *addr = "True";
    *len = 4;
  }
  else
  {
    *addr = "False";
    *len = 5;
  }
  return;

__BYTE:
__SHORT:
__INTEGER:

  *len = sprintf(COMMON_buffer, "%d", value->_integer.value);
  *addr = COMMON_buffer;

  return;

__LONG:

  *len = sprintf(COMMON_buffer, "%lld", value->_long.value);
  *addr = COMMON_buffer;

  return;

__DATE:

  LOCAL_format_date(DATE_split(value), LF_STANDARD, NULL, 0, addr, len);
  return;

__SINGLE:
__FLOAT:

  LOCAL_format_number(value->_float.value, LF_STANDARD, NULL, 0, addr, len, TRUE);
  return;

__STRING:

  *len = value->_string.len;
  *addr = value->_string.addr + value->_string.start;
  return;

__OBJECT:

  if (VALUE_is_null(value))
    goto __NULL;

  *len = sprintf(COMMON_buffer, "(%s %p)", OBJECT_class(value->_object.object)->name, value->_object.object);
  *addr = COMMON_buffer;
  return;

__VARIANT:

  VARIANT_undo(value);
  goto __CONV;

__VOID:

  THROW(E_NRETURN);

__CLASS:

  *len = sprintf(COMMON_buffer, "(Class %s)", value->_class.class->name);
  *addr = COMMON_buffer;
  return;

__ARRAY:
__FUNCTION:

  *len = sprintf(COMMON_buffer, "(%s ?)", TYPE_get_name(value->type));
  *addr = COMMON_buffer;

  /*THROW(E_TYPE, TYPE_get_name(T_STRING), TYPE_get_name(value->type));*/
}


PUBLIC void VALUE_from_string(VALUE *value, const char *addr, int len)
{
	value->type = T_NULL;

	if (!len || *addr == 0)
		return;

  if (!DATE_from_string(addr, len, value, TRUE))
  	return;

  if (!NUMBER_from_string(NB_READ_ALL | NB_READ_HEX_BIN | NB_LOCAL, addr, len, value))
  	return;

	if (len == 4 && strncasecmp(addr, "true", len) == 0)
	{
		value->type = T_BOOLEAN;
		value->_boolean.value = -1;
		return;
	}

	if (len == 5 && strncasecmp(addr, "false", len) == 0)
	{
		value->type = T_BOOLEAN;
		value->_boolean.value = 0;
		return;
	}
}


PUBLIC void VALUE_class_read(CLASS *class, VALUE *value, char *addr, CTYPE ctype)
{
  if (ctype.id == T_OBJECT)
  {
    VALUE_read(value, addr, (ctype.value >= 0) ? (TYPE)class->load->class_ref[ctype.value] : T_OBJECT);
  }
  else if (ctype.id == T_ARRAY)
  {
    value->type = T_ARRAY;
    value->_array.desc = class->load->array[ctype.value];
    value->_array.addr = addr;
    value->_array.keep = TRUE; // We suppose that the array goes to the stack
  }
  else
    VALUE_read(value, addr, (TYPE)ctype.id);
}


PUBLIC void VALUE_class_write(CLASS *class, VALUE *value, char *addr, CTYPE ctype)
{
  if (ctype.id == T_OBJECT)
  {
    VALUE_write(value, addr, (ctype.value >= 0) ? (TYPE)class->load->class_ref[ctype.value] : T_OBJECT);
  }
  else if (ctype.id == T_ARRAY)
  {
    THROW(E_UNKNOWN);
  }
  else
    VALUE_write(value, addr, (TYPE)ctype.id);
}


PUBLIC void VALUE_class_default(CLASS *class, VALUE *value, CTYPE ctype)
{
  if (ctype.id == T_OBJECT)
  {
    VALUE_default(value, (ctype.value >= 0) ? (TYPE)class->load->class_ref[ctype.value] : T_OBJECT);
  }
  else if (ctype.id == T_ARRAY)
  {
    value->type = T_ARRAY;
    value->_array.desc = class->load->array[ctype.value];
    value->_array.keep = FALSE;
    ARRAY_new(&value->_array.addr, (ARRAY_DESC *)value->_array.desc);
  }
  else
    VALUE_default(value, (TYPE)ctype.id);
}


PUBLIC void VALUE_class_constant(CLASS *class, VALUE *value, int ind)
{
  static void *jump[] =
  {
    &&__ILLEGAL, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT,
    &&__ILLEGAL, &&__STRING, &&__CSTRING, &&__ILLEGAL, &&__ILLEGAL, &&__ILLEGAL, &&__ILLEGAL, &&__ILLEGAL
  };

  CLASS_CONST *cc;

  if (ind < 0)
    goto __ILLEGAL;

  cc = &class->load->cst[ind];

  value->type = cc->type;
  goto *jump[cc->type];

__INTEGER:

  value->_integer.value = cc->_integer.value;
  return;

__LONG:

  value->_long.value = cc->_long.value;
  return;

__FLOAT:

  value->_float.value = cc->_float.value;
  return;

__STRING:

  value->type = T_CSTRING;
  value->_string.addr = (char *)cc->_string.addr;
  value->_string.start = 0;
  value->_string.len = cc->_string.len;
  return;

__CSTRING:

  value->type = T_CSTRING;
  value->_string.addr = (char *)LOCAL_gettext(cc->_string.addr);
  value->_string.start = 0;
  value->_string.len = strlen(value->_string.addr);
  return;

__ILLEGAL:

  EXEC_ILLEGAL();
}


PUBLIC bool VALUE_is_null(VALUE *val)
{
  if (val->type == T_NULL)
    return TRUE;

  if (VALUE_is_string(val) && (val->_string.addr == 0 || val->_string.len == 0))
    return TRUE;

  if (VALUE_is_object(val) && (val->_object.object == NULL))
    return TRUE;

  if (val->type == T_DATE && val->_date.date == 0 && val->_date.time == 0)
    return TRUE;

  if (val->type == T_VARIANT)
  {
    if (val->_variant.vtype == T_NULL)
      return TRUE;

    if (val->_variant.vtype == T_STRING && *((char **)val->_variant.value) == NULL)
      return TRUE;

    if (val->_variant.vtype == T_DATE
        && ((DATE *)val->_variant.value)->date == 0
        && ((DATE *)val->_variant.value)->time == 0)
      return TRUE;

    if (TYPE_is_object(val->_variant.vtype) && *((void **)val->_variant.value) == NULL)
      return TRUE;
  }

  return FALSE;
}


PUBLIC void VALUE_get_string(VALUE *val, char **text, int *length)
{
  if (VALUE_is_null(val))
  {
    *text = 0;
    *length = 0;
  }
  else
  {
    *length = val->_string.len;
    if (*length)
      *text = val->_string.start + val->_string.addr;
    else
      *text = NULL;
  }
}

