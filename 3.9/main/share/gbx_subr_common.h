/***************************************************************************

	gbx_subr_common.h

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

#ifdef STATIC_SUBR
#undef STATIC_SUBR
#define STATIC_SUBR static
#endif

#define OP_OBJECT_FLOAT (T_POINTER + 1)
#define OP_FLOAT_OBJECT (T_POINTER + 2)
#define OP_OBJECT_CONV  (T_POINTER + 3)
#define OP_CONV_OBJECT  (T_POINTER + 4)
#define OP_OBJECT       (T_POINTER + 5)

static int check_operators(VALUE *P1, VALUE *P2)
{
	if (TYPE_is_number(P1->type))
	{
		if (OBJECT_class(P2->_object.object)->operators)
			return OP_FLOAT_OBJECT;
	}
	else if (TYPE_is_number(P2->type))
	{
		if (OBJECT_class(P1->_object.object)->operators)
			return OP_OBJECT_FLOAT;
	}
	else
	{
		CLASS *class1 = OBJECT_class(P1->_object.object);
		CLASS *class2 = OBJECT_class(P2->_object.object);
		
		if (class1->operators)
		{
			if (class1 == class2)
				return OP_OBJECT;
			
			if (class2->operators)
			{
				if (class1->operators->strength > class2->operators->strength)
					return OP_OBJECT_CONV;
				else
					return OP_CONV_OBJECT;
			}
		}
	}
	
	return 0;
}

static void operator_object_float(VALUE *P1, VALUE *P2, uchar op)
{
	void *(*func)(void *, double) = (void *(*)(void *, double))((void **)(OBJECT_class(P1->_object.object)->operators))[op];
	VALUE_conv_float(P2);
	void *result = (*func)(P1->_object.object, P2->_float.value);
	OBJECT_unref(P1->_object.object);
	P1->_object.object = result;
}

static void operator_float_object(VALUE *P1, VALUE *P2, uchar op)
{
	void *(*func)(void *, double) = (void *(*)(void *, double))((void **)(OBJECT_class(P2->_object.object)->operators))[op];
	VALUE_conv_float(P1);
	void *result = (*func)(P2->_object.object, P1->_float.value);
	P1->_object.class = P2->_object.class;
	OBJECT_unref(P2->_object.object);
	P1->_object.object = result;
}

static void operator_object(VALUE *P1, VALUE *P2, uchar op)
{
	void *(*func)(void *, void *) = (void *(*)(void *, void *))((void **)(OBJECT_class(P2->_object.object)->operators))[op];
	void *result = (*func)(P1->_object.object, P2->_object.object);
	OBJECT_unref(P1->_object.object);
	OBJECT_unref(P2->_object.object);
	P1->_object.object = result;
}

static void operator_object_conv(VALUE *P1, VALUE *P2, char op)
{
	VALUE_conv(P2, (TYPE)P1->_object.class);
	operator_object(P1, P2, op);
}

static void operator_conv_object(VALUE *P1, VALUE *P2, char op)
{
	VALUE_conv(P1, (TYPE)P2->_object.class);
	operator_object(P1, P2, op);
}

#define MANAGE_VARIANT(_func) \
({ \
	type = Max(P1->type, P2->type); \
	if (TYPE_is_void(P1->type) || TYPE_is_void(P2->type)) \
		THROW(E_NRETURN); \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	VARIANT_undo(P1); \
	VARIANT_undo(P2); \
	\
	if (TYPE_is_string(P1->type)) \
		VALUE_conv_float(P1); \
	\
	if (TYPE_is_string(P2->type)) \
		VALUE_conv_float(P2); \
	\
	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type)) \
		type = T_NULL; \
	else \
		type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		(_func)(code | type); \
		VALUE_conv_variant(P1); \
		return; \
	} \
})

#define MANAGE_VARIANT_POINTER(_func) \
({ \
	type = Max(P1->type, P2->type); \
	if (TYPE_is_void(P1->type) || TYPE_is_void(P2->type)) \
		THROW(E_NRETURN); \
	\
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	VARIANT_undo(P1); \
	VARIANT_undo(P2); \
	\
	if (TYPE_is_string(P1->type)) \
		VALUE_conv_float(P1); \
	\
	if (TYPE_is_string(P2->type)) \
		VALUE_conv_float(P2); \
	\
	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type)) \
		type = T_NULL; \
	else \
		type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)) \
	{ \
		(_func)(code | type); \
		VALUE_conv_variant(P1); \
		return; \
	} \
})


#define MANAGE_VARIANT_POINTER_OBJECT(_func) \
({ \
	type = Max(P1->type, P2->type); \
	if (TYPE_is_void(P1->type) || TYPE_is_void(P2->type)) \
		THROW(E_NRETURN); \
	\
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	VARIANT_undo(P1); \
	VARIANT_undo(P2); \
	\
	if (TYPE_is_string(P1->type)) \
		VALUE_conv_float(P1); \
	\
	if (TYPE_is_string(P2->type)) \
		VALUE_conv_float(P2); \
	\
	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type)) \
		type = T_NULL; \
	else \
		type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)) \
	{ \
		(_func)(code | type); \
		VALUE_conv_variant(P1); \
		return; \
	} \
	\
	if (TYPE_is_object(type)) \
	{ \
		type = check_operators(P1, P2); \
		if (type) \
		{ \
			*PC |= type; \
			goto *jump[type]; \
		} \
	} \
})


STATIC_SUBR void _SUBR_add(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, 
		&&__DATE, NULL, NULL, &&__POINTER,
		&&__OBJECT_FLOAT, &&__FLOAT_OBJECT, &&__OBJECT_CONV, &&__CONV_OBJECT, &&__OBJECT
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:

	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value | P2->_integer.value; goto __END;

__BYTE:

	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value + P2->_integer.value); goto __END;

__SHORT:

	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value + P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value += P2->_integer.value; goto __END;

__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	P1->_long.value += P2->_long.value; goto __END;

__SINGLE:

	VALUE_conv(P1, T_SINGLE);
	VALUE_conv(P2, T_SINGLE);

	P1->_single.value += P2->_single.value; goto __END;

__DATE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	P1->_float.value += P2->_float.value;
	//fprintf(stderr, "+: %.24g\n", P1->_float.value);
	goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	P1->_pointer.value += (intptr_t)P2->_pointer.value; goto __END;

__OBJECT_FLOAT:
	
	operator_object_float(P1, P2, CO_ADDF);
	goto __END;

__FLOAT_OBJECT:

	operator_float_object(P1, P2, CO_ADDF);
	goto __END;
	
__OBJECT_CONV:

	operator_object_conv(P1, P2, CO_ADDF);
	goto __END;

__CONV_OBJECT:

	operator_conv_object(P1, P2, CO_ADDF);
	goto __END;
	
__OBJECT:

	operator_object(P1, P2, CO_ADDF);
	goto __END;
	
__VARIANT:

	MANAGE_VARIANT_POINTER_OBJECT(_SUBR_add);
	goto __ERROR;
	
__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

	SP--;
}

STATIC_SUBR void _SUBR_sub(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE, NULL, NULL, &&__POINTER
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:

	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value ^ P2->_integer.value; goto __END;

__BYTE:

	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value - P2->_integer.value); goto __END;

__SHORT:

	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value - P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value -= P2->_integer.value; goto __END;

__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	P1->_long.value -= P2->_long.value; goto __END;

__SINGLE:

	VALUE_conv(P1, T_SINGLE);
	VALUE_conv(P2, T_SINGLE);

	P1->_single.value -= P2->_single.value; goto __END;

__DATE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	P1->_float.value -= P2->_float.value; goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	P1->_pointer.value -= (intptr_t)P2->_pointer.value; goto __END;

__VARIANT:

	MANAGE_VARIANT_POINTER(_SUBR_sub);
	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

	SP--;
}

STATIC_SUBR void _SUBR_mul(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:

	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value & P2->_integer.value; goto __END;

__BYTE:

	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value * P2->_integer.value); goto __END;

__SHORT:

	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value * P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value *= P2->_integer.value; goto __END;

__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	P1->_long.value *= P2->_long.value; goto __END;

__SINGLE:

	VALUE_conv(P1, T_SINGLE);
	VALUE_conv(P2, T_SINGLE);

	P1->_single.value *= P2->_single.value; goto __END;

__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	P1->_float.value *= P2->_float.value;
	//fprintf(stderr, "*: %.24g\n", P1->_float.value);
	goto __END;

__VARIANT:

	MANAGE_VARIANT(_SUBR_mul);
	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

	SP--;
}

STATIC_SUBR void _SUBR_div(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:
__LONG:
__SINGLE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	P1->_float.value /= P2->_float.value;
	if (isfinite(P1->_float.value))
	{
		SP--;
		return;
	}

	THROW(E_ZERO);

__VARIANT:

	MANAGE_VARIANT(_SUBR_div);
	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));
}

STATIC_SUBR void _SUBR_compi(ushort code)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL, &&__OBJECT
		};

	static void *test[] = { &&__GT, &&__LE, &&__LT, &&__GE };

	char NO_WARNING(result);
	VALUE *P1;
	VALUE *P2;
	TYPE type;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x1F;
	goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	result = P1->_integer.value > P2->_integer.value ? 1 : P1->_integer.value < P2->_integer.value ? -1 : 0;
	goto __END;

__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	result = P1->_long.value > P2->_long.value ? 1 : P1->_long.value < P2->_long.value ? -1 : 0;
	goto __END;

__DATE:

	VALUE_conv(P1, T_DATE);
	VALUE_conv(P2, T_DATE);

	result = DATE_comp_value(P1, P2);
	goto __END;

__NULL:
__STRING:

	VALUE_conv_string(P1);
	VALUE_conv_string(P2);

	result = STRING_compare(P1->_string.addr + P1->_string.start, P1->_string.len, P2->_string.addr + P2->_string.start, P2->_string.len);

	RELEASE_STRING(P1);
	RELEASE_STRING(P2);
	goto __END;

__SINGLE:

	VALUE_conv(P1, T_SINGLE);
	VALUE_conv(P2, T_SINGLE);

	result = P1->_single.value > P2->_single.value ? 1 : P1->_single.value < P2->_single.value ? -1 : 0;
	goto __END;

__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	result = P1->_float.value > P2->_float.value ? 1 : P1->_float.value < P2->_float.value ? -1 : 0;
	goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	result = P1->_pointer.value > P2->_pointer.value ? 1 : P1->_pointer.value < P2->_pointer.value ? -1 : 0;
	goto __END;

__OBJECT:

	result = OBJECT_comp_value(P1, P2);
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __END_RELEASE;

__VARIANT:

	{
		bool variant = FALSE;

		if (TYPE_is_variant(P1->type))
		{
			VARIANT_undo(P1);
			variant = TRUE;
		}

		if (TYPE_is_variant(P2->type))
		{
			VARIANT_undo(P2);
			variant = TRUE;
		}

		type = Max(P1->type, P2->type);

		if (type == T_NULL || TYPE_is_string(type))
		{
			TYPE typem = Min(P1->type, P2->type);
			if (!TYPE_is_string(typem))
				THROW(E_TYPE, TYPE_get_name(typem), TYPE_get_name(type));
		}
		else if (TYPE_is_object(type))
			goto __ERROR;
		else if (TYPE_is_void(type))
			THROW(E_NRETURN);

		if (!variant)
			*PC |= type;

		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number, Date or String", TYPE_get_name(type));

__END_RELEASE:

	RELEASE(P1);
	RELEASE(P2);

__END:

	P1->type = T_BOOLEAN;
	SP--;

	goto *test[(code >> 8) - (C_GT >> 8)];

__GT:
	P1->_boolean.value = result > 0 ? -1 : 0;
	return;

__GE:
	P1->_boolean.value = result >= 0 ? -1 : 0;
	return;

__LT:
	P1->_boolean.value = result < 0 ? -1 : 0;
	return;

__LE:
	P1->_boolean.value = result <= 0 ? -1 : 0;
	return;
}
