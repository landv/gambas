/***************************************************************************

  gbx_exec_operator.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_EXEC_OPERATOR_C

#include "gb_common.h"
#include "gbx_exec.h"

typedef
	void *(*FUNC_O_OF)(void *, double);

typedef
	void *(*FUNC_O_OO)(void *, void *);

typedef
	int (*FUNC_I_OF)(void *, double);

typedef
	int (*FUNC_I_OO)(void *, void *);

typedef
	void *(*FUNC_O_O)(void *);

typedef
	double (*FUNC_F_O)(void *);

	
bool EXEC_check_operator_single(VALUE *P1)
{
	return (TYPE_is_object(P1->type) && P1->_object.object && OBJECT_class(P1->_object.object)->has_operators);
}

int EXEC_check_operator(VALUE *P1, VALUE *P2)
{
	if (TYPE_is_number(P1->type) && TYPE_is_object(P2->type))
	{
		if (P2->_object.object && OBJECT_class(P2->_object.object)->has_operators)
		{
			//*dynamic = P2->type == T_OBJECT;
			return OP_FLOAT_OBJECT;
		}
	}
	else if (TYPE_is_number(P2->type) && TYPE_is_object(P1->type))
	{
		if (P1->_object.object && OBJECT_class(P1->_object.object)->has_operators)
		{
			//*dynamic = P1->type == T_OBJECT;
			return OP_OBJECT_FLOAT;
		}
	}
	else if (TYPE_are_objects(P1->type, P2->type) && OBJECT_are_not_null(P1->_object.object, P2->_object.object))
	{
		CLASS *class1 = OBJECT_class(P1->_object.object);
		CLASS *class2 = OBJECT_class(P2->_object.object);
		
		//*dynamic = P1->type == T_OBJECT || P2->type = T_OBJECT;
		
		if (class1 && class1->has_operators)
		{
			if (class1 == class2)
				return OP_OBJECT_OBJECT;
			
			if (class2 && class2->has_operators)
				return OP_OBJECT_CONV;
		}
	}
	
	return OP_NOTHING;
}

void EXEC_operator(uchar what, uchar op, VALUE *P1, VALUE *P2)
{
	static void *jump[] = { NULL, &&__OBJECT_FLOAT, &&__FLOAT_OBJECT, &&__OBJECT_CONV, &&__CONV_OBJECT, &&__OBJECT_OBJECT };
	
	void *func;
	void *result;
	
	goto *jump[what];
	
__OBJECT_FLOAT:

	if (!P1->_object.object)
		THROW(E_NULL);
	
	func = ((void **)(OBJECT_class(P1->_object.object)->operators))[op];
	VALUE_conv_float(P2);
	result = (*(FUNC_O_OF)func)(P1->_object.object, P2->_float.value);
	OBJECT_REF(result, "EXEC_operator");
	OBJECT_UNREF(P1->_object.object, "EXEC_operator");
	goto __END;
	
__FLOAT_OBJECT:

	if (!P2->_object.object)
		THROW(E_NULL);

	func = ((void **)(OBJECT_class(P2->_object.object)->operators))[op];
	VALUE_conv_float(P1);
	result = (*(FUNC_O_OF)func)(P2->_object.object, P1->_float.value);
	OBJECT_REF(result, "EXEC_operator");
	P1->_object.class = P2->_object.class;
	OBJECT_UNREF(P2->_object.object, "EXEC_operator");
	goto __END;
	
__OBJECT_CONV:

	VALUE_conv(P2, (TYPE)P1->_object.class);
	goto __OBJECT_OBJECT;
	
__CONV_OBJECT:

	VALUE_conv(P1, (TYPE)P2->_object.class);
	goto __OBJECT_OBJECT;

__OBJECT_OBJECT:

	if (!OBJECT_are_not_null(P1->_object.object, P2->_object.object))
		THROW(E_NULL);
	
	func = ((void **)(OBJECT_class(P1->_object.object)->operators))[op];
	result = (*(FUNC_O_OO)func)(P1->_object.object, P2->_object.object);
	OBJECT_REF(result, "EXEC_operator");
	OBJECT_UNREF(P1->_object.object, "EXEC_operator");
	OBJECT_UNREF(P2->_object.object, "EXEC_operator");

__END:
	
	P1->_object.object = result;
}

void EXEC_operator_object_add_quick(VALUE *P1, double val)
{
	if (P1->_object.object)
	{
		void *func = ((void **)(OBJECT_class(P1->_object.object)->operators))[CO_ADDF];
		void *result = (*(FUNC_O_OF)func)(P1->_object.object, val);
		OBJECT_REF(result, "EXEC_operator_object_float_direct");
		OBJECT_UNREF(P1->_object.object, "EXEC_operator_object_float_direct");
		P1->_object.object = result;
	}
	else
		THROW(E_NULL);
}


bool EXEC_comparator(uchar what, uchar op, VALUE *P1, VALUE *P2)
{
	static void *jump[] = { NULL, &&__OBJECT_FLOAT, &&__FLOAT_OBJECT, &&__OBJECT_CONV, &&__CONV_OBJECT, &&__OBJECT_OBJECT };
	
	void *func;
	int result;
	
	goto *jump[what];
	
__OBJECT_FLOAT:

	func = ((void **)(OBJECT_class(P1->_object.object)->operators))[op];
	VALUE_conv_float(P2);
	result = (*(FUNC_I_OF)func)(P1->_object.object, P2->_float.value);
	OBJECT_UNREF(P1->_object.object, "EXEC_comparator");
	return result;
	
__FLOAT_OBJECT:

	func = ((void **)(OBJECT_class(P2->_object.object)->operators))[op];
	VALUE_conv_float(P1);
	result = (*(FUNC_I_OF)func)(P2->_object.object, P1->_float.value);
	OBJECT_UNREF(P2->_object.object, "EXEC_comparator");
	return result;
	
__OBJECT_CONV:

	VALUE_conv(P2, (TYPE)P1->_object.class);
	goto __OBJECT_OBJECT;
	
__CONV_OBJECT:

	VALUE_conv(P1, (TYPE)P2->_object.class);
	goto __OBJECT_OBJECT;

__OBJECT_OBJECT:

	func = ((void **)(OBJECT_class(P1->_object.object)->operators))[op];
	result = (*(FUNC_I_OO)func)(P1->_object.object, P2->_object.object);
	OBJECT_UNREF(P1->_object.object, "EXEC_comparator");
	OBJECT_UNREF(P2->_object.object, "EXEC_comparator");
	return result != 0;
}

void EXEC_operator_object_abs(VALUE *P1)
{
	if (P1->_object.object)
	{
		void *func = ((void **)(OBJECT_class(P1->_object.object)->operators))[CO_ABS];
		double result = (*(FUNC_F_O)func)(P1->_object.object);
		OBJECT_UNREF(P1->_object.object, "EXEC_operator_object_abs");
		P1->type = T_FLOAT;
		P1->_float.value = result;
	}
	else
		THROW(E_NULL);
}

void EXEC_operator_object_single(uchar op, VALUE *P1)
{
	if (P1->_object.object)
	{
		void *func = ((void **)(OBJECT_class(P1->_object.object)->operators))[op];
		void *result = (*(FUNC_O_O)func)(P1->_object.object);
		OBJECT_REF(result, "EXEC_operator_object_single");
		OBJECT_UNREF(P1->_object.object, "EXEC_operator_object_single");
		P1->_object.object = result;
	}
	else
		THROW(E_NULL);
}
