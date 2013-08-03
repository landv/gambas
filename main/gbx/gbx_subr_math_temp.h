/***************************************************************************

  gbx_subr_math_temp.h

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

#if SMT_TYPE == 1

void SMT_NAME(void)
{
  #ifdef SMT_FLOAT

  static void *jump[] = {
    &&__VARIANT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__ERROR
    };

  #elif defined(SMT_INTEGER)

  static void *jump[] = {
    &&__VARIANT, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__LONG, &&__ERROR, &&__ERROR, &&__ERROR
    };

  #else

  static void *jump[] = {
    &&__VARIANT, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__ERROR
    };

  #endif

  VALUE *P1;
  void *jump_end;
  TYPE type = EXEC_code & 0x0F;

  P1 = SP - 1;
  jump_end = &&__END;
  goto *jump[type];

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

  goto __ERROR;

#ifndef SMT_FLOAT

__INTEGER:

  #ifdef SMT_OP
  P1->_integer.value = SMT_OP ( P1->_integer.value );
  #elif defined(SMT_FUNC_INTEGER)
  P1->_integer.value = SMT_FUNC_INTEGER ( P1->_integer.value );
  #elif defined(SMT_FUNC)
  P1->_integer.value = SMT_FUNC ( P1->_integer.value );
  #endif

  P1->type = type;
  goto *jump_end;

__LONG:

  VALUE_conv(P1, T_LONG);

  #ifdef SMT_OP
  P1->_long.value = SMT_OP ( P1->_long.value );
  #elif defined(SMT_FUNC_LONG)
  P1->_long.value = SMT_FUNC_LONG ( P1->_long.value );
  #elif defined(SMT_FUNC)
  P1->_long.value = SMT_FUNC ( P1->_long.value );
  #else
    #error "LONG function not defined"
  #endif

  P1->type = type;
  goto *jump_end;

#endif

#ifndef SMT_INTEGER

__FLOAT:

  VALUE_conv_float(P1);

  #ifdef SMT_OP
  P1->_float.value = SMT_OP ( P1->_float.value );
  #elif defined(SMT_FUNC_FLOAT)
  P1->_float.value = SMT_FUNC_FLOAT ( P1->_float.value );
  #elif defined(SMT_FUNC)
  P1->_float.value = SMT_FUNC ( P1->_float.value );
  #endif

  if (!finite(P1->_float.value))
    THROW(E_MATH);

  goto *jump_end;

#endif

__ERROR:

  THROW(E_TYPE, "Number", TYPE_get_name(type));

__VARIANT_END:

  VALUE_conv_variant(P1);

__END:
  return;
/*  SP--;*/
/*  if (PCODE_is_void(*PC)) SP--;*/
}

#endif


#if SMT_TYPE == 2

void SMT_NAME(void)
{

  #ifdef SMT_FLOAT

  static void *jump[] = {
    &&__VARIANT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__FLOAT, &&__DATE
    };

  #elif defined(SMT_INTEGER)

  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__ERROR, &&__ERROR, &&__DATE
    };

  #else

  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__FLOAT, &&__FLOAT, &&__DATE
    };

  #endif

  TYPE type;
  VALUE *P1, *P2;
  void *jump_end;

  P1 = SP - 2;
  P2 = P1 + 1;

  jump_end = &&__END;
  type = EXEC_code & 0x0F;
  goto *jump[type];

#ifndef SMT_FLOAT

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

  /*
  VALUE_conv(P1, type);
  VALUE_conv(P2, type);
  */

  #ifdef SMT_TEST_ZERO
  if (P2->_integer.value == 0)
    THROW(E_ZERO);
  #endif

  P1->_integer.value SMT_OP P2->_integer.value;
  P1->type = type;
  goto *jump_end;

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

  #ifdef SMT_TEST_ZERO
  if (P2->_long.value == 0)
    THROW(E_ZERO);
  #endif

  P1->_long.value SMT_OP P2->_long.value;
  P1->type = type;
  goto *jump_end;

#endif

__DATE:

#ifndef SMT_DATE

  goto __ERROR;

#endif

#ifndef SMT_INTEGER

__FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

  #ifdef SMT_OP
  P1->_float.value SMT_OP P2->_float.value;
  #elif defined(SMT_FUNC)
  P1->_float.value = SMT_FUNC ( P1->_float.value, P2->_float.value );
  #endif

  #ifdef SMT_TEST_ZERO
  if (!finite(P1->_float.value))
  {
    if (P2->_float.value == 0.0)
      THROW(E_ZERO);
    else
      THROW(E_MATH);
  }
  #elif defined(SMT_TEST_RESULT)
  if (!finite(P1->_float.value))
    THROW(E_MATH);
  #endif

  goto *jump_end;

#endif

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
    VALUE_conv_float(P1);

  if (TYPE_is_string(P2->type))
    VALUE_conv_float(P2);

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

  VALUE_conv_variant(P1);

__END:

  SP--;
  /*if (!PCODE_is_void(*PC)) SP++;*/
}

#endif


#if SMT_TYPE == 3

void SMT_NAME(ushort code)
{

  static void *jump[] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__ERROR, &&__ERROR, &&__ERROR
    };

  TYPE type;
  VALUE *P1, *P2;

  P1 = SP - 2;
  P2 = P1 + 1;

  type = code & 0x0F;
  goto *jump[type];

__VARIANT:

  type = Max(P1->type, P2->type);

  if (TYPE_is_integer_long(type))
  {
    *PC |= type;
    goto *jump[type];
  }

  if (TYPE_is_variant(P1->type))
    VARIANT_undo(P1);

  if (TYPE_is_variant(P2->type))
    VARIANT_undo(P2);

  if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type))
    type = T_NULL;
  else
    type = Max(P1->type, P2->type);

  if (TYPE_is_integer_long(type))
    goto *jump[type];

  goto __ERROR;

__BOOLEAN:
  
  if (P2->_integer.value == 0)
    THROW(E_ZERO);

  P1->type = T_BOOLEAN;
  goto __END;

__BYTE:

  if (P2->_integer.value == 0)
    THROW(E_ZERO);

  P1->_integer.value = (unsigned char)(P1->_integer.value SMT_OP P2->_integer.value);
  P1->type = T_BYTE;
  goto __END;

__SHORT:

  if (P2->_integer.value == 0)
    THROW(E_ZERO);

  P1->_integer.value = (short)(P1->_integer.value SMT_OP P2->_integer.value);
  P1->type = T_SHORT;
  goto __END;

__INTEGER:

  if (P2->_integer.value == 0)
    THROW(E_ZERO);

  P1->_integer.value = P1->_integer.value SMT_OP P2->_integer.value;
  P1->type = T_INTEGER;
  goto __END;

__LONG:

	VALUE_conv(P1, T_LONG);
	VALUE_conv(P2, T_LONG);
	
  if (P2->_long.value == 0)
    THROW(E_ZERO);

  P1->_long.value = P1->_long.value SMT_OP P2->_long.value;
  P1->type = T_LONG;
  goto __END;

__ERROR:

  THROW(E_TYPE, "Integer", TYPE_get_name(type));

__END:

  SP--;
  /*if (!PCODE_is_void(*PC)) SP++;*/
}

#endif


#undef SMT_TYPE
#undef SMT_NAME

#ifdef SMT_OP
#undef SMT_OP
#endif

#ifdef SMT_FUNC
#undef SMT_FUNC
#endif

#ifdef SMT_FUNC_INTEGER
#undef SMT_FUNC_INTEGER
#endif

#ifdef SMT_FUNC_LONG
#undef SMT_FUNC_LONG
#endif

#ifdef SMT_FUNC_FLOAT
#undef SMT_FUNC_FLOAT
#endif

#ifdef SMT_FLOAT
#undef SMT_FLOAT
#endif

#ifdef SMT_DATE
#undef SMT_DATE
#endif

#ifdef SMT_INTEGER
#undef SMT_INTEGER
#endif

#ifdef SMT_TEST_ZERO
#undef SMT_TEST_ZERO
#endif

#ifdef SMT_CHECK_FLOAT
#undef SMT_CHECK_FLOAT
#endif

#ifdef SMT_RESULT
#undef SMT_RESULT
#endif
