/***************************************************************************

	gbx_subr_math.c

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

#define __GBX_SUBR_MATH_C

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif

#include "gb_common.h"
#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_math.h"


#define ABS(x) ((x) < 0 ? (-x) : (x))

#define SMT_NAME    SUBR_quo
#define SMT_TYPE    3
#define SMT_OP      /

#include "gbx_subr_math_temp.h"


#define SMT_NAME    SUBR_rem
#define SMT_TYPE    3
#define SMT_OP      %

#include "gbx_subr_math_temp.h"

void SUBR_pi(ushort code)
{
	SUBR_ENTER();

	if (NPARAM == 0)
	{
		SP->type = T_FLOAT;
		SP->_float.value = M_PI;
		SP++;
	}
	else
	{
		VALUE_conv_float(PARAM);
		PARAM->_float.value = M_PI * PARAM->_float.value;
	}
}


void SUBR_randomize(ushort code)
{
	SUBR_ENTER();

	if (NPARAM == 0)
		randomize(FALSE, 0);
	else
		randomize(TRUE, (uint)SUBR_get_integer(PARAM));

	RETURN->type = T_VOID;

	SUBR_LEAVE();
}


void SUBR_rnd(ushort code)
{
	double min = 0.0, max = 1.0;

	SUBR_ENTER();

	if (NPARAM >= 1)
	{
		VALUE_conv_float(&PARAM[0]);
		max = PARAM->_float.value;
	}

	if (NPARAM == 2)
	{
		min = max;
		VALUE_conv_float(&PARAM[1]);
		max = PARAM[1]._float.value;
	}

	RETURN->type = T_FLOAT;
	RETURN->_float.value = (rnd() * (max - min)) + min;

	SUBR_LEAVE();
}


void SUBR_round(ushort code)
{
	int val = 0;
	double power;

	SUBR_ENTER();

	if (NPARAM == 2)
		val = SUBR_get_integer(&PARAM[1]);

	power = pow(10, val);

	VALUE_conv_float(&PARAM[0]);

	RETURN->type = T_FLOAT;
	/*RETURN->_float.value = rint(PARAM->_float.value / power) * power;*/
	RETURN->_float.value = floor(PARAM->_float.value / power + 0.5) * power;

	SUBR_LEAVE();
}


void SUBR_math(ushort code)
{
	static void *jump[] = {
		NULL, &&__FRAC, &&__LOG, &&__EXP, &&__SQRT, &&__SIN, &&__COS, &&__TAN, &&__ATAN, &&__ASIN, &&__ACOS,
		&&__DEG, &&__RAD, &&__LOG10, &&__SINH, &&__COSH, &&__TANH, &&__ASINH, &&__ACOSH, &&__ATANH,
		&&__EXP2, &&__EXP10, &&__LOG2, &&__CBRT, &&__EXPM1, &&__LOG1P, &&__FLOOR, &&__CEIL
	};

	SUBR_ENTER_PARAM(1);

	VALUE_conv_float(PARAM);
	goto *jump[code & 0x1F];

__FRAC: PARAM->_float.value = frac(PARAM->_float.value); goto __END;
__LOG: PARAM->_float.value = __builtin_log(PARAM->_float.value); goto __END;
__EXP: PARAM->_float.value = __builtin_exp(PARAM->_float.value); goto __END;
__SQRT: PARAM->_float.value = __builtin_sqrt(PARAM->_float.value); goto __END;
__SIN: PARAM->_float.value = __builtin_sin(PARAM->_float.value); goto __END;
__COS: PARAM->_float.value = __builtin_cos(PARAM->_float.value); goto __END;
__TAN: PARAM->_float.value = __builtin_tan(PARAM->_float.value); goto __END;
__ATAN: PARAM->_float.value = __builtin_atan(PARAM->_float.value); goto __END;
__ASIN: PARAM->_float.value = __builtin_asin(PARAM->_float.value); goto __END;
__ACOS: PARAM->_float.value = __builtin_acos(PARAM->_float.value); goto __END;
__DEG: PARAM->_float.value = deg(PARAM->_float.value); goto __END;
__RAD: PARAM->_float.value = rad(PARAM->_float.value); goto __END;
__LOG10: PARAM->_float.value = log10(PARAM->_float.value); goto __END;
__SINH: PARAM->_float.value = __builtin_sinh(PARAM->_float.value); goto __END;
__COSH: PARAM->_float.value = __builtin_cosh(PARAM->_float.value); goto __END;
__TANH: PARAM->_float.value = __builtin_tanh(PARAM->_float.value); goto __END;
__ASINH: PARAM->_float.value = __builtin_asinh(PARAM->_float.value); goto __END;
__ACOSH: PARAM->_float.value = __builtin_acosh(PARAM->_float.value); goto __END;
__ATANH: PARAM->_float.value = __builtin_atanh(PARAM->_float.value); goto __END;
__EXP2: PARAM->_float.value = __builtin_exp2(PARAM->_float.value); goto __END;
#if defined(OS_FREEBSD) || defined(__clang__)
	__EXP10: PARAM->_float.value = exp10(PARAM->_float.value); goto __END;
#else
	__EXP10: PARAM->_float.value = __builtin_exp10(PARAM->_float.value); goto __END;
#endif
__LOG2: PARAM->_float.value = __builtin_log2(PARAM->_float.value); goto __END;
__CBRT: PARAM->_float.value = __builtin_cbrt(PARAM->_float.value); goto __END;
__EXPM1: PARAM->_float.value = __builtin_expm1(PARAM->_float.value); goto __END;
__LOG1P: PARAM->_float.value = __builtin_log1p(PARAM->_float.value); goto __END;
__FLOOR: PARAM->_float.value = __builtin_floor(PARAM->_float.value); goto __END;
__CEIL: PARAM->_float.value = __builtin_ceil(PARAM->_float.value); goto __END;

__END:

	//fprintf(stderr, "m: %.24g\n", PARAM->_float.value);
	if (!isfinite(PARAM->_float.value))
		THROW(E_MATH);
}


