/***************************************************************************

  gbx_subr_test.c

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

#include "gb_common.h"
#include <math.h>

#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_date.h"
#include "gbx_object.h"
#include "gbx_math.h"
#include "gbx_compare.h"

#define STT_NAME   SUBR_case
#define STT_TEST   ==
#define STT_CASE

#include "gbx_subr_test_temp.h"


void SUBR_bit(ushort code)
{
  static void *jump[16] = {
    &&__ERROR, &&__BCLR, &&__BSET, &&__BTST, &&__BCHG, &&__ASL, &&__ASR, &&__ROL,
    &&__ROR, &&__LSL, &&__LSR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR
    };
    
	static int nbits[6] = { 0, 0, 8, 16, 32, 64 };

  int64_t val;
  int bit;
  TYPE type;
  int n;
  bool variant;

  SUBR_ENTER_PARAM(2);

  type = PARAM->type;

	variant = TYPE_is_variant(type);
	if (variant)
		type = PARAM->_variant.vtype;
	
	if (type <= T_BOOLEAN || type > T_LONG)
	  THROW(E_TYPE, "Number", TYPE_get_name(type));
	
  VALUE_conv(PARAM, T_LONG);
  val = PARAM->_long.value;

	n = nbits[type];

  VALUE_conv_integer(&PARAM[1]);
  bit = PARAM[1]._integer.value;

  if ((bit < 0) || (bit >= n))
    THROW(E_ARG);

  RETURN->type = type;

	goto *jump[code & 0xF];

__BCLR:

	val &= ~(1ULL << bit);
	goto __END;

__BSET:

	val |= (1ULL << bit);
	goto __END;

__BTST:

	RETURN->type = T_BOOLEAN;
	RETURN->_boolean.value = (val & (1ULL << bit)) ? (-1) : 0;
  goto __LEAVE;

__BCHG:

	val ^= (1ULL << bit);
  goto __END;

__ASL:

	{
	  static void *asl_jump[6] = { &&__ERROR, &&__ERROR, &&__ASL_BYTE, &&__ASL_SHORT, &&__ASL_INTEGER, &&__ASL_LONG };

		goto *asl_jump[type];

	__ASL_BYTE:
		val = ((unsigned char)val << bit);
		goto __END_BYTE;

	__ASL_SHORT:
		val = (((short)val << bit) & 0x7FFF) | (((short)val) & 0x8000);
		goto __END_SHORT;

	__ASL_INTEGER:
		val = (((int)val << bit) & 0x7FFFFFFF) | (((int)val) & 0x80000000);
		goto __END_INTEGER;

	__ASL_LONG:
		val = ((val << bit) & 0x7FFFFFFFFFFFFFFFLL) | (val & 0x8000000000000000LL);
		goto __END_LONG;
	}
	
__ASR:

	{
	  static void *asr_jump[6] = { &&__ERROR, &&__ERROR, &&__ASR_BYTE, &&__ASR_SHORT, &&__ASR_INTEGER, &&__ASR_LONG };

		goto *asr_jump[type];

	__ASR_BYTE:
		val = ((unsigned char)val >> bit);
		goto __END_BYTE;

	__ASR_SHORT:
		val = (((short)val >> bit) & 0x7FFF) | (((short)val) & 0x8000);
		goto __END_SHORT;

	__ASR_INTEGER:
		val = (((int)val >> bit) & 0x7FFFFFFF) | (((int)val) & 0x80000000);
		goto __END_INTEGER;

	__ASR_LONG:
		val = ((val >> bit) & 0x7FFFFFFFFFFFFFFFLL) | (val & 0x8000000000000000LL);
		goto __END_LONG;
	}
	
__ROL:

	{
	  static void *rol_jump[6] = { &&__ERROR, &&__ERROR, &&__ROL_BYTE, &&__ROL_SHORT, &&__ROL_INTEGER, &&__ROL_LONG };
	  
	  goto *rol_jump[type];
	  
	__ROL_BYTE:
		val = (val << bit) | (val >> (8 - bit));
		goto __END_BYTE;
	
	__ROL_SHORT:
		val = ((ushort)val << bit) | ((ushort)val >> (16 - bit));
		goto __END_SHORT;
	
	__ROL_INTEGER:
		val = ((uint)val << bit) | ((uint)val >> (32 - bit));
		goto __END_INTEGER;
	
	__ROL_LONG:
		val = ((uint64_t)val << bit) | ((uint64_t)val >> (64 - bit));
		goto __END_LONG;
	}

__ROR:

	{
	  static void *ror_jump[6] = { &&__ERROR, &&__ERROR, &&__ROR_BYTE, &&__ROR_SHORT, &&__ROR_INTEGER, &&__ROR_LONG };
	  
	  goto *ror_jump[type];
	  
	__ROR_BYTE:
		val = (val >> bit) | (val << (8 - bit));
		goto __END_BYTE;
	
	__ROR_SHORT:
		val = ((ushort)val >> bit) | ((ushort)val << (16 - bit));
		goto __END_SHORT;
	
	__ROR_INTEGER:
		val = ((uint)val >> bit) | ((uint)val << (32 - bit));
		goto __END_INTEGER;
	
	__ROR_LONG:
		val = ((uint64_t)val >> bit) | ((uint64_t)val << (64 - bit));
		goto __END_LONG;
	}

__LSL:

	{
	  static void *lsl_jump[6] = { &&__ERROR, &&__ERROR, &&__LSL_BYTE, &&__LSL_SHORT, &&__LSL_INTEGER, &&__LSL_LONG };

		goto *lsl_jump[type];

	__LSL_BYTE:
		val = ((unsigned char)val << bit);
		goto __END_BYTE;

	__LSL_SHORT:
		val = ((unsigned short)val << bit);
		goto __END_SHORT;

	__LSL_INTEGER:
		val = ((unsigned int)val << bit);
		goto __END_INTEGER;

	__LSL_LONG:
		val = ((uint64_t)val << bit);
		goto __END_LONG;
	}
	
__LSR:

	{
	  static void *lsr_jump[6] = { &&__ERROR, &&__ERROR, &&__LSR_BYTE, &&__LSR_SHORT, &&__LSR_INTEGER, &&__LSR_LONG };

		goto *lsr_jump[type];

	__LSR_BYTE:
		val = ((unsigned char)val >> bit);
		goto __END_BYTE;

	__LSR_SHORT:
		val = ((unsigned short)val >> bit);
		goto __END_SHORT;

	__LSR_INTEGER:
		val = ((unsigned int)val >> bit);
		goto __END_INTEGER;

	__LSR_LONG:
		val = ((uint64_t)val >> bit);
		goto __END_LONG;
	}

__ERROR:

	THROW_ILLEGAL();

__END:

	{
  	static void *end_jump[6] = { &&__ERROR, &&__ERROR, &&__END_BYTE, &&__END_SHORT, &&__END_INTEGER, &&__END_LONG };
  	
  	goto *end_jump[type];
  	
	__END_BYTE:
		RETURN->_integer.value = (unsigned int)val & 0xFF;
		goto __END_VARIANT;
  	
	__END_SHORT:
		RETURN->_integer.value = (int)(short)val;
		goto __END_VARIANT;
	
	__END_INTEGER:
		RETURN->_integer.value = (int)val;
		goto __END_VARIANT;
  	
	__END_LONG:
		RETURN->_long.value = val;
		goto __END_VARIANT;
	}
	
__END_VARIANT:

	if (variant)
		VALUE_conv_variant(RETURN);

__LEAVE:

  SUBR_LEAVE();
}


void SUBR_if(ushort code)
{
	int i;
	unsigned char test;
	TYPE type;
	
  SUBR_ENTER_PARAM(3);

  VALUE_conv_boolean(PARAM);
	i = PARAM->_boolean.value ? 1 : 2;
	
	test = code & 0x1F;

	if (!test)
	{
		type = PARAM[1].type;
		if (PARAM[2].type == type && type <= T_VARIANT)
		{
			*PC |= 0x1F;
		}
		else
		{
			type = SUBR_check_good_type(&PARAM[1], 2);
			if (TYPE_is_object(type))
				type = T_OBJECT;
			*PC |= (unsigned char)type;
			
			VALUE_conv(&PARAM[i], type);
		}
	}
	else if (test != 0x1F)
	{
		VALUE_conv(&PARAM[i], (TYPE)test);
	}

	*PARAM = PARAM[i];
	RELEASE(&PARAM[3 - i]);
	SP -= 2;
}


void SUBR_choose(ushort code)
{
  int val;

  SUBR_ENTER();

  VALUE_conv_integer(PARAM);
  val = PARAM->_integer.value;

  if (val >= 1 && val < NPARAM)
  {
    VALUE_conv_variant(&PARAM[val]);
    *RETURN = PARAM[val];
  }
  else
  {
    RETURN->type = T_VARIANT;
    RETURN->_variant.vtype = T_NULL;
  }

  SUBR_LEAVE();
}


void SUBR_near(void)
{
  int result;

  SUBR_ENTER_PARAM(2);

  VALUE_conv_string(&PARAM[0]);
  VALUE_conv_string(&PARAM[1]);

  //result = STRING_comp_value_ignore_case(&PARAM[0], &PARAM[1]) ? -1 : 0;
  result = STRING_equal_ignore_case(PARAM[0]._string.addr + PARAM[0]._string.start, PARAM[0]._string.len, PARAM[1]._string.addr + PARAM[1]._string.start, PARAM[1]._string.len) ? -1 : 0;

  RELEASE_STRING(&PARAM[0]);
  RELEASE_STRING(&PARAM[1]);

  PARAM->type = T_BOOLEAN;
  PARAM->_boolean.value = result;

  SP--;
}

#if 0
void SUBR_comp(ushort code)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL, &&__OBJECT
		};

	//static void *test[] = { &&__EQ, &&__NE, &&__GT, &&__LE, &&__LT, &&__GE };

	char NO_WARNING(result);
	VALUE *P1;
	VALUE *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	goto *jump[code & 0x1F];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	result = P1->_integer.value == P2->_integer.value;
	goto __END;
	
__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	result = P1->_long.value == P2->_long.value;
	goto __END;

__DATE:

	VALUE_conv(P1, T_DATE);
	VALUE_conv(P2, T_DATE);

	result = DATE_comp_value(P1, P2) == 0;
	goto __END;

__NULL:

	if (P2->type == T_NULL)
	{
		result = VALUE_is_null(P1);
		goto __END_RELEASE;
	}
	else if (P1->type == T_NULL)
	{
		result = VALUE_is_null(P2);
		goto __END_RELEASE;
	}

__STRING:

	VALUE_conv_string(P1);
	VALUE_conv_string(P2);

	if (P1->_string.len != P2->_string.len)
		result = 0;
	else
		result = STRING_equal_same(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len);
	
	RELEASE_STRING(P1);
	RELEASE_STRING(P2);
	goto __END;

__SINGLE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	result = P1->_float.value == P2->_float.value;
	goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	result = P1->_pointer.value == P2->_pointer.value;
	goto __END;

__OBJECT:

	result = OBJECT_comp_value(P1, P2) == 0;
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __END_RELEASE;

__VARIANT:

	{
		bool variant = FALSE;
		TYPE type;
	
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

		if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
			type = T_OBJECT;
		else if (TYPE_is_object(type))
			THROW(E_TYPE, "Object", TYPE_get_name(Min(P1->type, P2->type)));

		if (!variant)
			*PC |= type;

		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number, Date or String", TYPE_get_name(code & 0x1F));

__END_RELEASE:

	RELEASE(P1);
	RELEASE(P2);

__END:

	P1->type = T_BOOLEAN;
	SP--;
	P1->_boolean.value = -result;
}

void SUBR_compn(ushort code)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL, &&__OBJECT
		};

	//static void *test[] = { &&__EQ, &&__NE, &&__GT, &&__LE, &&__LT, &&__GE };

	char NO_WARNING(result);
	VALUE *P1;
	VALUE *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	goto *jump[code & 0x1F];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	result = P1->_integer.value == P2->_integer.value;
	goto __END;
	
__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);

	result = P1->_long.value == P2->_long.value;
	goto __END;

__DATE:

	VALUE_conv(P1, T_DATE);
	VALUE_conv(P2, T_DATE);

	result = DATE_comp_value(P1, P2) == 0;
	goto __END;

__NULL:

	if (P2->type == T_NULL)
	{
		result = VALUE_is_null(P1);
		goto __END_RELEASE;
	}
	else if (P1->type == T_NULL)
	{
		result = VALUE_is_null(P2);
		goto __END_RELEASE;
	}

__STRING:

	VALUE_conv_string(P1);
	VALUE_conv_string(P2);

	if (P1->_string.len != P2->_string.len)
		result = 0;
	else
		result = STRING_equal_same(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len);
	
	RELEASE_STRING(P1);
	RELEASE_STRING(P2);
	goto __END;

__SINGLE:
__FLOAT:

	VALUE_conv_float(P1);
	VALUE_conv_float(P2);

	result = P1->_float.value == P2->_float.value;
	goto __END;

__POINTER:

	VALUE_conv(P1, T_POINTER);
	VALUE_conv(P2, T_POINTER);

	result = P1->_pointer.value == P2->_pointer.value;
	goto __END;

__OBJECT:

	result = OBJECT_comp_value(P1, P2) == 0;
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __END_RELEASE;

__VARIANT:

	{
		bool variant = FALSE;
		TYPE type;
	
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

		if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
			type = T_OBJECT;
		else if (TYPE_is_object(type))
			THROW(E_TYPE, "Object", TYPE_get_name(Min(P1->type, P2->type)));

		if (!variant)
			*PC |= type;

		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number, Date or String", TYPE_get_name(code & 0x1F));

__END_RELEASE:

	RELEASE(P1);
	RELEASE(P2);

__END:

	P1->type = T_BOOLEAN;
	SP--;

	P1->_boolean.value = result - 1; // ? 0 : -1;
}

#define sgn(_x) \
({ \
	int x = _x; \
	int minusOne = x >> 31; \
	unsigned int negateX = (unsigned int) -x; \
	int plusOne = (int)(negateX >> 31); \
  int result = minusOne | plusOne; \
  result; \
})

void SUBR_compi(ushort code)
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
#endif

void SUBR_strcomp(ushort code)
{
  int mode = GB_COMP_BINARY;
  char *s1, *s2;
  int l1, l2;
  int ret;

  SUBR_ENTER();

  VALUE_conv_string(&PARAM[0]);
  VALUE_conv_string(&PARAM[1]);

  if (NPARAM == 3)
    mode = SUBR_get_integer(&PARAM[2]);

	mode &= GB_COMP_TYPE_MASK;
	
	if (mode == GB_COMP_BINARY)
		ret = STRING_compare(PARAM[0]._string.addr + PARAM[0]._string.start, PARAM[0]._string.len, PARAM[1]._string.addr + PARAM[1]._string.start, PARAM[1]._string.len);
	else if (mode == GB_COMP_NOCASE)
		ret = STRING_compare_ignore_case(PARAM[0]._string.addr + PARAM[0]._string.start, PARAM[0]._string.len, PARAM[1]._string.addr + PARAM[1]._string.start, PARAM[1]._string.len);
	else
	{
		SUBR_get_string_len(&PARAM[0], &s1, &l1);
		SUBR_get_string_len(&PARAM[1], &s2, &l2);
		
		if (mode & GB_COMP_NATURAL)
			ret = COMPARE_string_natural(s1, l1, s2, l2, mode & GB_COMP_NOCASE);
		else if (mode & GB_COMP_LIKE)
			ret = COMPARE_string_like(s1, l1, s2, l2);
		else if (mode & GB_COMP_LANG)
			ret = COMPARE_string_lang(s1, l1, s2, l2, mode & GB_COMP_NOCASE, FALSE);
		else
			THROW(E_ARG);
	}
  
	RETURN->_integer.type = T_INTEGER;
	RETURN->_integer.value = ret;

  SUBR_LEAVE();
}



void SUBR_is(ushort code)
{
	VALUE *P1 = SP - 2;
	VALUE *P2 = SP - 1;
	void *object;
	CLASS *klass;
	bool res;

	VALUE_conv(P1, T_OBJECT);
	object = P1->_object.object;
	klass = P2->_class.class;

	if (!object)
		res = FALSE;
	else
		res = (OBJECT_class(object) == klass || CLASS_inherits(OBJECT_class(object), klass));

	OBJECT_UNREF(object);

	P1->type = T_BOOLEAN;
	P1->_boolean.value = -(res ^ (code & 1));
	SP--;
}


void SUBR_min_max(ushort code)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__DATE
    };

  TYPE type;
  VALUE *P1, *P2;
  void *jump_end;
  bool is_max;

  P1 = SP - 2;
  P2 = P1 + 1;

  jump_end = &&__END;
  type = code & 0x0F;
  is_max = ((code >> 8) == CODE_MAX);
  goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	P1->type = type;

	if (is_max)
	{
		if (P2->_integer.value > P1->_integer.value)
			P1->_integer.value = P2->_integer.value;
	}
	else
	{
		if (P2->_integer.value < P1->_integer.value)
			P1->_integer.value = P2->_integer.value;
	}
	
	goto *jump_end;

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	if (is_max)
	{
		if (P2->_long.value > P1->_long.value)
			P1->_long.value = P2->_long.value;
	}
	else
	{
		if (P2->_long.value < P1->_long.value)
			P1->_long.value = P2->_long.value;
	}
	
	goto *jump_end;

__FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	if (is_max)
	{
		if (P2->_float.value > P1->_float.value)
			P1->_float.value = P2->_float.value;
	}
	else
	{
		if (P2->_float.value < P1->_float.value)
			P1->_float.value = P2->_float.value;
	}
	
	goto *jump_end;

__DATE:

  VALUE_conv(P1, T_DATE);
  VALUE_conv(P2, T_DATE);

	if (DATE_comp_value(P1, P2) == (is_max ? -1 : 1))
		*P1 = *P2;

	goto *jump_end;

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

  type = Max(P1->type, P2->type);
  
  if (TYPE_is_number_date(type))
  {
    jump_end = &&__VARIANT_END;
    goto *jump[type];
  }

  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number or date", TYPE_get_name(type));

__VARIANT_END:

  VALUE_conv_variant(P1);

__END:

  SP--;
}
