/***************************************************************************

  subr_math.c

  Mathematical routines

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

static MATH_FUNC MathFunc[] = {
  NULL, frac, log, exp, sqrt, sin, cos, tan, atan, asin, acos,
  deg, rad, log10, sinh, cosh, tanh, asinh, acosh, atanh,
  exp2, exp10, log2, cbrt, expm1, log1p
  };

static MATH_FUNC_2 MathFunc2[] = {
  NULL, atan2, ang, hypot
  };


#define SMT_NAME    SUBR_quo
#define SMT_TYPE    3
#define SMT_OP      /

#include "gbx_subr_math_temp.h"


#define SMT_NAME    SUBR_rem
#define SMT_TYPE    3
#define SMT_OP      %

#include "gbx_subr_math_temp.h"


void SUBR_pi(void)
{
  SUBR_ENTER();

  if (NPARAM == 0)
  {
    RETURN->type = T_FLOAT;
    RETURN->_float.value = M_PI;
  }
  else
  {
    VALUE_conv(PARAM, T_FLOAT);
    RETURN->type = T_FLOAT;
    RETURN->_float.value = M_PI * PARAM->_float.value;
  }

  SUBR_LEAVE();
}


void SUBR_randomize(void)
{
	SUBR_ENTER();

	if (NPARAM == 0)
		randomize(FALSE, 0);
	else
		randomize(TRUE, (uint)SUBR_get_integer(PARAM));

  RETURN->type = T_VOID;

  SUBR_LEAVE();
}


void SUBR_rnd(void)
{
  double min = 0.0, max = 1.0;

  SUBR_ENTER();

  if (NPARAM >= 1)
  {
    VALUE_conv(&PARAM[0], T_FLOAT);
    max = PARAM->_float.value;
  }

  if (NPARAM == 2)
  {
    min = max;
    VALUE_conv(&PARAM[1], T_FLOAT);
    max = PARAM[1]._float.value;
  }

  RETURN->type = T_FLOAT;
  RETURN->_float.value = (rnd() * (max - min)) + min;

  SUBR_LEAVE();
}


void SUBR_round(void)
{
  int val = 0;
  double power;

  SUBR_ENTER();

  if (NPARAM == 2)
    val = SUBR_get_integer(&PARAM[1]);

  power = pow(10, val);

  VALUE_conv(&PARAM[0], T_FLOAT);

  RETURN->type = T_FLOAT;
  /*RETURN->_float.value = rint(PARAM->_float.value / power) * power;*/
  RETURN->_float.value = floor(PARAM->_float.value / power + 0.5) * power;

  SUBR_LEAVE();
}


void SUBR_math(void)
{
  SUBR_ENTER_PARAM(1);

  /*if (TYPE_is_variant(PARAM->type))
    VARIANT_undo(PARAM);

  if (!TYPE_is_number(PARAM->type))
    THROW(E_TYPE, "Number", TYPE_get_name(PARAM->type));*/

  VALUE_conv(PARAM, T_FLOAT);

  PARAM->_float.value = (*MathFunc[EXEC_code & 0x1F])(PARAM->_float.value);

  if (!finite(PARAM->_float.value))
    THROW(E_MATH);
}


void SUBR_math2(void)
{
  SUBR_ENTER_PARAM(2);

  VALUE_conv(&PARAM[0], T_FLOAT);
  VALUE_conv(&PARAM[1], T_FLOAT);

  PARAM->_float.value = (*MathFunc2[EXEC_code & 0x1F])(PARAM[0]._float.value, PARAM[1]._float.value);

  if (!finite(PARAM->_float.value))
    THROW(E_MATH);

  SP--;
}


void SUBR_pow(void)
{
  SUBR_ENTER_PARAM(2);

  VALUE_conv(&PARAM[0], T_FLOAT);
  VALUE_conv(&PARAM[1], T_FLOAT);

  PARAM->_float.value = pow(PARAM[0]._float.value, PARAM[1]._float.value);

  if (!finite(PARAM->_float.value))
    THROW(E_MATH);

  SP--;
}


void SUBR_not(void)
{
  static void *jump[17] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL,
    &&__OBJECT
    };

  VALUE *P1;
  void *jump_end;
  TYPE type = EXEC_code & 0x1F;
  boolean test;

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

  VALUE_conv(P1, T_VARIANT);

__END:
  return;
}



void SUBR_add_quick(int value)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__DATE, &&__STRING, &&__STRING
    };

  TYPE type;
  VALUE *P1 = SP - 1;
  void *jump_end;

__VARIANT:

  if (TYPE_is_variant(P1->type))
  {
    jump_end = &&__VARIANT_END;
    VARIANT_undo(P1);
  }
  else
    jump_end = &&__END;

  type = P1->type;

  if (type <= T_CSTRING)
    goto *jump[type];

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__BOOLEAN:
  
  P1->_integer.value ^= (value & 1) ? -1 : 0;
  goto *jump_end;

__BYTE:
  
  P1->_integer.value = (unsigned char)(P1->_integer.value + value);
  goto *jump_end;

