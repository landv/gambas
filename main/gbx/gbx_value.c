/***************************************************************************

  gbx_value.c

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

#define __GBX_VALUE_C

#include "gb_common.h"
#include "gb_common_case.h"

#include "gbx_math.h"
#include "gbx_type.h"

#include "gbx_c_array.h"
#include "gbx_string.h"
#include "gbx_number.h"
#include "gbx_object.h"
#include "gbx_variant.h"
#include "gbx_date.h"
#include "gbx_struct.h"
#include "gbx_exec.h"
#include "gbx_local.h"
#include "gb_common_buffer.h"
#include "gbx_extern.h"

#include "gbx_value.h"

#if 0
static bool unknown_function(VALUE *value)
{
	if (value->_function.kind == FUNCTION_UNKNOWN)
	{
		EXEC_unknown_property = TRUE;
		EXEC_unknown_name = CP->load->unknown[value->_function.index];

		EXEC_special(SPEC_UNKNOWN, value->_function.class, value->_function.object, 0, FALSE);

		//object = value->_function.object;
		OBJECT_UNREF(value->_function.object);

		SP--;
		//*val = *SP;
		COPY_VALUE(value, SP);
		return TRUE;
	}
	else
		return FALSE;
}
#endif

void THROW_TYPE_INTEGER(TYPE type)
{
	THROW(E_TYPE, TYPE_get_name(T_INTEGER), TYPE_get_name(type));
}

void THROW_TYPE_FLOAT(TYPE type)
{
	THROW(E_TYPE, TYPE_get_name(T_FLOAT), TYPE_get_name(type));
}

void THROW_TYPE_STRING(TYPE type)
{
	THROW(E_TYPE, TYPE_get_name(T_STRING), TYPE_get_name(type));
}


static void VALUE_put(VALUE *value, void *addr, TYPE type)
{
	static void *jump[16] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
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

	*((int64_t *)addr) = value->_long.value;
	return;

__SINGLE:

	*((float *)addr) = value->_single.value;
	return;

__FLOAT:

	*((double *)addr) = value->_float.value;
	return;

__DATE:

	/* Inverted, if value ~= addr */

	((int *)addr)[1] = value->_date.time;
	((int *)addr)[0] = value->_date.date;
	return;

/*__STRING:

	((int *)addr)[0] = (int)(value->_string.addr + value->_string.start);
	((int *)addr)[1] = value->_string.len;
	return;*/

__POINTER:

	*((void **)addr) = value->_pointer.value;
	return;

__OBJECT:

	*((void **)addr) = value->_object.object;
	return;

__VARIANT:

	*((VARIANT *)addr) = *((VARIANT *)&value->_variant.vtype);
	return;

__CLASS:

	*((void **)addr) = value->_class.class;
	return;

__VOID:
__FUNCTION:
__NULL:
__STRING:

	ERROR_panic("Bad type (%d) for VALUE_put", type);
}


/* This function must keep the datatype, as it is used for initializing local variables */

void VALUE_default(VALUE *value, TYPE type)
{
	static void *jump[16] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
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
	value->_single.value = 0;
	return;

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

__POINTER:
	value->_pointer.value = NULL;
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

__FUNCTION:
__CLASS:
__NULL:
	ERROR_panic("VALUE_default: Unknown default type");
}


