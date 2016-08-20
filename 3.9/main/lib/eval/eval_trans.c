/***************************************************************************

  eval_trans.c

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

#define _TRANS_C

#include <ctype.h>
#include <errno.h>

#include "gb_common.h"
#include "gb_error.h"

#include "gb_reserved.h"
#include "eval_read.h"
#include "eval_trans.h"
#include "eval.h"

/*
PUBLIC void TRANS_reset(void)
{
  JOB->line = 1;
  JOB->current = JOB->pattern;
  JOB->end = &(JOB->pattern[ARRAY_count(JOB->pattern)]);
}


PUBLIC boolean TRANS_newline(void)
{
  if (PATTERN_IS_NEWLINE(*JOB->current))
  {
    JOB->line = PATTERN_INDEX(*JOB->current) + 1;
    JOB->current++;
    return TRUE;
  }

  return FALSE;
}
*/

#if 0
PUBLIC boolean TRANS_get_number(int index, TRANS_NUMBER *result)
{
  char car;
  int val;
  double dval;
  char *end;
  int pos;

  int base = 0;
  char *number = TABLE_get_symbol_name(EVAL->table, index);
  boolean minus = FALSE;
  boolean is_unsigned = FALSE;

  car = *number;

  if (car == '+' || car == '-')
  {
    minus = (car == '-');
    car = *(++number);
  }

  if (car == '&')
  {
    car = *(++number);
    car = toupper(car);

    if (car == 'H')
    {
      base = 16;
      car = *(++number);
    }
    else if (car == 'X')
    {
      base = 2;
      car = *(++number);
    }
    else
      base = 16;
  }
  else if (car == '%')
  {
    base = 2;
    car = *(++number);
  }

  if (!car)
    return TRUE;

  if (car == '-' || car == '+')
    return TRUE;

  if (car == '0' && toupper(number[1]) == 'X')
    return TRUE;

  pos = strlen(number) - 1;
  if (number[pos] == '&')
  {
    number[pos] = 0;
    is_unsigned = TRUE;
  }

  errno = 0;

  if (base)
  {
    val = strtol(number, &end, base);

    if (!is_unsigned && val >= 0x8000L && val <= 0xFFFFL)
      val |= 0xFFFF0000;
  }
  else
  {
    base = 10;
    val = strtol(number, &end, base);
    if (errno || *end)
    {
      if (is_unsigned)
        return TRUE;

      errno = 0;
      base = 0;
      dval = strtod(number, &end);
    }
  }

  if (*end || errno)
    return TRUE;

  if (!base)
  {
    result->type = T_FLOAT;
    result->dval = minus ? (-dval) : dval;
  }
  else
  {
    result->type = T_INTEGER;
    result->ival = minus ? (-val) : val;
  }

  return FALSE;
}
#endif

bool TRANS_get_number(int index, TRANS_NUMBER *result)
{
  GB_VALUE value;
  SYMBOL *sym = TABLE_get_symbol(EVAL->table, index);
	int len = sym->len;

	if (len > 0 && tolower(sym->name[len - 1]) == 'i')
	{
		len--;
		result->complex = TRUE;
	}
	else
		result->complex = FALSE;
	
  if (GB.NumberFromString(GB_NB_READ_ALL | GB_NB_READ_HEX_BIN, sym->name, len, &value))
    return TRUE;

  if (value.type == T_INTEGER)
  {
    result->type = T_INTEGER;
    result->ival = ((GB_INTEGER *)(void *)&value)->value;
  }
  else if (value.type == T_LONG)
  {
    result->type = T_LONG;
    result->lval = ((GB_LONG *)(void *)&value)->value;
  }
  else
  {
    result->type = T_FLOAT;
    result->dval = ((GB_FLOAT *)(void *)&value)->value;
  }

  return FALSE;
}

#if 0
static PATTERN *trans_square(PATTERN *look, int mode, TRANS_DECL *result)
{
  TRANS_NUMBER tnum;
  int i;

  if (!(mode & TT_CAN_SQUARE))
  {
    if (PATTERN_is(*look, RS_LSQR))
      THROW("Arrays are forbidden here");
    return look;
  }

  if (!PATTERN_IS(*look, RS_LSQR))
    return look;

  /*
  if (result->is_array)
    THROW("Syntax error. Duplicated array declaration");
  */

  look++;

  /*
  if (PATTERN_IS(*look, RS_RSQR))
  {
    look++;
    return look;
  }
  */

  if (mode && TT_CAN_ARRAY)
  {
    for (i = 0;; i++)
    {
      if (i > MAX_ARRAY_DIM)
        THROW("Too many dimensions");

      if (TRANS_get_number(PATTERN_INDEX(*look), &tnum))
        THROW(E_SYNTAX);
      if (tnum.type != T_INTEGER)
        THROW(E_SYNTAX);
      if (tnum.ival < 1 || tnum.ival > (2 << 22)) /* 4 Mo, ca devrait suffire... ;-) */
        THROW("Bad subscript range");

      result->array.dim[i] = tnum.ival;
      result->array.ndim++;
      look++;

      if (PATTERN_is(*look, RS_RSQR))
        break;

      if (!PATTERN_is(*look, RS_COMMA))
        THROW("Missing comma");
      look++;
    }
  }

  if (!PATTERN_IS(*look, RS_RSQR))
    THROW("Missing ']'");

  look++;
  return look;
}
#endif