void SUBR_math2(ushort code)
{
	static void *jump[] = { NULL, &&__ATAN2, &&__ANG, &&__HYPOT };

	SUBR_ENTER_PARAM(2);

	VALUE_conv_float(&PARAM[0]);
	VALUE_conv_float(&PARAM[1]);

	goto *jump[code & 0x1F];
	
__ATAN2: PARAM->_float.value = __builtin_atan2(PARAM[0]._float.value, PARAM[1]._float.value); goto __END;
__ANG: PARAM->_float.value = __builtin_atan2(PARAM[1]._float.value, PARAM[0]._float.value); goto __END;
__HYPOT: PARAM->_float.value = sqrt(PARAM[0]._float.value * PARAM[0]._float.value + PARAM[1]._float.value * PARAM[1]._float.value); goto __END;

__END:

	if (!finite(PARAM->_float.value))
		THROW(E_MATH);

	SP--;
}


void SUBR_pow(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__NUMBER_INTEGER, &&__NUMBER_FLOAT, &&__OBJECT_FLOAT, &&__OBJECT_OTHER, &&__OBJECT_OBJECT
		};
	
	VALUE *P1, *P2;
	uchar type;
	bool variant = FALSE;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__VARIANT:

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

	if (TYPE_is_number(P1->type) && TYPE_is_number(P2->type))
	{
		if (TYPE_is_integer(P2->type))
			type = 1;
		else
			type = 2;
	}
	else
	{
		type = EXEC_check_operator(P1, P2, CO_POW);
		
		if (type == OP_OBJECT_FLOAT)
			type = 3;
		else if (type == OP_OBJECT_OTHER)
			type = 4;
		else if (type == OP_OBJECT_OBJECT)
			type = 5;
		else
			THROW(E_TYPE, "Number", TYPE_get_name(P2->type));
	}
	
	if (!variant)
	{
		if (P1->type != T_OBJECT && P2->type != T_OBJECT)
			*PC |= type;
		goto *jump[type];
	}
	else
	{
		SUBR_pow(type);
		VALUE_conv_variant(P1);
		return;
	}

