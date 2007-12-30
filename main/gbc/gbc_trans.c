/***************************************************************************

  trans.c

  Common routines for pattern translation

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


#define __GBC_TRANS_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#include "gb_common.h"
#include "gb_error.h"

#include "gbc_compile.h"
#include "gbc_read.h"
#include "gbc_trans.h"
#include "gb_reserved.h"
#include "gb_code.h"


PUBLIC int TRANS_in_affectation = 0;

PUBLIC void TRANS_reset(void)
{
  JOB->line = 1;
  JOB->current = JOB->pattern;
  JOB->end = &(JOB->pattern[JOB->pattern_count]);
}


PUBLIC boolean TRANS_newline(void)
{
  if (PATTERN_is_newline(*JOB->current))
  {
    JOB->line = PATTERN_index(*JOB->current) + 1;
    JOB->current++;
    return TRUE;
  }

  return FALSE;
}


PUBLIC bool TRANS_get_number(long index, TRANS_NUMBER *result)
{
  char car;
  long long val;
  double dval = 0;
  char *end;
  int pos;
  bool long_int = FALSE;

  int base = 0;
  char *number = (char *)TABLE_get_symbol_name(JOB->class->table, index);
  boolean minus = FALSE;
  boolean is_unsigned = FALSE;

  //fprintf(stderr, "TRANS_get_number: '%s'\n", number);

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
    if (is_unsigned)
      val = (long long)strtoull(number, &end, base);
    else
    {
      val = (long)strtoul(number, &end, base);
      if (errno || *end)
        val = strtoll(number, &end, base);
    }

    long_int = val != (long)val;

    /*if (is_unsigned)*/
    //  val = (long)strtoul(number, &end, base);
    /*else
      val = strtol(number, &end, base);*/

    if (!is_unsigned)
    {
      /*if (long_int)
      {
        if (val >= 0x80000000L && val <= 0xFFFFFFFFL)
          val |= 0xFFFFFFFF00000000LL;
      }
      else*/
      {
        if (val >= 0x8000L && val <= 0xFFFFL)
          val |= 0xFFFFFFFFFFFF0000LL;
      }
    }
  }
  else
  {
    base = 10;
    val = strtol(number, &end, base);

    if (errno || *end || val < 0)
    {
      errno = 0;
      val = strtoll(number, &end, base);
      long_int = TRUE;
    }

    if (errno || *end || val < 0)
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
  else if (long_int)
  {
    result->type = T_LONG;
    result->lval = minus ? (-val) : val;
    //fprintf(stderr, "TRANS_get_number: LONG: %lld\n", result->lval);
  }
  else
  {
    result->type = T_INTEGER;
    result->ival = (long)(minus ? (-val) : val);
    result->lval = result->ival;
    //fprintf(stderr, "TRANS_get_number: INT: %ld\n", result->ival);
  }

  //fprintf(stderr, "TRANS_get_number: => %d\n", result->type);

  return FALSE;
}


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

  if (!PATTERN_is(*look, RS_LSQR))
    return look;

  /*
  if (result->is_array)
    THROW("Syntax error. Duplicated array declaration");
  */

  look++;

  /*
  if (PATTERN_is(*look, RS_RSQR))
  {
    look++;
    return look;
  }
  */

  if (mode && TT_CAN_ARRAY)
  {
    for (i = 0;; i++)
    {
      if (i >= MAX_ARRAY_DIM)
        THROW("Too many dimensions");

      if (TRANS_get_number(PATTERN_index(*look), &tnum))
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

  if (!PATTERN_is(*look, RS_RSQR))
    THROW("Missing ']'");

  look++;
  return look;
}


PUBLIC long TRANS_get_class(PATTERN pattern)
{
  long index = PATTERN_index(pattern);

  if (!CLASS_exist_class(JOB->class, index))
    THROW("Unknown identifier: &1", TABLE_get_symbol_name(JOB->class->table, index));

  return CLASS_add_class(JOB->class, index);
}


PUBLIC bool TRANS_type(int mode, TRANS_DECL *result)
{
  PATTERN *look = JOB->current;
  short id = 0;
  long value = -1L;
  long flag = 0;

  /* Ne pas remplir la structure de z�os */

  /* Attention ! Probl�e du tableau d'objet ! */

  TYPE_clear(&result->type);
  result->is_new = FALSE;
  result->init = NULL;
  result->array.ndim = 0;

  look = trans_square(look, mode, result);

  if (!PATTERN_is(*look, RS_AS))
  {
    if (mode & TT_DO_NOT_CHECK_AS)
      return FALSE;
    else
      THROW("Missing AS");
  }

  look++;

  if (mode & TT_CAN_NEW)
  {
    if (PATTERN_is(*look, RS_NEW))
    {
      if (TYPE_get_id(result->type) == T_ARRAY)
        THROW("Cannot mix NEW and array declaration");

      result->is_new = TRUE;
      look++;
      result->init = look;
    }
  }

  if (PATTERN_is_type(*look))
  {
    id = RES_get_type(PATTERN_index(*look));

    if (PATTERN_is(look[1], RS_LSQR))
    {
      value = CLASS_get_array_class(JOB->class, id);
      id = T_OBJECT;

      if (!PATTERN_is(look[2], RS_RSQR))
      {
        if (mode & TT_CAN_NEW)
        {
          if (TYPE_get_id(result->type) == T_ARRAY)
            THROW("Cannot mix NEW and array declaration");

          result->is_new = TRUE;
          result->init = look;
        }
        else
          THROW("Syntax error");
      }

      while (!PATTERN_is_newline(*look))
        look++;
    }
    else
    {
      if (id == T_OBJECT)
        value = (-1);
      look++;
    }
  }
  else if (PATTERN_is_class(*look))
  {
    id = T_OBJECT;
    value = TRANS_get_class(*look);
    look++;
  }
  else
    THROW(E_UNEXPECTED, READ_get_pattern(look));

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
  {
    result->type = TYPE_make(id, value, flag);

    if ((mode & TT_CAN_NEW) && !result->is_new && PATTERN_is(*look, RS_EQUAL))
    {
      look++;
      result->init = look;
      while (!PATTERN_is_newline(*look))
        look++;
    }
  }

  JOB->current = look;
  return TRUE;
}


PUBLIC bool TRANS_check_declaration(void)
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



PUBLIC PATTERN *TRANS_get_constant_value(TRANS_DECL *decl, PATTERN *current)
{
  long index;
  TRANS_NUMBER number;
  int type;
  PATTERN value;

	value = *current++;
  index = PATTERN_index(value);

  type = TYPE_get_id(decl->type);

  if (type != T_BOOLEAN && type <= T_FLOAT)
  {
    if (TRANS_get_number(index, &number))
      THROW("Type mismatch");
  }

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

      if (number.type != T_INTEGER)
      {
        if (number.type == T_LONG)
          THROW("Out of range");
        else
          THROW("Type mismatch");
      }

      if (((type == T_BYTE) && (number.ival < 0 || number.ival > 255))
          || ((type == T_SHORT) && (number.ival < -32768L || number.ival > 32767L)))
        THROW("Out of range");

      decl->value = number.ival;

      //fprintf(stderr, "TRANS_get_constant_value: INT: %ld\n", decl->value);
      break;

    case T_FLOAT: case T_SINGLE:

      if (type == T_SINGLE && !finite((float)number.dval))
        THROW("Out of range");

      decl->is_integer = FALSE;
      decl->value = index;
      break;

    case T_LONG:

      if (number.type == T_FLOAT)
        THROW("Type mismatch");

      decl->is_integer = FALSE;
      decl->value = index;
      decl->lvalue = number.lval;

      //fprintf(stderr, "TRANS_get_constant_value: LONG: %lld\n", decl->lvalue);
      break;

    case T_STRING:

			if (PATTERN_is(value, RS_LBRA))
			{
				value = *current++;
			  index = PATTERN_index(value);
				value = *current++;
				if (!PATTERN_is(value, RS_RBRA))
					THROW("Syntax error");
				TYPE_set_id(&decl->type, T_CSTRING);
			}

      decl->is_integer = FALSE;
      decl->value = index;
      break;

    default:

      THROW("Bad constant type");
  }
  
  return current;
}



PUBLIC void TRANS_want(int reserved, char *msg)
{
  if (!PATTERN_is(*JOB->current, reserved))
    THROW("Syntax error. &1 expected", msg ? msg : COMP_res_info[reserved].name);
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


PUBLIC bool TRANS_is_end_function(bool is_proc, PATTERN *look)
{
  if (PATTERN_is_newline(*look))
    return TRUE;

  if (is_proc)
    return PATTERN_is(*look, RS_PROCEDURE) || PATTERN_is(*look, RS_SUB);
  else
    return PATTERN_is(*look, RS_FUNCTION);
}