#if 0
PUBLIC boolean TRANS_type(int mode, TRANS_DECL *result)
{
  PATTERN *look = JOB->current;
  short id = 0;
  int value = -1L;
  int flag = 0;

  /* Ne pas remplir la structure de z�os */

  /* Attention ! Probl�e du tableau d'objet ! */

  TYPE_clear(&result->type);
  result->is_new = FALSE;
  result->array.ndim = 0;

  look = trans_square(look, mode, result);

  if (!PATTERN_IS(*look, RS_AS))
  {
    if (mode & TT_DO_NOT_CHECK_AS)
      return FALSE;
    else
      THROW("Missing AS");
  }

  look++;

  if (mode & TT_CAN_NEW)
  {
    if (PATTERN_IS(*look, RS_NEW))
    {
      if (TYPE_get_id(result->type) == T_ARRAY)
        THROW("Cannot mix NEW and array declaration");

      result->is_new = TRUE;
      look++;
    }
  }

  if (PATTERN_IS_TYPE(*look))
  {
    id = RES_get_type(PATTERN_index(*look));
    if (id == T_OBJECT)
      value = (-1);
    look++;
  }
  else if (PATTERN_IS_IDENTIFIER(*look))
  {
    id = T_OBJECT;
    value = CLASS_add_class(JOB->class, PATTERN_INDEX(*look));
    look++;
  }
  else
    THROW(E_SYNTAX);

  /*look = trans_square(look, mode, result);*/

  if (id == T_VOID)
    return FALSE;

  /*
  if (result->is_array && result->array.ndim == 0)
    result->is_array = FALSE;
  */

  if (result->array.ndim > 0)
  {
    result->array.type = TYPE_make(id, value, flag);
    result->type = TYPE_make(T_ARRAY, CLASS_add_array(JOB->class, &result->array), 0);
  }
  else
    result->type = TYPE_make(id, value, flag);

  JOB->current = look;
  return TRUE;
}
#endif

#if 0
PUBLIC boolean TRANS_check_declaration(void)
{
  PATTERN *look = JOB->current;

  if (!PATTERN_is_identifier(*look))
    return FALSE;
  look++;

  if (PATTERN_is(*look, RS_LSQR))
  {
    for(;;)
    {
      look++;
      if (PATTERN_is(*look, RS_RSQR))
        break;
      if (PATTERN_is_newline(*look))
        return FALSE;
    }
    look++;
  }

  if (!PATTERN_is(*look, RS_AS))
    return FALSE;

  return TRUE;
}
#endif

#if 0
PUBLIC void TRANS_get_constant_value(TRANS_DECL *decl, PATTERN value)
{
  int index;
  TRANS_NUMBER number;
  int type;

  index = PATTERN_index(value);

  /* V�ification de la constante */

  type = TYPE_get_id(decl->type);

  switch(type)
  {
    case T_BOOLEAN:

      decl->is_integer = TRUE;

      if (PATTERN_is(value, RS_TRUE))
        decl->value = -1L;
      else if (PATTERN_is(value, RS_FALSE))
        decl->value = 0L;
      else
        THROW("Type mismatch");

      break;

    case T_BYTE: case T_SHORT: case T_INTEGER:

      decl->is_integer = TRUE;

      if (TRANS_get_number(index, &number))
        THROW("Type mismatch");

      if (number.type != T_INTEGER)
        THROW("Type mismatch");

      if (((type == T_BYTE) && (number.ival < 0 || number.ival > 255))
          || ((type == T_SHORT) && (number.ival < -32768L || number.ival > 32767L)))
        THROW("Out of range");

      decl->value = number.ival;
      break;

    case T_STRING: case T_FLOAT:

      decl->is_integer = FALSE;
      decl->value = index;
      break;

    default:

      THROW("Bad constant type");
  }
}
#endif

#if 0
PUBLIC void TRANS_want(int reserved)
{
  if (!PATTERN_is(*JOB->current, reserved))
    THROW("Syntax error. %s expected", COMP_res_info[reserved].name);
  JOB->current++;
}


PUBLIC boolean TRANS_is(int reserved)
{
  if (PATTERN_is(*JOB->current, reserved))
  {
    JOB->current++;
    return TRUE;
  }
  else
    return FALSE;
}

PUBLIC void TRANS_ignore(int reserved)
{
  if (PATTERN_is(*JOB->current, reserved))
    JOB->current++;
}

#endif