__NUMBER_INTEGER:
	
	{
		static void *ni_jump[] = { &&__M4, &&__M3, &&__M2, &&__M1, &&__P0, &&__END, &&__P2, &&__P3, &&__P4 };
		int val = P2->_integer.value;
		
		VALUE_conv_float(P1);
		
		if (val >= -4 && val <= 4)
			goto *ni_jump[val + 4];
		else
			goto __NUMBER_FLOAT;
		
		__P0: P1->_float.value = 1.0; goto __END;
		__P2: P1->_float.value *= P1->_float.value; goto __END_NUMBER;
		__P3: P1->_float.value *= P1->_float.value * P1->_float.value; goto __END_NUMBER;
		__P4: P1->_float.value = P1->_float.value * P1->_float.value * P1->_float.value * P1->_float.value; goto __END_NUMBER;
		__M1: P1->_float.value = 1.0 / P1->_float.value; goto __END_NUMBER;
		__M2: P1->_float.value = 1.0 / P1->_float.value / P1->_float.value; goto __END_NUMBER;
		__M3: P1->_float.value = 1.0 / P1->_float.value / P1->_float.value / P1->_float.value; goto __END_NUMBER;
		__M4: P1->_float.value = 1.0 / P1->_float.value / P1->_float.value / P1->_float.value / P1->_float.value; goto __END_NUMBER;
	}

__NUMBER_FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);
	P1->_float.value = pow(P1->_float.value, P2->_float.value);
	goto __END;

__OBJECT_FLOAT:

	EXEC_operator(OP_OBJECT_FLOAT, CO_POWF, P1, P2);
	goto __END;

__OBJECT_OTHER:

	EXEC_operator(OP_OBJECT_OTHER, CO_POWO, P1, P2);
	goto __END;

__OBJECT_OBJECT:

	EXEC_operator(OP_OBJECT_OBJECT, CO_POW, P1, P2);
	goto __END;

__END_NUMBER:
	
	if (!finite(P1->_float.value))
		THROW(E_MATH);
	
__END:

	SP--;
}


void SUBR_not(ushort code)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL,
		&&__OBJECT
		};

	VALUE *P1;
	void *jump_end;
	TYPE type = code & 0x1F;
	bool test;

	P1 = SP - 1;
	jump_end = &&__END;
	goto *jump[type];

__BOOLEAN:

	P1->_integer.value = P1->_integer.value ? 0 : (-1);
	goto *jump_end;

__BYTE:

	P1->_integer.value = (unsigned char)~P1->_integer.value;
	goto *jump_end;

__SHORT:

	P1->_integer.value = (short)~P1->_integer.value;
	goto *jump_end;

__INTEGER:

	P1->_integer.value = ~P1->_integer.value;
	goto *jump_end;

__LONG:

	P1->_long.value = ~P1->_long.value;
	goto *jump_end;

__SINGLE:
__FLOAT:
__DATE:
	goto __ERROR;

__STRING:
__OBJECT:
__NULL:

	test = VALUE_is_null(P1);
	RELEASE(P1);

	P1->_integer.value =  test ? (-1) : 0;
	P1->type = T_BOOLEAN;
	goto *jump_end;

__VARIANT:

	type = P1->type;

	if (TYPE_is_variant(type))
	{
		type = P1->_variant.vtype;
		jump_end = &&__VARIANT_END;
		VARIANT_undo(P1);
	}
	else if (TYPE_is_object(type))
		*PC |= T_OBJECT;
	else if (type)
		*PC |= type;
	else
		goto __ERROR;

	if (TYPE_is_object(type))
		goto __OBJECT;
	else
		goto *jump[type];

__ERROR:

	THROW(E_TYPE, "Number, String or Object", TYPE_get_name(type));

__VARIANT_END:

	VALUE_conv_variant(P1);

__END:
	return;
}

void SUBR_and_(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__ERROR, &&__ERROR, &&__ERROR
		};

	TYPE type;
	VALUE *P1, *P2;
	void *jump_end;
	short op;

	P1 = SP - 2;
	P2 = P1 + 1;

	jump_end = &&__END;
	type = code & 0x0F;
	op = (code >> 8)  - (C_AND >> 8);
	goto *jump[type];

__BYTE:

	P1->type = type;

	{
		static void *exec[] = { &&__AND_C, && __OR_C, &&__XOR_C };
		goto *exec[op];

		__AND_C: P1->_integer.value = (unsigned char)(P1->_integer.value & P2->_integer.value); goto *jump_end;
		__OR_C: P1->_integer.value = (unsigned char)(P1->_integer.value | P2->_integer.value); goto *jump_end;
		__XOR_C: P1->_integer.value = (unsigned char)(P1->_integer.value ^ P2->_integer.value); goto *jump_end;
	}

	goto *jump_end;

__SHORT:

	P1->type = type;

	{
		static void *exec[] = { &&__AND_H, && __OR_H, &&__XOR_H };
		goto *exec[op];

		__AND_H: P1->_integer.value = (short)(P1->_integer.value & P2->_integer.value); goto *jump_end;
		__OR_H: P1->_integer.value = (short)(P1->_integer.value | P2->_integer.value); goto *jump_end;
		__XOR_H: P1->_integer.value = (short)(P1->_integer.value ^ P2->_integer.value); goto *jump_end;
	}

	goto *jump_end;