__SHORT:

  P1->_integer.value = (short)(P1->_integer.value + value);
  goto *jump_end;

__INTEGER:

  P1->_integer.value += value;
  goto *jump_end;

__LONG:

  P1->_long.value += (int64_t)value;
  goto *jump_end;

__DATE:
__STRING:

  VALUE_conv(P1, T_FLOAT);

__FLOAT:

  P1->_float.value += (double)value;
  goto *jump_end;

__VARIANT_END:

  VALUE_conv(P1, T_VARIANT);

__END:
  return;
}


void SUBR_and_(void)
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
  type = EXEC_code & 0x0F;
  op = (EXEC_code - C_AND) >> 8;
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
    VALUE_conv(P1, T_BOOLEAN);

  if (TYPE_is_string(P2->type))
    VALUE_conv(P2, T_BOOLEAN);

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

  VALUE_conv(P1, T_VARIANT);

__END:

  SP--;
}



void SUBR_sgn(void)
{
  static void *jump[] = {
    &&__VARIANT, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__ERROR
    };

  VALUE *P1;
  TYPE type;

  P1 = SP - 1;
  type = EXEC_code & 0x0F;
  goto *jump[type];

__INTEGER:  P1->_integer.value = lsgn(P1->_integer.value); goto __END;

__LONG: P1->_integer.value = llsgn(P1->_long.value); goto __END;

__FLOAT: P1->_integer.value = fsgn(P1->_float.value); goto __END;

__VARIANT:

  type = P1->type;

  if (TYPE_is_number(type))
  {
    *PC |= type;
    goto *jump[type];
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


void SUBR_neg_(void)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__ERROR
    };

  VALUE *P1;
  void *jump_end;
  TYPE type;
  short op;

  P1 = SP - 1;
  jump_end = &&__END;

  type = EXEC_code & 0x0F;

  op = EXEC_code >> 8;
  op = (op == (C_NEG >> 8)) ? 0 : op - CODE_ABS + 1;

  goto *jump[type];

__BYTE:

  {
    static void *exec[] = { &&__NEG_C, &&__ABS_C, &&__INT_C, &&__FIX_C }; //, &&__SGN_I };
    goto *exec[op];

    __NEG_C: P1->_integer.value = (unsigned char)(-P1->_integer.value); goto *jump_end;
    __ABS_C: P1->_integer.value = (unsigned char)ABS(P1->_integer.value); goto *jump_end;
    __INT_C: goto *jump_end;
    __FIX_C: goto *jump_end;
  }

__SHORT:

  {
    static void *exec[] = { &&__NEG_H, &&__ABS_H, &&__INT_H, &&__FIX_H }; //, &&__SGN_I };
    goto *exec[op];

    __NEG_H: P1->_integer.value = (short)(-P1->_integer.value); goto *jump_end;
    __ABS_H: P1->_integer.value = (short)ABS(P1->_integer.value); goto *jump_end;
    __INT_H: goto *jump_end;
    __FIX_H: goto *jump_end;
  }

__BOOLEAN:
__INTEGER:

  {
    static void *exec[] = { &&__NEG_I, &&__ABS_I, &&__INT_I, &&__FIX_I }; //, &&__SGN_I };
    goto *exec[op];

    __NEG_I: P1->_integer.value = (-P1->_integer.value); goto *jump_end;
    __ABS_I: P1->_integer.value = ABS(P1->_integer.value); goto *jump_end;
    __INT_I: goto *jump_end;
    __FIX_I: goto *jump_end;
    //__SGN_I: P1->_integer.value = lsgn(P1->_integer.value); goto __END_SGN;
  }

__LONG:

  //VALUE_conv(P1, T_LONG);

  {
    static void *exec[] = { &&__NEG_L, &&__ABS_L, &&__INT_L, &&__FIX_L }; //, &&__SGN_L };
    goto *exec[op];

    __NEG_L: P1->_long.value = (-P1->_long.value); goto *jump_end;
    __ABS_L: P1->_long.value = ABS(P1->_long.value); goto *jump_end;
    __INT_L: goto *jump_end;
    __FIX_L: goto *jump_end;
    //__SGN_L: P1->_integer.value = llsgn(P1->_integer.value); P1->type = T_INTEGER; goto __END_SGN;
  }

__FLOAT:

  VALUE_conv(P1, T_FLOAT);

  {
    static void *exec[] = { &&__NEG_F, &&__ABS_F, &&__INT_F, &&__FIX_F }; //, &&__SGN_F };
    goto *exec[op];

    __NEG_F: P1->_float.value = (-P1->_float.value); goto *jump_end;
    __ABS_F: P1->_float.value = fabs(P1->_float.value); goto *jump_end;
    __INT_F: P1->_float.value = floor(P1->_float.value); goto *jump_end;
    __FIX_F: P1->_float.value = fix(P1->_float.value); goto *jump_end;
    //__SGN_F: P1->_integer.value = fsgn(P1->_integer.value); P1->type = T_INTEGER; goto __END_SGN;
  }

__VARIANT:

  type = P1->type;

  if (TYPE_is_number_date(type))
  {
    *PC |= type;
    goto *jump[type];
  }

  if (TYPE_is_variant(type))
  {
    type = P1->_variant.vtype;
    if (TYPE_is_number_date(type))
    {
      VARIANT_undo(P1);
      jump_end = &&__VARIANT_END;
      goto *jump[type];
    }
  }

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__VARIANT_END:

  VALUE_conv(P1, T_VARIANT);

//__END_SGN:
//  P1->type = T_INTEGER;

__END:
  return;
}



