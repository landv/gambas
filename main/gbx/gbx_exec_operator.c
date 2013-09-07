/***************************************************************************

  gbx_exec_operator.c

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

#define __GBX_EXEC_OPERATOR_C

#include "gb_common.h"
#include "gbx_type.h"
#include "gbx_api.h"
#include "gbx_exec.h"

typedef
	void *(*FUNC_O_OF)(void *, double, bool);

typedef
	void *(*FUNC_O_OO)(void *, void *, bool);

typedef
	int (*FUNC_I_OF)(void *, double, bool);

typedef
	int (*FUNC_I_OO)(void *, void *, bool);

typedef
	void *(*FUNC_O_O)(void *);

typedef
	int (*FUNC_I_O)(void *);

typedef
	double (*FUNC_F_O)(void *);

static void raise_error(void *o1, void *o2)
{
	if (o2 && OBJECT_class(o2) == OBJECT_class(o1))
		GB_Error((char *)E_TYPE, "Number", TYPE_get_name((TYPE)OBJECT_class(o1)));
	else
		GB_Error((char *)E_TYPE, TYPE_get_name((TYPE)OBJECT_class(o1)), o2 ? TYPE_get_name((TYPE)OBJECT_class(o2)) : "Number");
}

bool EXEC_check_operator_single(VALUE *P1, uchar op)
{
	return (TYPE_is_object(P1->type) && P1->_object.object && OBJECT_class(P1->_object.object)->has_operators 
		&& CLASS_has_operator(OBJECT_class(P1->_object.object), op));
}

int EXEC_check_operator(VALUE *P1, VALUE *P2, uchar op)
{
	CLASS *class1, *class2;
	
	if (TYPE_is_number(P1->type) && TYPE_is_object(P2->type))
	{
		if (P2->_object.object && OBJECT_class(P2->_object.object)->has_operators && CLASS_has_operator(OBJECT_class(P2->_object.object), op + 1))
		{
			//*dynamic = P2->type == T_OBJECT;
			return OP_FLOAT_OBJECT;
		}
	}
	else if (TYPE_is_number(P2->type) && TYPE_is_object(P1->type))
	{
		if (P1->_object.object && OBJECT_class(P1->_object.object)->has_operators && CLASS_has_operator(OBJECT_class(P1->_object.object), op + 1))
		{
			//*dynamic = P1->type == T_OBJECT;
			return OP_OBJECT_FLOAT;
		}
	}
	else if (TYPE_are_objects(P1->type, P2->type) && OBJECT_are_not_null(P1->_object.object, P2->_object.object))
	{
		class1 = OBJECT_class(P1->_object.object);
		class2 = OBJECT_class(P2->_object.object);
		
		//*dynamic = P1->type == T_OBJECT || P2->type = T_OBJECT;
		
		if (class1->has_operators)
		{
			if (class1 == class2 && CLASS_has_operator(class1, op))
				return OP_OBJECT_OBJECT;
			
			if (class2->has_operators)
			{
				if (CLASS_get_operator_strength(class1) > CLASS_get_operator_strength(class2) && CLASS_has_operator(class1, op + 2))
					return OP_OBJECT_OTHER;
				else if (CLASS_has_operator(class2, op + 2))
					return OP_OTHER_OBJECT;
			}
			else if (CLASS_has_operator(class1, op))
				return OP_OBJECT_OTHER;
		}
		else if (class2->has_operators && CLASS_has_operator(class2, op + 2))
			return OP_OTHER_OBJECT;
	}
	
	return OP_NOTHING;
}

void EXEC_operator(uchar what, uchar op, VALUE *P1, VALUE *P2)
{
	static void *jump[] = { NULL, &&__OBJECT_FLOAT, &&__FLOAT_OBJECT, &&__OBJECT_OTHER, &&__OTHER_OBJECT, &&__OBJECT_OBJECT };
	
	void *func;
	void *result;
	bool invert;
	void *o1, *o2;
	
	goto *jump[what];
	
__OBJECT_FLOAT:

	o1 = P1->_object.object;
	if (!o1)
		THROW(E_NULL);
	
	func = OBJECT_class(o1)->operators[op];
	VALUE_conv_float(P2);
	result = (*(FUNC_O_OF)func)(o1, P2->_float.value, FALSE);
	OBJECT_REF(result);
	OBJECT_UNREF(o1);

	if (!result)
	{
		if (op != CO_DIVF)
			raise_error(o1, NULL);
	}
	
	goto __END;
	
__FLOAT_OBJECT:

	o1 = P2->_object.object;
	if (!o1)
		THROW(E_NULL);

	func = OBJECT_class(o1)->operators[op];
	VALUE_conv_float(P1);
	result = (*(FUNC_O_OF)func)(o1, P1->_float.value, TRUE);
	OBJECT_REF(result);
	P1->_object.class = P2->_object.class;
	OBJECT_UNREF(o1);
	
	if (!result && !EXEC_has_native_error())
		raise_error(o1, NULL);
	
	goto __END;
	
__OTHER_OBJECT:

	o2 = P1->_object.object;
	o1 = P2->_object.object;
	
	invert = TRUE;
	goto __OTHER;

__OBJECT_OTHER:
__OBJECT_OBJECT:

	o1 = P1->_object.object;
	o2 = P2->_object.object;

	invert = FALSE;
	goto __OTHER;

__OTHER:

	if (!OBJECT_are_not_null(o1, o2))
		THROW(E_NULL);
	
	func = OBJECT_class(o1)->operators[op];
	result = (*(FUNC_O_OO)func)(o1, o2, invert);
	OBJECT_REF(result);
	OBJECT_UNREF(o1);
	OBJECT_UNREF(o2);

	if (!result && !EXEC_has_native_error())
		raise_error(o1, o2);
	
__END:
	
	P1->_object.object = result;

	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}
}

void EXEC_operator_object_add_quick(VALUE *P1, double val)
{
	if (P1->_object.object)
	{
		void *func = OBJECT_class(P1->_object.object)->operators[CO_ADDF];
		void *result = (*(FUNC_O_OF)func)(P1->_object.object, val, FALSE);
		OBJECT_REF(result);
		OBJECT_UNREF(P1->_object.object);
		P1->_object.object = result;
	}
	else
		THROW(E_NULL);

	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}
}


int EXEC_comparator(uchar what, uchar op, VALUE *P1, VALUE *P2)
{
	static void *jump[] = { NULL, &&__OBJECT_FLOAT, &&__FLOAT_OBJECT, &&__OBJECT_OTHER, &&__OTHER_OBJECT, &&__OBJECT_OBJECT };
	
	void *func;
	int result;
	bool invert;
	void *o1, *o2;
	
	goto *jump[what];
	
__OBJECT_FLOAT:

	o1 = P1->_object.object;
	if (!o1)
		THROW(E_NULL);
	
	func = OBJECT_class(o1)->operators[op];
	VALUE_conv_float(P2);
	result = (*(FUNC_I_OF)func)(o1, P2->_float.value, FALSE);
	OBJECT_UNREF(o1);

	if (result < (-1))
		raise_error(o1, NULL);
	
	goto __END;
	
__FLOAT_OBJECT:

	o2 = P2->_object.object;
	if (!o2)
		THROW(E_NULL);
	
	func = OBJECT_class(o2)->operators[op];
	VALUE_conv_float(P1);
	result = (*(FUNC_I_OF)func)(o2, P1->_float.value, TRUE);
	OBJECT_UNREF(o2);

	if (result < (-1))
		raise_error(o2, NULL);
	
	goto __END;
	
__OTHER_OBJECT:

	o2 = P1->_object.object;
	o1 = P2->_object.object;
	invert = TRUE;
	goto __OTHER;

__OBJECT_OTHER:
__OBJECT_OBJECT:

	o1 = P1->_object.object;
	o2 = P2->_object.object;
	invert = FALSE;
	goto __OTHER;

__OTHER:

	if (!OBJECT_are_not_null(o1, o2))
		THROW(E_NULL);
	
	func = OBJECT_class(o1)->operators[op];
	result = (*(FUNC_I_OO)func)(o1, o2, invert);
	OBJECT_UNREF(o1);
	OBJECT_UNREF(o2);
	//result = !!result; // result != 0;
	
	if (result < (-1))
		raise_error(o1, o2);
	
__END:

	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}

	return result;
}

void EXEC_operator_object_sgn(VALUE *P1)
{
	if (P1->_object.object)
	{
		void *func = OBJECT_class(P1->_object.object)->operators[CO_SGN];
		int result = (*(FUNC_I_O)func)(P1->_object.object);
		OBJECT_UNREF(P1->_object.object);
		P1->type = T_INTEGER;
		P1->_integer.value = result;
	}
	else
		THROW(E_NULL);

	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}
}

void EXEC_operator_object_fabs(VALUE *P1)
{
	if (P1->_object.object)
	{
		void *func = OBJECT_class(P1->_object.object)->operators[CO_FABS];
		double result = (*(FUNC_F_O)func)(P1->_object.object);
		OBJECT_UNREF(P1->_object.object);
		P1->type = T_FLOAT;
		P1->_float.value = result;
	}
	else
		THROW(E_NULL);

	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}
}

void EXEC_operator_object_single(uchar op, VALUE *P1)
{
	if (P1->_object.object)
	{
		void *func = OBJECT_class(P1->_object.object)->operators[op];
		void *result = (*(FUNC_O_O)func)(P1->_object.object);
		OBJECT_REF(result);
		OBJECT_UNREF(P1->_object.object);
		P1->_object.object = result;
	}
	else
		THROW(E_NULL);
	
	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}
}
