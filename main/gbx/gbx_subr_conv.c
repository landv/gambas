/***************************************************************************

  gbx_subr_conv.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "gb_common.h"

#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_local.h"

#include "gbx_string.h"
#include "gbx_date.h"
#include "gbx_number.h"


void SUBR_is_type(void)
{
  static void *jump[] = {
    &&__BAD, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__BAD, &&__BAD, &&__BAD, &&__BAD, &&__BAD, &&__NULL,
    &&__OBJECT, &&__NUMBER
    };

  bool test;
  TYPE type;

  SUBR_ENTER_PARAM(1);

  VARIANT_undo(PARAM);

  type = EXEC_code & 0x3F;
  goto *jump[type];

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:
__LONG:
__SINGLE:
__FLOAT:
__DATE:

  test = PARAM->type == type;
  goto __END;

__STRING:

  test = PARAM->type == T_STRING || PARAM->type == T_CSTRING;
  if (test)
    goto __END;
  else
    goto __NULL;

__NULL:

  test = VALUE_is_null(PARAM);
  goto __END;

__OBJECT:

  test = TYPE_is_object_null(PARAM->type);
  goto __END;

__NUMBER:

  test = TYPE_is_number(PARAM->type);
  goto __END;

__BAD:

  THROW(E_ILLEGAL);

__END:

  RETURN->type = T_BOOLEAN;
  RETURN->_integer.value = test ? -1 : 0;

  SUBR_LEAVE();
}


void SUBR_conv(void)
{
  VALUE_conv(SP - 1, EXEC_code & 0x3F);
}


void SUBR_type(void)
{
  TYPE type;

  SUBR_ENTER_PARAM(1);

  /* pas bon ! Ne respecte pas la transitivit�de type */

  /*if (TYPE_is_object(PARAM->type))
  {
    type = TYPE_get_name((TYPE)OBJECT_class(PARAM->_object.object));
    STRING_new_constant_value(RETURN, type, -1);
  }
  else
  */

  if (VALUE_is_null(PARAM))
    RETURN->_integer.value = T_NULL;
  else
  {
    type = PARAM->type;
    if (type == T_VARIANT)
      type = PARAM->_variant.vtype;

    if (type == T_CSTRING)
      RETURN->_integer.value = T_STRING;
    else if (TYPE_is_object(type))
      RETURN->_integer.value = T_OBJECT;
    else
      RETURN->_integer.value = type;
  }

  RETURN->type = T_INTEGER;

  SUBR_LEAVE();
}


void SUBR_str(void)
{
  char *addr;
  int len;

  SUBR_ENTER_PARAM(1);

  VALUE_to_string(PARAM, &addr, &len);
  STRING_new_temp_value(RETURN, addr, len);

  SUBR_LEAVE();
}


void SUBR_val(void)
{
  char *addr;
  int len;
  VALUE result;

  SUBR_ENTER_PARAM(1);

  SUBR_check_string(PARAM);

  VALUE_get_string(PARAM, &addr, &len);

  VALUE_from_string(&result, addr, len);

  VALUE_conv(&result, T_VARIANT);

  *RETURN = result;

  SUBR_LEAVE();
}


void SUBR_format(void)
{
  int fmt_type;
  char *format = NULL;
  int len = 0;
  DATE_SERIAL *date;
  char *str;
  int len_str;

  SUBR_ENTER();

  if (NPARAM == 1)
    fmt_type = LF_STANDARD;
  else
  {
		if (PARAM[1].type == T_VARIANT)
			VARIANT_undo(&PARAM[1]);
			
    if (TYPE_is_string(PARAM[1].type))
    {
      fmt_type = LF_USER;
      VALUE_get_string(&PARAM[1], &format, &len);
    }
    else if (TYPE_is_integer(PARAM[1].type))
    {
      fmt_type = PARAM[1]._integer.value;
			if (fmt_type <= LF_USER || fmt_type >= LF_MAX)
				THROW(E_ARG);
    }
    else
      THROW(E_TYPE, TYPE_get_name(T_INTEGER), TYPE_get_name(PARAM[1].type));
  }

	if (PARAM->type == T_VARIANT)
		VARIANT_undo(PARAM);
	
  if (PARAM->type == T_DATE)
  {
    date = DATE_split(PARAM);
    if (LOCAL_format_date(date, fmt_type, format, len, &str, &len_str))
      THROW(E_FORMAT);
  }
  else
  {
    VALUE_conv(PARAM, T_FLOAT);
    if (LOCAL_format_number(PARAM->_float.value, fmt_type, format, len, &str, &len_str, TRUE))
      THROW(E_FORMAT);
  }

  /*if (NPARAM >= 2)
    RELEASE_STRING(&PARAM[1]);*/

  STRING_new_temp_value(RETURN, str, len_str);

  SUBR_LEAVE();
}


void SUBR_hex(void)
{
  int prec = 0;

  SUBR_ENTER();

  VALUE_conv(PARAM, T_LONG);

  if (NPARAM == 2)
  {
    VALUE_conv(&PARAM[1], T_INTEGER);

    prec = PARAM[1]._integer.value;

    if (prec < 1 || prec > 16)
      THROW(E_ARG);
  }

  NUMBER_int_to_string(PARAM->_long.value, prec, 16, RETURN);

  SUBR_LEAVE();
}


void SUBR_bin(void)
{
  int prec = 0;

  SUBR_ENTER();

  VALUE_conv(PARAM, T_LONG);

  if (NPARAM == 2)
  {
    VALUE_conv(&PARAM[1], T_INTEGER);

    prec = PARAM[1]._integer.value;

    if (prec < 1 || prec > 64)
      THROW(E_ARG);
  }

  NUMBER_int_to_string(PARAM->_long.value, prec, 2, RETURN);

  SUBR_LEAVE();
}