void VALUE_convert(VALUE *value, TYPE type)
{
	static const void *jump[16][16] =
	{
	/*   ,------>  void       b          c          h          i          l          g          f          d          cs         s          p          v          func       class      n         */
	//  |
	/* void   */ { &&__OK,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    },
	/* b      */ { &&__N,     &&__OK,    &&__b2c,   &&__b2h,   &&__TYPE,  &&__b2l,   &&__b2g,   &&__b2f,   &&__N,     &&__b2s,   &&__b2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* c      */ { &&__N,     &&__c2b,   &&__OK,    &&__c2h,   &&__TYPE,  &&__c2l,   &&__c2g,   &&__c2f,   &&__c2d,   &&__c2s,   &&__c2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* h      */ { &&__N,     &&__h2b,   &&__h2c,   &&__OK,    &&__TYPE,  &&__h2l,   &&__h2g,   &&__h2f,   &&__h2d,   &&__h2s,   &&__h2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* i      */ { &&__N,     &&__i2b,   &&__i2c,   &&__i2h,   &&__OK,    &&__i2l,   &&__i2g,   &&__i2f,   &&__i2d,   &&__i2s,   &&__i2s,   &&__i2p,   &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* l      */ { &&__N,     &&__l2b,   &&__l2c,   &&__l2h,   &&__l2i,   &&__OK,    &&__l2g,   &&__l2f,   &&__l2d,   &&__l2s,   &&__l2s,   &&__l2p,   &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* g      */ { &&__N,     &&__g2b,   &&__g2c,   &&__g2h,   &&__g2i,   &&__g2l,   &&__OK,    &&__g2f,   &&__g2d,   &&__g2s,   &&__g2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* f      */ { &&__N,     &&__f2b,   &&__f2c,   &&__f2h,   &&__f2i,   &&__f2l,   &&__f2g,   &&__OK,    &&__f2d,   &&__f2s,   &&__f2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* d      */ { &&__N,     &&__d2b,   &&__d2c,   &&__d2h,   &&__d2i,   &&__d2l,   &&__d2g,   &&__d2f,   &&__OK,    &&__d2s,   &&__d2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* cs     */ { &&__N,     &&__s2b,   &&__s2c,   &&__s2h,   &&__s2i,   &&__s2l,   &&__s2g,   &&__s2f,   &&__s2d,   &&__OK,    &&__OK,    &&__N,     &&__s2v,   &&__N,     &&__N,     &&__N,     },
	/* s      */ { &&__N,     &&__s2b,   &&__s2c,   &&__s2h,   &&__s2i,   &&__s2l,   &&__s2g,   &&__s2f,   &&__s2d,   &&__OK,    &&__OK,    &&__N,     &&__s2v,   &&__N,     &&__N,     &&__N,     },
	/* p      */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__p2i,   &&__p2l,   &&__N,     &&__N,     &&__N,     &&__p2s,   &&__p2s,   &&__OK,    &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* v      */ { &&__N,     &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__OK,    &&__N,     &&__v2,    &&__v2,    },
	/* func   */ { &&__N,     &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__F2p,   &&__func,  &&__OK,    &&__N,     &&__func,  },
	/* class  */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__2v,    &&__N,     &&__OK,    &&__N,     },
	/* null   */ { &&__N,     &&__n2b,   &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__n2d,   &&__n2s,   &&__n2s,   &&__n2p,     &&__2v,    &&__N,     &&__N,     &&__OK,    },
	};

	int len;
	char *addr;
	CLASS *class;
	bool test;

__CONV:

	if ((type | value->type) >> 4)
		goto __OBJECT;
	else
		goto *jump[value->type][type];

__c2b:
__h2b:
__i2b:

	value->_integer.value = -(value->_integer.value != 0);
	value->type = T_BOOLEAN;
	return;

__l2b:

	value->_integer.value = -(value->_long.value != 0);
	value->type = T_BOOLEAN;
	return;

__g2b:

	value->_integer.value = -(value->_single.value != 0);
	value->type = T_BOOLEAN;
	return;

__f2b:

	value->_integer.value = -(value->_float.value != 0);
	value->type = T_BOOLEAN;
	return;

__d2b:
	value->_integer.value = -(value->_date.date != 0 || value->_date.time != 0);
	value->type = T_BOOLEAN;
	return;

__b2c:
__h2c:
__i2c:

	value->_integer.value = (unsigned char)value->_integer.value;
	value->type = T_BYTE;
	return;

__l2c:

	value->_integer.value = (unsigned char)value->_long.value;
	value->type = T_BYTE;
	return;

__g2c:

	value->_integer.value = (unsigned char)value->_single.value;
	value->type = T_BYTE;
	return;

__f2c:

	value->_integer.value = (unsigned char)value->_float.value;
	value->type = T_BYTE;
	return;

__b2h:
__c2h:
__i2h:

	value->_integer.value = (short)value->_integer.value;
	value->type = T_SHORT;
	return;

__l2h:

	value->_integer.value = (short)value->_long.value;
	value->type = T_SHORT;
	return;

__g2h:

	value->_integer.value = (short)value->_single.value;
	value->type = T_SHORT;
	return;

__f2h:

	value->_integer.value = (short)value->_float.value;
	value->type = T_SHORT;
	return;

__l2i:

	value->_integer.value = (int)value->_long.value;
	value->type = T_INTEGER;
	return;

__g2i:

	value->_integer.value = (int)value->_single.value;
	value->type = T_INTEGER;
	return;
	
__f2i:

	value->_integer.value = (int)value->_float.value;
	value->type = T_INTEGER;
	return;
	
__p2i:

	value->_integer.value = (int)(intptr_t)value->_pointer.value;
	value->type = T_INTEGER;
	return;

__b2l:
__c2l:
__h2l:
__i2l:

	value->_long.value = (int64_t)value->_integer.value;
	value->type = T_LONG;
	return;

__g2l:

	value->_long.value = (int64_t)value->_single.value;
	value->type = T_LONG;
	return;

__f2l:

	value->_long.value = (int64_t)value->_float.value;
	value->type = T_LONG;
	return;

__p2l:

	value->_long.value = (int64_t)(intptr_t)value->_pointer.value;
	value->type = T_LONG;
	return;

__b2g:
__c2g:
__h2g:
__i2g:

	value->_single.value = value->_integer.value;
	value->type = T_SINGLE;
	return;

__l2g:

	value->_single.value = (float)value->_long.value;
	if (!isfinite(value->_single.value))
		THROW(E_OVERFLOW);
	value->type = T_SINGLE;
	return;

__f2g:

	value->_single.value = (float)value->_float.value;
	if (!isfinite(value->_single.value))
		THROW(E_OVERFLOW);
	value->type = T_SINGLE;
	return;

__b2f:
__c2f:
__h2f:
__i2f:

	value->_float.value = value->_integer.value;
	value->type = T_FLOAT;
	return;

__l2f:

	value->_float.value = value->_long.value;
	value->type = T_FLOAT;
	return;

__g2f:

	value->_float.value = value->_single.value;
	value->type = T_FLOAT;
	return;

__c2d:
__h2d:
__i2d:

	value->_date.date = Max(0, value->_integer.value);
	value->_date.time = 0;
	value->type = T_DATE;
	return;

__l2d:

	if (value->_long.value < 0)
		value->_date.date = 0;
	else if (value->_long.value > INT_MAX)
		value->_date.date = INT_MAX;
	else
		value->_date.date = (int)value->_long.value;

	value->_date.time = 0;
	value->type = T_DATE;
	return;

__g2d:
	{
		float val = value->_single.value;
		float ival = floorf(val);
		value->_date.time = (int)((val - ival) * 86400000.0 + 0.5);
		value->_date.date = (int)ival;
		value->type = T_DATE;
		return;
	}

__f2d:
	{
		double val = value->_float.value;
		double ival = floor(val);
		value->_date.time = (int)((val - ival) * 86400000.0 + 0.5);
		value->_date.date = (int)ival;
		value->type = T_DATE;
		return;
	}

__d2c:
__d2h:
__d2i:

	value->_integer.value = value->_date.date;
	value->type = T_INTEGER;
	goto *jump[T_INTEGER][type];

__d2l:

	value->_long.value = value->_date.date;
	value->type = T_LONG;
	return;

__d2g:

	value->_single.value = (float)value->_date.date + (float)value->_date.time / 86400000.0;
	value->type = T_SINGLE;
	return;

__d2f:

	value->_float.value = (double)value->_date.date + (double)value->_date.time / 86400000.0;
	value->type = T_FLOAT;
	return;

__b2s:

	if (value->_boolean.value)
		STRING_char_value(value, 'T');
	else
		STRING_void_value(value);
	return;

__c2s:
__h2s:
__i2s:

/*len = sprintf(COMMON_buffer, "%d", value->_integer.value);
	STRING_new_temp_value(value, COMMON_buffer, len);*/
	NUMBER_int_to_string(value->_integer.value, 0, 10, value);
	BORROW(value);
	return;

__l2s:

/*len = sprintf(COMMON_buffer, "%" PRId64, value->_long.value);
	STRING_new_temp_value(value, COMMON_buffer, len);*/
	NUMBER_int_to_string(value->_long.value, 0, 10, value);
	BORROW(value);
	return;

__g2s:

	LOCAL_format_number(value->_single.value, LF_GENERAL_NUMBER, NULL, 0, &addr, &len, FALSE);
	STRING_new_temp_value(value, addr, len);
	BORROW(value);
	return;

__f2s:

	LOCAL_format_number(value->_float.value, LF_GENERAL_NUMBER, NULL, 0, &addr, &len, FALSE);
	STRING_new_temp_value(value, addr, len);
	BORROW(value);
	return;

__p2s:
	#if OS_64BITS
		NUMBER_int_to_string((int64_t)(intptr_t)value->_pointer.value, 0, 16, value);
	#else
		NUMBER_int_to_string((int)(intptr_t)value->_pointer.value, 0, 16, value);
	#endif
	BORROW(value);
	return;
	
__d2s:

	len = DATE_to_string(COMMON_buffer, value);
	STRING_new_temp_value(value, COMMON_buffer, len);
	BORROW(value);
	return;

__s2b:

	addr = value->_string.addr;
	value->_integer.value = -(addr != NULL && value->_string.len != 0);
	if (value->type == T_STRING)
		STRING_unref(&addr);
	value->type = T_BOOLEAN;
	return;

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

__s2g:

	addr = value->type == T_STRING ? value->_string.addr : NULL;

	if (NUMBER_from_string(NB_READ_FLOAT, value->_string.addr + value->_string.start, value->_string.len, value))
		goto __N;
	
	value->_single.value = value->_float.value;

	STRING_unref(&addr);
	value->type = type;
	return;

__s2f:

	addr = value->type == T_STRING ? value->_string.addr : NULL;

	if (NUMBER_from_string(NB_READ_FLOAT, value->_string.addr + value->_string.start, value->_string.len, value))
		goto __N;

	STRING_unref(&addr);
	value->type = type;
	return;

__s2d:

	addr = value->type == T_STRING ? value->_string.addr : NULL;

	if (DATE_from_string(value->_string.addr + value->_string.start, value->_string.len, value, FALSE))
		goto __N;

	STRING_unref(&addr);
	return;

__n2b:

	value->_integer.value = 0;
	value->type = T_BOOLEAN;
	return;

__n2d:

	DATE_void_value(value);
	return;

__n2s:

	STRING_void_value(value);
	return;

__n2p:

	value->_pointer.value = 0;
	value->type = T_POINTER;
	return;
	
__v2:

	VALUE_undo_variant(value);
	goto __CONV;

__s2v:

	addr = STRING_copy_from_value_temp(value);

	if (addr != value->_string.addr)
	{
		STRING_ref(addr);

		if (value->type == T_STRING)
			STRING_unref(&value->_string.addr);
	}

	value->_variant.vtype = T_STRING; //value->type;
	value->_variant.value._string = addr;
	value->type = T_VARIANT;
	return;

__2v:

	/* VALUE_put ne fonctionne pas avec T_STRING ! */
	if (value->type != T_NULL)
		VALUE_put(value, &value->_variant.value, value->type);

	value->_variant.vtype = value->type;
	value->type = T_VARIANT;
	return;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
	goto __N;

__i2p:
	value->_pointer.value = (void *)(intptr_t)value->_integer.value;
	value->type = T_POINTER;
	return;
	
__l2p:
	value->_pointer.value = (void *)(intptr_t)value->_long.value;
	value->type = T_POINTER;
	return;
	
__F2p:

	value->_pointer.value = EXTERN_make_callback(&value->_function);
	value->type = T_POINTER;
	return;

__OBJECT:

	if (!TYPE_is_object(type))
	{
		if (type == T_BOOLEAN)
		{
			test = (value->_object.object != NULL);
			OBJECT_UNREF(value->_object.object);
			value->_boolean.value = -test;
			value->type = T_BOOLEAN;
			return;
		}

		if (type == T_VARIANT)
			goto __2v;
		
		if (!value->_object.object)
			goto __N;
		
		if (value->type == T_OBJECT)
			class = OBJECT_class(value->_object.object);
		else
			class = value->_object.class;

		if (class->has_convert)
		{
			void *unref = value->_object.object;
			TYPE old_type = value->type;

			if (!((*class->convert)(value->_object.object, type, value)))
			{
				OBJECT_UNREF(unref);
				
				if (value->type == old_type)
					goto __TYPE;
				else
					goto __OK;
			}
		}
		
		goto __N;
	}

	if (!TYPE_is_object(value->type))
	{
		if (value->type == T_NULL)
		{
			OBJECT_null(value, (CLASS *)type); // Also works if type == T_OBJECT
			goto __TYPE;
		}

		if (value->type == T_POINTER)
		{
			class = (CLASS *)type;
			
			if (CLASS_is_struct(class))
			{
				value->_object.object = CSTRUCT_create_static(STRUCT_CONST, class, value->_pointer.value);
				OBJECT_REF(value->_object.object);
				goto __TYPE;
			}
		}
		
		if (value->type == T_VARIANT)
			goto __v2;

		if (value->type == T_FUNCTION)
			goto __func;

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

			OBJECT_REF(value->_object.object);
			value->type = T_OBJECT;
			/* on continue... */
		}
		else
		{
			if (TYPE_is_pure_object(type))
			{
				class = (CLASS *)type;

				if (class->has_convert)
				{
					if (!((*class->convert)(NULL, value->type, value)))
					{
						OBJECT_REF(value->_object.object);
						goto __TYPE;
					}
				}
			}
			
			goto __N;
		}
	}

	if (value->_object.object == NULL)
		goto __TYPE;

	if (value->type == T_OBJECT)
		class = OBJECT_class(value->_object.object);
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

	if (class->has_convert)
	{
		void *unref = value->_object.object;
		if (!((*class->convert)(value->_object.object, type, value)))
		{
			OBJECT_UNREF(unref);
			OBJECT_REF(value->_object.object);
			goto __TYPE;
		}
	}
	
	CLASS *class2 = (CLASS *)type;
	if (class2->has_convert)
	{
		void *unref = value->_object.object;
		if (!((*class2->convert)(NULL, OBJECT_class(unref), value)))
		{
			OBJECT_UNREF(unref);
			OBJECT_REF(value->_object.object);
			goto __TYPE;
		}
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



void VALUE_write(VALUE *value, void *addr, TYPE type)
{
	static void *jump[16] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
		};

	char *str;

__CONV:

	if (TYPE_is_object(type))
		goto __OBJECT;
	else
		goto *jump[type];

__BOOLEAN:

	VALUE_conv_boolean(value);
	*((unsigned char *)addr) = (value->_boolean.value != 0 ? 255 : 0);
	return;

__BYTE:

	VALUE_conv(value, T_BYTE);
	*((unsigned char *)addr) = (unsigned char)(value->_byte.value);
	return;

__SHORT:

	VALUE_conv(value, T_SHORT);
	*((short *)addr) = (short)(value->_short.value);
	return;

__INTEGER:

	VALUE_conv_integer(value);
	*((int *)addr) = value->_integer.value;
	return;

__LONG:

	VALUE_conv(value, T_LONG);
	*((int64_t *)addr) = value->_long.value;
	return;

__SINGLE:

	VALUE_conv(value, T_SINGLE);
	*((float *)addr) = value->_single.value;
	return;

__FLOAT:

	VALUE_conv_float(value);
	*((double *)addr) = value->_float.value;
	return;

__DATE:

	VALUE_conv(value, T_DATE);
	((int *)addr)[0] = value->_date.date;
	((int *)addr)[1] = value->_date.time;
	return;

__STRING:

	/* Il faut faire l'affectation en deux temps au cas o
		value->_string.addr == *addr ! */

	VALUE_conv_string(value);

	str = STRING_copy_from_value_temp(value);
	STRING_ref(str);
	STRING_unref((char **)addr);
	*((char **)addr) = str;
	return;

__OBJECT:

	VALUE_conv(value, type);

	OBJECT_REF(value->_object.object);
	OBJECT_UNREF(*((void **)addr));
	*((void **)addr) = value->_object.object;
	return;

__CLASS:

	VALUE_conv(value, type);

	OBJECT_REF(value->_class.class);
	OBJECT_UNREF(*((void **)addr));
	*((void **)addr) = value->_class.class;
	return;

__POINTER:

	VALUE_conv(value, T_POINTER);
	*((void **)addr) = value->_pointer.value;
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

	addr = &((VARIANT *)addr)->value.data;
	/*goto *jump[Min(T_OBJECT, type)];*/
	goto __CONV;

__VOID:
__FUNCTION:
__NULL:

	ERROR_panic("Bad type (%d) for VALUE_write", type);
}



void VALUE_read(VALUE *value, void *addr, TYPE type)
{
	static void *jump[16] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__CSTRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
		};

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

	value->_long.value = *((int64_t *)addr);
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

	{
		char *str = *((char **)addr);

		value->type = T_STRING;
		value->_string.addr = str;
		value->_string.start = 0;
		value->_string.len = STRING_length(str);

		return;
	}

__CSTRING:

	{
		char *str = *((char **)addr);

		value->type = T_CSTRING;
		value->_string.addr = str;
		value->_string.start = 0;
		value->_string.len = (str == NULL) ? 0 : strlen(str);

		return;
	}

__OBJECT:

	value->_object.object = *((void **)addr);
	return;

__POINTER:

	value->_pointer.value = *((void **)addr);
	return;

__VARIANT:

	value->_variant.type = T_VARIANT;
	value->_variant.vtype = ((VARIANT *)addr)->type;

	if (value->_variant.vtype == T_VOID)
		value->_variant.vtype = T_NULL;

	VARIANT_copy_value(&value->_variant, ((VARIANT *)addr));

	return;

__CLASS:

	value->_class.class = *((void **)addr);
	value->_class.super = NULL;
	return;

__VOID:
__FUNCTION:
__NULL:

	ERROR_panic("Bad type (%d) for VALUE_read", type);
}


void VALUE_free(void *addr, TYPE type)
{
	if (type == T_STRING)
	{
		STRING_unref((char **)addr);
		*((char **)addr) = NULL;
	}
	else if (TYPE_is_object(type))
	{
		OBJECT_UNREF(*((void **)addr));
		*((void **)addr) = NULL;
	}
	else if (type == T_VARIANT)
	{
		VARIANT_free((VARIANT *)addr);
		((VARIANT *)addr)->type = T_NULL;
	}
}



void VALUE_to_string(VALUE *value, char **addr, int *len)
{
	static void *jump[16] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
		};

__CONV:

	if (TYPE_is_object(value->type))
		goto __OBJECT;
	else
		goto *jump[value->type];

__NULL:

	*addr = ""; // To be coherent with Print "", as Null == "" */
	*len = 0;
	return;

__BOOLEAN:

	if (value->_boolean.value)
	{
		*addr = (char *)LOCAL_gettext("True");
		*len = strlen(*addr);
	}
	else
	{
		*addr = (char *)LOCAL_gettext("False");
		*len = strlen(*addr);
		//*addr = LOCAL_local.false_str;
		//*len = LOCAL_local.len_false_str;
	}
	return;

__BYTE:
__SHORT:
__INTEGER:

	*len = sprintf(COMMON_buffer, "%d", value->_integer.value);
	*addr = COMMON_buffer;

	return;

__LONG:

	*len = sprintf(COMMON_buffer, "%" PRId64, value->_long.value);
	*addr = COMMON_buffer;

	return;

__DATE:

	LOCAL_format_date(DATE_split(value), LF_STANDARD, NULL, 0, addr, len);
	return;

__SINGLE:

	LOCAL_format_number(value->_single.value, LF_SHORT_NUMBER, NULL, 0, addr, len, TRUE);
	return;

__FLOAT:

	LOCAL_format_number(value->_float.value, LF_STANDARD, NULL, 0, addr, len, TRUE);
	return;

__STRING:

	*len = value->_string.len;
	*addr = value->_string.addr + value->_string.start;
	return;

__OBJECT:

	{
		CLASS *class;
		
		if (VALUE_is_null(value))
			goto __NULL;

		class = OBJECT_class(value->_object.object);
		if (class->has_convert)
		{
			VALUE temp;
			if (!((*class->convert)(value->_object.object, T_CSTRING, &temp)))
			{
				*addr = temp._string.addr + temp._string.start;
				*len = temp._string.len;
				STRING_free_later(*addr);
				return;
			}
		}
		
		*len = sprintf(COMMON_buffer, "(%s %p)", class->name, value->_object.object);
		*addr = COMMON_buffer;
		return;
	}

__POINTER:

	if (VALUE_is_null(value))
		goto __NULL;

	*len = sprintf(COMMON_buffer, "(Pointer %p)", value->_pointer.value);
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

__FUNCTION:

	//if (unknown_function(value))
	//	goto __CONV;
	
	*len = sprintf(COMMON_buffer, "(Function %s:%d)", value->_function.class->name, value->_function.index);
	*addr = COMMON_buffer;
}


void VALUE_from_string(VALUE *value, const char *addr, int len)
{
	value->type = T_NULL;

	while (len > 0 && isspace(*addr))
		addr++, len--;
	
	while (len > 0 && isspace(addr[len - 1]))
		len--;
	
	if (len <= 0)
		return;

	if (!DATE_from_string(addr, len, value, TRUE))
		return;

	if (!NUMBER_from_string(NB_READ_ALL | NB_READ_HEX_BIN | NB_LOCAL, addr, len, value))
		return;

	if (len == LOCAL_local.len_true_str && strncasecmp(addr, LOCAL_local.true_str, len) == 0)
	{
		value->type = T_BOOLEAN;
		value->_boolean.value = -1;
		return;
	}

	if (len == LOCAL_local.len_false_str && strncasecmp(addr, LOCAL_local.false_str, len) == 0)
	{
		value->type = T_BOOLEAN;
		value->_boolean.value = 0;
		return;
	}
}


void VALUE_class_read(CLASS *class, VALUE *value, char *addr, CTYPE ctype, void *ref)
{
	VALUE_class_read_inline(class, value, addr, ctype, ref);
}


void VALUE_class_write(CLASS *class, VALUE *value, char *addr, CTYPE ctype)
{
	if (ctype.id == T_OBJECT)
	{
		TYPE type = (ctype.value >= 0) ? (TYPE)class->load->class_ref[ctype.value] : T_OBJECT;
		
		VALUE_conv(value, type);

		OBJECT_REF(value->_object.object);
		OBJECT_UNREF(*((void **)addr));
		*((void **)addr) = value->_object.object;
		//VALUE_write(value, addr, (ctype.value >= 0) ? (TYPE)class->load->class_ref[ctype.value] : T_OBJECT);
	}
	else if (ctype.id == TC_STRUCT)
	{
		TYPE type = (TYPE)class->load->class_ref[ctype.value];
		VALUE_conv(value, type);
		THROW_ILLEGAL();
	}
	else if (ctype.id == TC_ARRAY)
	{
		THROW_ILLEGAL();
	}
	else
	{
		VALUE_write(value, addr, (TYPE)ctype.id);
	}
}

void VALUE_class_constant(CLASS *class, VALUE *value, int ind)
{
	VALUE_class_constant_inline(class, value, ind);
}


bool VALUE_is_null(VALUE *val)
{
	static void *jump[16] = {
		&&__FALSE, &&__FALSE, &&__FALSE, &&__FALSE, &&__FALSE, &&__FALSE, &&__FALSE, &&__FALSE, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FALSE, &&__FALSE, &&__NULL
		};
	
	TYPE type = val->type;
	
	if (TYPE_is_object(type))
		return val->_object.object == NULL;
	else
		goto *jump[type];
	
__NULL:
	return TRUE;
	
__STRING:
	return val->_string.addr == 0 || val->_string.len == 0;
	
__DATE:
	return val->_date.date == 0 && val->_date.time == 0;
	
__POINTER:
	return val->_pointer.value == NULL;
	
__VARIANT:
	
	if (val->_variant.vtype == T_NULL)
		return TRUE;

	if (val->_variant.vtype == T_STRING)
		return val->_variant.value._string == NULL;

	if (val->_variant.vtype == T_DATE)
		return val->_variant.value.data == 0;
	
	if (val->_variant.vtype == T_POINTER)
		return val->_variant.value._pointer == NULL;

	if (TYPE_is_object(val->_variant.vtype))
		return val->_variant.value._object == NULL;
	
__FALSE:
	return FALSE;
}


void VALUE_convert_boolean(VALUE *value)
{
	static const void *jump[16] =
	{
		&&__NR, &&__OK, &&__c2b, &&__h2b, &&__i2b, &&__l2b, &&__g2b, &&__f2b,
		&&__d2b, &&__s2b, &&__s2b, &&__N, &&__v2, &&__func, &&__N, &&__n2b
	};

	char *addr;

__CONV:

	if (TYPE_is_object(value->type))
	{
		if (value->_object.object)
		{
			OBJECT_just_unref(value->_object.object);
			value->_boolean.value = -1;
		}
		value->type = T_BOOLEAN;
		return;
	}
	else
		goto *jump[value->type];

__c2b:
__h2b:
__i2b:

	value->_integer.value = (value->_integer.value != 0) ? -1 : 0;
	value->type = T_BOOLEAN;
	return;

__l2b:

	value->_integer.value = (value->_long.value != 0) ? -1 : 0;
	value->type = T_BOOLEAN;
	return;

__g2b:

	value->_integer.value = (value->_single.value != 0) ? -1 : 0;
	value->type = T_BOOLEAN;
	return;

__f2b:

	value->_integer.value = (value->_float.value != 0) ? -1 : 0;
	value->type = T_BOOLEAN;
	return;

__d2b:
	
	value->_integer.value = (value->_date.date != 0 || value->_date.time != 0) ? -1 : 0;
	value->type = T_BOOLEAN;
	return;

__s2b:

	addr = value->_string.addr;
	value->_integer.value = ((addr != NULL) && (value->_string.len != 0)) ? -1 : 0;
	if (value->type == T_STRING)
		STRING_unref(&addr);
	value->type = T_BOOLEAN;
	return;

__n2b:

	value->_integer.value = 0;
	value->type = T_BOOLEAN;
	return;

__v2:

	VALUE_undo_variant(value);
	goto __CONV;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
		goto __N;

__N:

	THROW(E_TYPE, "Boolean", TYPE_get_name(value->type));

__NR:

	THROW(E_NRETURN);
	
__OK:
	return;
}


void VALUE_convert_integer(VALUE *value)
{
	static const void *jump[16] =
	{
		&&__NR, &&__TYPE, &&__TYPE, &&__TYPE, &&__OK, &&__l2i, &&__g2i, &&__f2i,
		&&__d2i, &&__s2i, &&__s2i, &&__N, &&__v2, &&__func, &&__N, &&__N
	};
		
	char *addr;

__CONV:

	goto *jump[value->type];

__l2i:

	value->_integer.value = (int)value->_long.value;
	goto __TYPE;

__g2i:

	value->_integer.value = (int)value->_single.value;
	goto __TYPE;

__f2i:

	value->_integer.value = (int)value->_float.value;
	goto __TYPE;

__d2i:

	value->_integer.value = value->_date.date;
	goto __TYPE;

__s2i:

	addr = value->type == T_STRING ? value->_string.addr : NULL;

	if (NUMBER_from_string(NB_READ_INTEGER, value->_string.addr + value->_string.start, value->_string.len, value))
		goto __N;

	STRING_unref(&addr);
	goto __TYPE;

__v2:

	VALUE_undo_variant(value);
	if (TYPE_is_object(value->type))
		goto __N;
	else
		goto __CONV;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
		goto __N;

__TYPE:

	value->type = T_INTEGER;

__OK:

	return;

__N:

	THROW_TYPE_INTEGER(value->type);

__NR:

	THROW(E_NRETURN);
}


void VALUE_convert_float(VALUE *value)
{
	static const void *jump[16] =
	{
		&&__NR, &&__b2f, &&__c2f, &&__h2f, &&__i2f, &&__l2f, &&__g2f, &&__OK,
		&&__d2f, &&__s2f, &&__s2f, &&__N, &&__v2, &&__func, &&__N, &&__N
	};
	
	char *addr;

__CONV:

	if (TYPE_is_object(value->type))
		goto __N;
	else
		goto *jump[value->type];

__b2f:
__c2f:
__h2f:
__i2f:

	value->_float.value = value->_integer.value;
	value->type = T_FLOAT;
	return;

__l2f:

	value->_float.value = value->_long.value;
	value->type = T_FLOAT;
	return;
	
__g2f:

	value->_float.value = value->_single.value;
	value->type = T_FLOAT;
	return;

__d2f:

	value->_float.value = (double)value->_date.date + (double)value->_date.time / 86400000.0;
	value->type = T_FLOAT;
	return;

__s2f:

	addr = value->type == T_STRING ? value->_string.addr : NULL;

	if (NUMBER_from_string(NB_READ_FLOAT, value->_string.addr + value->_string.start, value->_string.len, value))
		goto __N;

	STRING_unref(&addr);
	return;

__v2:

	VALUE_undo_variant(value);
	if (TYPE_is_object(value->type))
		goto __N;
	else
		goto __CONV;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
		goto __N;

__N:

	THROW_TYPE_FLOAT(value->type);

__NR:

	THROW(E_NRETURN);
	
__OK:

	return;
}


void VALUE_convert_string(VALUE *value)
{
	static const void *jump[16] =
	{
		&&__NR, &&__b2s, &&__c2s, &&__h2s, &&__i2s, &&__l2s, &&__g2s, &&__f2s,
		&&__d2s, &&__OK, &&__OK, &&__N, &&__v2, &&__func, &&__N, &&__n2s
	};

	int len;
	char *addr;

__CONV:

	goto *jump[value->type];

__b2s:

	if (value->_boolean.value)
		STRING_char_value(value, 'T');
	else
		STRING_void_value(value);
	return;

__c2s:
__h2s:
__i2s:

	NUMBER_int_to_string(value->_integer.value, 0, 10, value);
	BORROW(value);
	return;

__l2s:

	NUMBER_int_to_string(value->_long.value, 0, 10, value);
	BORROW(value);
	return;

__g2s:

	LOCAL_format_number(value->_single.value, LF_GENERAL_NUMBER, NULL, 0, &addr, &len, FALSE);
	STRING_new_temp_value(value, addr, len);
	BORROW(value);
	return;

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

__n2s:

	STRING_void_value(value);
	return;

__v2:

	VALUE_undo_variant(value);
	if (TYPE_is_object(value->type))
		goto __N;
	else
		goto __CONV;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
		goto __N;

__OK:

	return;

__N:

	THROW_TYPE_STRING(value->type);

__NR:

	THROW(E_NRETURN);
}


void VALUE_convert_variant(VALUE *value)
{
	static const void *jump[16] =
	{
		&&__NR, &&__2v, &&__2v, &&__2v, &&__2v, &&__2v, &&__2v, &&__2v,
		&&__2v, &&__s2v, &&__s2v, &&__2v, &&__OK, &&__func, &&__2v, &&__2v
	};

	char *addr;

//__CONV:

	if (TYPE_is_object(value->type))
		goto __2v;
	else
		goto *jump[value->type];

__s2v:

	addr = STRING_copy_from_value_temp(value);

	if (addr != value->_string.addr)
	{
		STRING_ref(addr);

		if (value->type == T_STRING)
			STRING_unref(&value->_string.addr);
	}

	value->_variant.value._string = addr;
	value->_variant.vtype = T_STRING;

	goto __TYPE;

__2v:

	/* VALUE_put ne fonctionne pas avec T_STRING ! */
	if (value->type != T_NULL)
		VALUE_put(value, &value->_variant.value, value->type);

	value->_variant.vtype = value->type;
	goto __TYPE;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
		goto __N;

__TYPE:

	value->type = T_VARIANT;

__OK:

	return;

__N:

	THROW(E_TYPE, "Variant", TYPE_get_name(value->type));

__NR:

	THROW(E_NRETURN);
}

#if 0
void VALUE_convert_object(VALUE *value, TYPE type)
{
	CLASS *class;

__CONV:

	#if 0
	if (!TYPE_is_object(type))
	{
		if (type == T_BOOLEAN)
		{
			test = (value->_object.object != NULL);
			OBJECT_UNREF(value->_object.object);
			value->_boolean.value = test ? -1 : 0;
			goto __TYPE;
		}

		if (type == T_VARIANT)
			goto __2v;
		
		goto __N;
	}
	#endif

	if (!TYPE_is_object(value->type))
	{
		if (value->type == T_NULL)
		{
			OBJECT_null(value, (CLASS *)type); /* marche aussi pour type = T_OBJECT */
			goto __TYPE;
		}

		if (value->type == T_VARIANT)
			goto __v2;

		if (value->type == T_FUNCTION)
			goto __func;

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

			OBJECT_REF(value->_object.object);
			value->type = T_OBJECT;
			/* on continue... */
		}
		else
			goto __N;

	}

	if (value->_object.object == NULL)
		goto __TYPE;

	if (value->type == T_OBJECT)
	{
		/*if (value->_object.object == NULL)
			goto __TYPE;*/

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

	if (class->special[SPEC_CONVERT] != NO_SYMBOL)
	{
		void *conv = ((void *(*)())(CLASS_get_desc(class, class->special[SPEC_CONVERT])->constant.value._pointer))(value->_object.object, type);
		if (conv)
		{
			OBJECT_REF(conv);
			OBJECT_UNREF(value->_object.object);
			value->_object.object = conv;
			goto __TYPE;
		}
	}
	
	THROW(E_TYPE, TYPE_get_name(type), TYPE_get_name((TYPE)class));

__v2:

	VALUE_undo_variant(value);
	goto __CONV;

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
		goto __N;

__TYPE:

	value->type = type;
	return;

__N:

	THROW(E_TYPE, TYPE_get_name(type), TYPE_get_name(value->type));
}
#endif

void VALUE_undo_variant(VALUE *value)
{
	static void *jump[16] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__CSTRING, &&__POINTER, &&__VOID, &&__FUNCTION, &&__CLASS, &&__NULL
		};

	TYPE type = value->_variant.vtype;

	//if (index != T_NULL)
	//	VALUE_read(value, &value->_variant.value, value->_variant.vtype);

	value->type = type;

	if (TYPE_is_object(type))
		goto __OBJECT;
	else
		goto *jump[type];

__BOOLEAN:

	value->_boolean.value = value->_variant.value._boolean ? -1 : 0;
	return;

__BYTE:

	value->_byte.value = value->_variant.value._byte;
	return;

__SHORT:

	value->_short.value = value->_variant.value._short;
	return;

__INTEGER:

	value->_integer.value = value->_variant.value._integer;
	return;

__LONG:

	value->_long.value = value->_variant.value._long;
	return;

__SINGLE:

	value->_single.value = value->_variant.value._single;
	return;

__FLOAT:

	value->_float.value = value->_variant.value._float;
	return;

__DATE:

	// It works, as the normal date field is before the variant date field!
	value->_date.date = value->_variant.value._date.date;
	value->_date.time = value->_variant.value._date.time;
	return;

__STRING:

	{
		char *str = value->_variant.value._string;

		value->type = T_STRING;
		value->_string.addr = str;
		value->_string.start = 0;
		value->_string.len = STRING_length(str);

		return;
	}

__CSTRING:

	{
		char *str = value->_variant.value._string;

		value->type = T_CSTRING;
		value->_string.addr = str;
		value->_string.start = 0;
		value->_string.len = strlen(str);

		return;
	}

__OBJECT:

	value->_object.object = value->_variant.value._object;
	return;

__POINTER:

	value->_pointer.value = value->_variant.value._pointer;
	return;

__CLASS: // Is it useful for variants ?

	value->_class.class = value->_variant.value._object;
	value->_class.super = NULL;
	return;

__NULL:
	return;
	
__VOID:
__FUNCTION:

	ERROR_panic("Bad type (%d) for VALUE_undo_variant", type);
}