__BOOLEAN:
__INTEGER:

	P1->type = type;

	{
		static void *exec[] = { &&__AND_I, && __OR_I, &&__XOR_I };
		goto *exec[op];

		__AND_I: P1->_integer.value &= P2->_integer.value; goto *jump_end;
		__OR_I: P1->_integer.value |= P2->_integer.value; goto *jump_end;
		__XOR_I: P1->_integer.value ^= P2->_integer.value; goto *jump_end;
	}

	goto *jump_end;

__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	P1->type = type;

	{
		static void *exec[] = { &&__AND_L, && __OR_L, &&__XOR_L };
		goto *exec[op];

		__AND_L: P1->_long.value &= P2->_long.value; goto *jump_end;
		__OR_L: P1->_long.value |= P2->_long.value; goto *jump_end;
		__XOR_L: P1->_long.value ^= P2->_long.value; goto *jump_end;
	}

__VARIANT:

	type = Max(P1->type, P2->type);

	if (TYPE_is_number_date(type))
	{
		*PC |= type;
		goto *jump[type];
	}

	if (TYPE_is_variant(P1->type))
		VARIANT_undo(P1);

	if (TYPE_is_variant(P2->type))
		VARIANT_undo(P2);

	if (TYPE_is_string(P1->type))
		VALUE_convert_boolean(P1);

	if (TYPE_is_string(P2->type))
		VALUE_convert_boolean(P2);

	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type))
		type = T_NULL;
	else
		type = Max(P1->type, P2->type);

	if (TYPE_is_number_date(type))
	{
		jump_end = &&__VARIANT_END;
		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));

__VARIANT_END:

	VALUE_conv_variant(P1);

__END:

	SP--;
}

#define MANAGE_VARIANT(_func) \
({ \
	type = P1->type; \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	if (TYPE_is_variant(type)) \
	{ \
		type = P1->_variant.vtype; \
		if (TYPE_is_number_date(type)) \
		{ \
			VARIANT_undo(P1); \
			(_func)(code | type); \
			VALUE_conv_variant(P1); \
			return; \
		} \
	} \
})

#define MANAGE_VARIANT_OBJECT(_func, _op) \
({ \
	type = P1->type; \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	if (EXEC_check_operator_single(P1, _op)) \
	{ \
		if (P1->type != T_OBJECT) \
			*PC |= T_DATE + 1; \
		goto *jump[T_DATE + 1]; \
	} \
	\
	if (TYPE_is_variant(type)) \
	{ \
		VARIANT_undo(P1); \
		type = P1->type; \
		if (TYPE_is_number_date(type)) \
		{ \
			(_func)(code | type); \
			VALUE_conv_variant(P1); \
			return; \
		} \
		if (EXEC_check_operator_single(P1, _op)) \
		{ \
			(_func)(T_DATE + 1); \
			VALUE_conv_variant(P1); \
			return; \
		} \
	} \
})

#define MANAGE_VARIANT_OBJECT_2(_func, _op, _op2) \
({ \
	type = P1->type; \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		*PC |= type; \
		goto *jump[type]; \
	} \
	\
	if (EXEC_check_operator_single(P1, _op)) \
	{ \
		if (P1->type != T_OBJECT) \
			*PC |= T_DATE + 1; \
		goto *jump[T_DATE + 1]; \
	} \
	if (EXEC_check_operator_single(P1, _op2)) \
	{ \
		if (P1->type != T_OBJECT) \
			*PC |= T_DATE + 2; \
		goto *jump[T_DATE + 2]; \
	} \
	\
	if (TYPE_is_variant(type)) \
	{ \
		VARIANT_undo(P1); \
		type = P1->type; \
		if (TYPE_is_number_date(type)) \
		{ \
			(_func)(code | type); \
			VALUE_conv_variant(P1); \
			return; \
		} \
		if (EXEC_check_operator_single(P1, _op)) \
		{ \
			(_func)(T_DATE + 1); \
			VALUE_conv_variant(P1); \
			return; \
		} \
		if (EXEC_check_operator_single(P1, _op2)) \
		{ \
			(_func)(T_DATE + 2); \
			VALUE_conv_variant(P1); \
			return; \
		} \
	} \
})


