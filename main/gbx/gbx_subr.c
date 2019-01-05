/***************************************************************************

	gbx_subr.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __GBX_SUBR_C

#include "gb_common.h"
#include "gbx_subr.h"
#include "gambas.h"
#include "gbx_api.h"

/*int NPARAM;*/

void SUBR_leave_void(int nparam)
{
	RELEASE_MANY(SP, nparam);
	
	SP->type = T_VOID;
	SP++;
}

void SUBR_leave(int nparam)
{
	BORROW(RP);

	RELEASE_MANY(SP, nparam);
	
	//*SP++ = *RP;
	COPY_VALUE(SP, RP);
	SP++;
	RP->type = T_VOID;
}


bool SUBR_check_string_real(VALUE *param)
{
__RETRY:

	if (TYPE_is_string(param->type))
		return (param->_string.len == 0);

	if (TYPE_is_null(param->type))
		return TRUE;

	if (param->type == T_VARIANT)
	{
		VARIANT_undo(param);
		goto __RETRY;
	}

	THROW_TYPE(T_STRING, param->type);
}


void SUBR_check_integer(VALUE *param)
{
	if (param->type == T_VARIANT)
		VARIANT_undo(param);

	if (TYPE_is_integer(param->type))
		return;

	THROW_TYPE(T_INTEGER, param->type);
}


void SUBR_check_float(VALUE *param)
{
	if (param->type == T_VARIANT)
		VARIANT_undo(param);

	if (TYPE_is_number(param->type))
	{
		VALUE_conv_float(param);
		return;
	}

	THROW_TYPE(T_INTEGER, param->type);
}


int SUBR_get_integer(VALUE *param)
{
	SUBR_check_integer(param);
	return param->_integer.value;
}


void *SUBR_get_pointer(VALUE *param)
{
	if (param->type == T_VARIANT)
		VARIANT_undo(param);

	if (param->type != T_POINTER)
		THROW_TYPE(T_POINTER, param->type);
	
	return (void *)param->_pointer.value;
}

void *SUBR_get_pointer_or_string(VALUE *param)
{
	if (param->type == T_VARIANT)
		VARIANT_undo(param);

	if (TYPE_is_string(param->type))
		return (void *)(param->_string.addr + param->_string.start);
	
	if (param->type == T_POINTER)
		return (void *)param->_pointer.value;
	
	THROW_TYPE(T_POINTER, param->type);
}


double SUBR_get_float(VALUE *param)
{
	SUBR_check_float(param);
	return param->_float.value;
}


char *SUBR_get_string(VALUE *param)
{
	if (SUBR_check_string(param))
		return "";

	return STRING_copy_from_value_temp(param);
}


bool SUBR_get_boolean(VALUE *param)
{
	VALUE_conv_boolean(param);
	return param->_boolean.value;
}

static TYPE conv_type(TYPE type)
{
	/*if (type <= T_BYTE)
		type = T_BYTE;*/
	if (type == T_CSTRING) // || type == T_NULL)
		type = T_STRING;
	else if (type == T_CLASS || type == T_FUNCTION)
		type = T_VARIANT;

	return type;
}

TYPE SUBR_check_good_type(VALUE *param, int count)
{
	int i;
	TYPE type, type2;
	
	if (count == 0)
		goto __VARIANT;

	type = conv_type(param[0].type);
	
	if (type == T_VARIANT)
		goto __VARIANT;
	
	if (TYPE_is_value(type))
	{
		for (i = 1; i < count; i++)
		{
			type2 = conv_type(param[i].type);
			
			if (type2 == type)
				continue;
			
			if (type == T_NULL)
			{
				if (type2 <= T_FLOAT)
					goto __VARIANT;
				
				type = type2;
				continue;
			}
			
			if (type <= T_FLOAT && type2 <= T_FLOAT)
			{
				if (type2 > type)
					type = type2;
				continue;
			}
			
			if (type2 == T_NULL)
			{
				if (type <= T_FLOAT)
					goto __VARIANT;
				else
					continue;
			}
			
			if (TYPE_is_object(type) && TYPE_is_object(type2))
			{
				type = T_OBJECT;
				continue;
			}
			
			type = T_VARIANT;
			break;
		}
	}
	
	if (type == T_VOID)
		THROW(E_NRETURN);
	else if (!TYPE_is_value(type))
		THROW_TYPE(T_VARIANT, type);
	
	return type;
	
__VARIANT:

	return T_VARIANT;
}

TYPE SUBR_get_type(VALUE *param)
{
	if (param->type == T_INTEGER)
		return (TYPE)param->_integer.value;
	if (param->type == T_CLASS)
		return (TYPE)param->_class.class;
	THROW_ILLEGAL();
}
