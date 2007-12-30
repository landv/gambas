/***************************************************************************

  subr_test_temp.c

  The test routines templates

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef STT_INEQUALITY

PUBLIC void STT_NAME(void)
{
  static void *jump[17] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR,
    &&__NULL,
    &&__OBJECT
    };

  TYPE type;
  VALUE *P1, *P2;
  boolean result, variant;

  P1 = SP - 2;
  P2 = P1 + 1;

  variant = FALSE;
  type = EXEC_code & 0x1F;
  goto *jump[type];

__VARIANT:

  type = Max(P1->type, P2->type);

  if (TYPE_is_variant(P1->type))
  {
    #ifdef STT_CASE
    TEMP = *P1;
    P1 = &TEMP;
    #endif
    VARIANT_undo(P1);
    variant = TRUE;
  }

  if (TYPE_is_variant(P2->type))
  {
    VARIANT_undo(P2);
    variant = TRUE;
  }

  if (variant)
    type = Max(P1->type, P2->type);

  if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
    type = T_OBJECT;
  else if (TYPE_is_object(type))
    THROW(E_TYPE, "Object", TYPE_get_name(Min(P1->type, P2->type)));

  if (!variant)
    *PC |= type;

  goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

  result = P1->_integer.value STT_TEST P2->_integer.value;
  goto __END;

__LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

  result = P1->_long.value STT_TEST P2->_long.value;
  goto __END;

__DATE:

  VALUE_conv(P1, T_DATE);
  VALUE_conv(P2, T_DATE);
  
  #ifdef STT_NEAR
  result = P1->_date.date STT_TEST P2->_date.date;
  #else
  result = (DATE_comp_value(P1, P2) STT_TEST 0);
  #endif
  goto __END;

__STRING:

  VALUE_conv(P1, T_STRING);
  VALUE_conv(P2, T_STRING);

  #ifdef STT_NEAR
  result = (STRING_comp_value_ignore_case(P1, P2) STT_TEST 0);
  #else
  result = (STRING_comp_value(P1, P2) STT_TEST 0);
  #endif

  goto __END_RELEASE;

#ifdef STT_NEAR

__SINGLE:

  VALUE_conv(P1, T_FLOAT);
  VALUE_conv(P2, T_FLOAT);

  result = fabs(P1->_float.value - P2->_float.value) <= 1E-6 * fabs(P1->_float.value + P2->_float.value);
  goto __END;
  
__FLOAT:

  VALUE_conv(P1, T_FLOAT);
  VALUE_conv(P2, T_FLOAT);

  result = fabs(P1->_float.value - P2->_float.value) <= 1E-12 * fabs(P1->_float.value + P2->_float.value);
  goto __END;
  
#else
  
__SINGLE:
__FLOAT:

  VALUE_conv(P1, T_FLOAT);
  VALUE_conv(P2, T_FLOAT);

  result = P1->_float.value STT_TEST P2->_float.value;
  goto __END;
  
#endif

__OBJECT:

  result = OBJECT_comp_value(P1, P2) STT_TEST TRUE;
  goto __END_RELEASE;

__NULL:

  result = VALUE_is_null(P1->type == T_NULL ? P2 : P1) STT_TEST TRUE;
  goto __END_RELEASE;

__ERROR:

  THROW(E_TYPE, "Number or Date", TYPE_get_name(type));

__END_RELEASE:

#ifdef STT_CASE
  RELEASE(P2);
#else
  RELEASE(P1);
  RELEASE(P2);
#endif

__END:

  #ifdef STT_CASE

  P2->type = T_BOOLEAN;
  P2->_boolean.value = result ? -1 : 0;

  #else

  P1->type = T_BOOLEAN;
  P1->_boolean.value = result ? -1 : 0;

  SP--;

  #endif

}

#else /* inequality tests */

PUBLIC void STT_NAME(void)
{
  static void *jump[17] = {
    &&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR,
    &&__NULL, &&__ERROR
    };

  TYPE type, typem;
  VALUE *P1, *P2;
  boolean result, variant;

  P1 = SP - 2;
  P2 = P1 + 1;

  variant = FALSE;
  type = EXEC_code & 0x1F;
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

  type = Max(P1->type, P2->type);

  if (type == T_NULL || TYPE_is_string(type))
  {
    typem = Min(P1->type, P2->type);
    if (!TYPE_is_string(typem))
      THROW(E_TYPE, TYPE_get_name(typem), TYPE_get_name(type));
  }
  else if (TYPE_is_object(type))
    goto __ERROR;

  if (!variant)
    *PC |= type;

  goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

  result = P1->_integer.value STT_TEST P2->_integer.value;
  goto __END;

__LONG:
  
  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

  result = P1->_long.value STT_TEST P2->_long.value;
  goto __END;

__DATE:

  VALUE_conv(P1, T_DATE);
  VALUE_conv(P2, T_DATE);

  result = (DATE_comp_value(P1, P2) STT_TEST 0);
  goto __END;

__NULL:

  VALUE_conv(P1, T_STRING);
  VALUE_conv(P2, T_STRING);

__STRING:

  result = (STRING_comp_value(P1, P2) STT_TEST 0);

  goto __END_RELEASE;

__SINGLE:
__FLOAT:

  VALUE_conv(P1, T_FLOAT);
  VALUE_conv(P2, T_FLOAT);

  result = P1->_float.value STT_TEST P2->_float.value;
  goto __END;

__ERROR:

  THROW(E_TYPE, "Number, Date or String", TYPE_get_name(type));

__END_RELEASE:

  RELEASE(P1);
  RELEASE(P2);

__END:

  P1->type = T_BOOLEAN;
  P1->_boolean.value = result ? -1 : 0;

  SP--;
}

#endif


#ifdef STT_INEQUALITY
#undef STT_INEQUALITY
#endif

#undef STT_NAME
#undef STT_TEST

#ifdef STT_NO_OBJECT
#undef STT_NO_OBJECT
#endif

#ifdef STT_CASE
#undef STT_CASE
#endif

#ifdef STT_NULL
#undef STT_NULL
#endif

#ifdef STT_NEAR
#undef STT_NEAR
#endif