void SUBR_sgn(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR, &&__OBJECT
		};

	VALUE *P1;
	TYPE type;

	P1 = SP - 1;
	type = code & 0x0F;
	goto *jump[type];

__INTEGER:  P1->_integer.value = lsgn(P1->_integer.value); goto __END;

__LONG: P1->_integer.value = llsgn(P1->_long.value); goto __END;

__SINGLE: P1->_integer.value = (P1->_single.value > 0) ? 1 : ((P1->_single.value < 0) ? (-1) : 0); goto __END;

__FLOAT: P1->_integer.value = fsgn(P1->_float.value); goto __END;

__OBJECT:

	EXEC_operator_object_sgn(P1);
	return;

__VARIANT:

	type = P1->type;

	if (TYPE_is_number(type))
	{
		*PC |= type;
		goto *jump[type];
	}

	if (EXEC_check_operator_single(P1, CO_SGN))
	{
		if (P1->type != T_OBJECT)
			*PC |= T_DATE + 1;
		goto *jump[T_DATE + 1];
	}

	if (TYPE_is_variant(type))
	{
		type = P1->_variant.vtype;
		if (TYPE_is_number(type))
		{
			VARIANT_undo(P1);
			goto *jump[type];
		}
	}

	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));

__END:

	P1->type = T_INTEGER;
}


void SUBR_neg(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR, &&__OBJECT
		};

	VALUE *P1;
	TYPE type;

	P1 = SP - 1;

	type = code & 0x0F;

	goto *jump[type];

__BOOLEAN:

	return;

__BYTE:
	
	P1->_integer.value = (unsigned char)(-P1->_integer.value); return;

__SHORT:

	P1->_integer.value = (short)(-P1->_integer.value); return;

__INTEGER:

	P1->_integer.value = (-P1->_integer.value); return;

__LONG:

	P1->_long.value = (-P1->_long.value); return;

__SINGLE:

	P1->_single.value = (-P1->_single.value); return;

__FLOAT:
	
	P1->_float.value = (-P1->_float.value); return;

__OBJECT:
	
	EXEC_operator_object_single(CO_NEG, P1);
	return;
	
__VARIANT:

	MANAGE_VARIANT_OBJECT(SUBR_neg, CO_NEG);

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));
}


void SUBR_abs(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR, &&__OBJECT, &&__OBJECT_FLOAT
		};

	VALUE *P1;
	TYPE type;

	P1 = SP - 1;

	type = code & 0x0F;

	goto *jump[type];

__BOOLEAN:

	P1->type = T_INTEGER;
	goto __INTEGER;

__BYTE:
	
	P1->_integer.value = (unsigned char)ABS(-P1->_integer.value); return;

__SHORT:

	P1->_integer.value = (short)ABS(P1->_integer.value); return;

__INTEGER:

	P1->_integer.value = ABS(P1->_integer.value); return;

__LONG:

	P1->_long.value = ABS(P1->_long.value); return;

__SINGLE:

	P1->_single.value = fabsf(P1->_single.value); return;

__FLOAT:
	
	P1->_float.value = fabs(P1->_float.value); return;

__OBJECT:

	EXEC_operator_object_single(CO_ABS, P1);
	return;

__OBJECT_FLOAT:

	EXEC_operator_object_fabs(P1);
	return;

__VARIANT:

	MANAGE_VARIANT_OBJECT_2(SUBR_abs, CO_ABS, CO_FABS);

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));
}


void SUBR_int(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR
		};

	VALUE *P1;
	TYPE type;

	P1 = SP - 1;

	type = code & 0x0F;

	goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:
__LONG:

	return;

__SINGLE:

	P1->_single.value = floorf(P1->_single.value); return;

__FLOAT:
	
	P1->_float.value = floor(P1->_float.value); return;

__VARIANT:

	MANAGE_VARIANT(SUBR_int);

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));
}


void SUBR_fix(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR
		};

	VALUE *P1;
	TYPE type;

	P1 = SP - 1;

	type = code & 0x0F;

	goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:
__LONG:

	return;

__SINGLE:

	P1->_single.value = fixf(P1->_single.value); return;

__FLOAT:
	
	P1->_float.value = fix(P1->_float.value); return;

__VARIANT:

	MANAGE_VARIANT(SUBR_fix);

__ERROR:

	THROW(E_TYPE, "Number", TYPE_get_name(type));
}
