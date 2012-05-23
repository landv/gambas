/***************************************************************************

	gbx_subr_common.h

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

#ifdef STATIC_SUBR
#undef STATIC_SUBR
#define STATIC_SUBR static
#endif

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


STATIC_SUBR void _SUBR_add(ushort code)
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

__VARIANT:

	MANAGE_VARIANT_POINTER(_SUBR_add);
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