void SUBR_add_(void)
{
  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__DATE
    };

  TYPE type;
  VALUE *P1, *P2;
  void *jump_end;
  short op;

  P1 = SP - 2;
  P2 = P1 + 1;

  jump_end = &&__END;
  type = EXEC_code & 0x0F;
  op = (EXEC_code - C_ADD) >> 8;
  goto *jump[type];

__BOOLEAN:
	
	P1->type = type;

  {
    static void *exec[] = { &&__ADD_B, && __SUB_B, &&__MUL_B, &&__FLOAT };
    goto *exec[op];

    __ADD_B: P1->_integer.value = P1->_integer.value | P2->_integer.value; goto *jump_end;
    __SUB_B: P1->_integer.value = P1->_integer.value ^ P2->_integer.value; goto *jump_end;
    __MUL_B: P1->_integer.value = P1->_integer.value & P2->_integer.value; goto *jump_end;
  }

__BYTE:
	
	P1->type = type;

  {
    static void *exec[] = { &&__ADD_C, && __SUB_C, &&__MUL_C, &&__FLOAT };
    goto *exec[op];

    __ADD_C: P1->_integer.value = (unsigned char)(P1->_integer.value + P2->_integer.value); goto *jump_end;
    __SUB_C: P1->_integer.value = (unsigned char)(P1->_integer.value - P2->_integer.value); goto *jump_end;
    __MUL_C: P1->_integer.value = (unsigned char)(P1->_integer.value * P2->_integer.value); goto *jump_end;
  }

__SHORT:
	
	P1->type = type;

  {
    static void *exec[] = { &&__ADD_H, && __SUB_H, &&__MUL_H, &&__FLOAT };
    goto *exec[op];

    __ADD_H: P1->_integer.value = (short)(P1->_integer.value + P2->_integer.value); goto *jump_end;
    __SUB_H: P1->_integer.value = (short)(P1->_integer.value - P2->_integer.value); goto *jump_end;
    __MUL_H: P1->_integer.value = (short)(P1->_integer.value * P2->_integer.value); goto *jump_end;
  }

__INTEGER:

	P1->type = type;

  {
    static void *exec[] = { &&__ADD_I, && __SUB_I, &&__MUL_I, &&__FLOAT };
    goto *exec[op];

    __ADD_I: P1->_integer.value += P2->_integer.value; goto *jump_end;
    __SUB_I: P1->_integer.value -= P2->_integer.value; goto *jump_end;
    __MUL_I: P1->_integer.value *= P2->_integer.value; goto *jump_end;
  }

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	P1->type = type;

  {
    static void *exec[] = { &&__ADD_L, && __SUB_L, &&__MUL_L, &&__FLOAT };
    goto *exec[op];

    __ADD_L: P1->_long.value += P2->_long.value; goto *jump_end;
    __SUB_L: P1->_long.value -= P2->_long.value; goto *jump_end;
    __MUL_L: P1->_long.value *= P2->_long.value; goto *jump_end;
  }

__DATE:

  VALUE_conv(P1, T_FLOAT);
  VALUE_conv(P2, T_FLOAT);

  {
    static void *exec[] = { &&__ADD_F, && __SUB_F, &&__ERROR, &&__ERROR };
    goto *exec[op];
  }

__FLOAT:

  VALUE_conv(P1, T_FLOAT);
  VALUE_conv(P2, T_FLOAT);

  {
    static void *exec[] = { &&__ADD_F, && __SUB_F, &&__MUL_F, &&__DIV_F };
    goto *exec[op];

    __ADD_F: P1->_float.value += P2->_float.value; goto *jump_end;
    __SUB_F: P1->_float.value -= P2->_float.value; goto *jump_end;
    __MUL_F: P1->_float.value *= P2->_float.value; goto *jump_end;

    __DIV_F:
    	P1->_float.value /= P2->_float.value;
  	  if (!finite(P1->_float.value))
		  {
				if (P2->_float.value == 0.0)
					THROW(E_ZERO);
				else
					THROW(E_MATH);
			}
    	goto *jump_end;
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
    VALUE_conv(P1, T_FLOAT);

  if (TYPE_is_string(P2->type))
    VALUE_conv(P2, T_FLOAT);

  if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type))
    type = T_NULL;
  else
    type = Max(P1->type, P2->type);

  if (TYPE_is_number_date(type))
  {
    jump_end = &&__VARIANT_END;
    goto *jump[type];
  }

  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__VARIANT_END:

  VALUE_conv(P1, T_VARIANT);

__END:

  SP--;
}
