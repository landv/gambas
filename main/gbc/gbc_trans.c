/***************************************************************************

  gbc_trans.c

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

int TRANS_in_affectation = 0;
//int TRANS_in_expression = 0;

void TRANS_reset(void)
{
  JOB->line = 1;
  JOB->current = JOB->pattern;
  JOB->end = &(JOB->pattern[JOB->pattern_count]);
}


bool TRANS_newline(void)
{
  if (PATTERN_is_newline(*JOB->current))
  {
    JOB->line = PATTERN_index(*JOB->current) + 1;
    JOB->current++;
    return TRUE;
  }

  return FALSE;
}

static bool read_integer(char **number, int base, int64_t *result)
{
  uint64_t nbr2, nbr;
  int d, n;
  unsigned char c;

  n = 0;
  nbr = 0;

  for(;;)
  {
	  c = (unsigned char)**number;

    if (c >= '0' && c <= '9')
      d = c - '0';
    else if (c >= 'A' && c <='Z')
      d = c - 'A' + 10;
    else if (c >= 'a' && c <='z')
      d = c - 'a' + 10;
    else
      break;

    if (d >= base)
      break;

	  (*number)++;
    n++;
    
    nbr2 = nbr * base + d;
    
		if (nbr2 < nbr || nbr2 > INT64_MAX)
			return TRUE;
    
    nbr = nbr2;
  }

  if (n == 0)
    return TRUE;

  *result = nbr;
  return FALSE;
}


bool TRANS_get_number(int index, TRANS_NUMBER *result)
{
  char car;
  int64_t val = 0;
  double dval = 0;
  char *end;
  int pos;
  bool long_int = FALSE;

  int base = 0;
  char *number = (char *)TABLE_get_symbol_name(JOB->class->table, index);
  bool minus = FALSE;
  bool is_unsigned = FALSE;

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
    if (read_integer(&number, base, &val) || *number)
    	return TRUE;
    
    long_int = (uint64_t)val != (uint)val;

    if (!is_unsigned)
    {
			if (val >= 0x8000L && val <= 0xFFFFL)
				val |= INT64_C(0xFFFFFFFFFFFF0000);
    }
  }
  else
  {
		if (is_unsigned)
			return TRUE;
    
    end = number;
    if (read_integer(&end, 10, &val) || *end)
    {
      errno = 0;
      base = 0;
      dval = strtod(number, &end);
      
      if (*end || errno)
      	return TRUE;
		}
		else
		{
			base = 10;
			long_int = (uint64_t)val != (uint)val;
		}
  }

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
    result->ival = (int)(minus ? (-val) : val);
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


int TRANS_get_class(PATTERN pattern)
{
  int index = PATTERN_index(pattern);
  int index_array;

  if (!CLASS_exist_class(JOB->class, index))
  {
		// Maybe a compound class?
		
		CLASS_SYMBOL *sym = CLASS_get_symbol(JOB->class, index);
		int i;
		char c;
		
		//fprintf(stderr, "TRANS_get_class: %.*s\n", sym->symbol.len, sym->symbol.name);
		
		for (i = sym->symbol.len - 1; i >= 0; i--)
		{
			c = sym->symbol.name[i];
			if (c == '[')
			{
				//fprintf(stderr, "TRANS_get_class: find %.*s\n", i, sym->symbol.name);
  			if (TABLE_find_symbol(JOB->class->table, sym->symbol.name, i, NULL, &index_array))
  			{
  				index_array = TRANS_get_class(PATTERN_make(RT_CLASS, index_array));
  				if (JOB->class->class[index_array].exported)
						return CLASS_add_class_exported(JOB->class, index);
  				else
						return CLASS_add_class(JOB->class, index);
				}
			}
		}
		
    THROW("Unknown identifier: &1", TABLE_get_symbol_name(JOB->class->table, index));
	}

  return CLASS_add_class(JOB->class, index);
}


bool TRANS_type(int mode, TRANS_DECL *result)
{
  PATTERN *look = JOB->current;
  short id;
  int value;
  int flag = 0;

  /* Do not fill the structure with zeros */

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

  if (!PATTERN_is_type(*look) && !PATTERN_is_class(*look))
    THROW(E_UNEXPECTED, READ_get_pattern(look));
    
	if (PATTERN_is_type(*look))
	{
		id = RES_get_type(PATTERN_index(*look));
		value = -1;
	}
	else
	{
    id = T_OBJECT;
    value = TRANS_get_class(*look);
	}
	
	if (PATTERN_is(look[1], RS_LSQR))
	{
		value = CLASS_get_array_class(JOB->class, id, value);
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
		//if (id == T_OBJECT)
		//	value = (-1);
		look++;
	}
  
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

bool TRANS_check_declaration(void)
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



PATTERN *TRANS_get_constant_value(TRANS_DECL *decl, PATTERN *current)
{
  int index;
  TRANS_NUMBER number;
  int type;
  PATTERN value;

  type = TYPE_get_id(decl->type);

	value = *current++;
  index = PATTERN_index(value);
	
	if (type == T_STRING)
	{
		if (PATTERN_is(value, RS_LBRA))
		{
			value = *current++;
			if (!PATTERN_is_string(value))
				THROW("Syntax error");
			index = PATTERN_index(value);
			value = *current++;
			if (!PATTERN_is(value, RS_RBRA))
				THROW("Syntax error");
			TYPE_set_id(&decl->type, T_CSTRING);
		}
		else
		{
			if (!PATTERN_is_string(value))
				THROW("Syntax error");
		}

		decl->is_integer = FALSE;
		decl->value = index;
	}
	else
	{
		if (PATTERN_is_string(value))
			THROW("Syntax error");
			
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
	
			default:
	
				THROW("Bad constant type");
		}
	}
  
  return current;
}



void TRANS_want(int reserved, char *msg)
{
  if (!PATTERN_is(*JOB->current, reserved))
    THROW("Syntax error. &1 expected", msg ? msg : COMP_res_info[reserved].name);
  JOB->current++;
}


bool TRANS_is(int reserved)
{
  if (PATTERN_is(*JOB->current, reserved))
  {
    JOB->current++;
    return TRUE;
  }
  else
    return FALSE;
}

void TRANS_ignore(int reserved)
{
  if (PATTERN_is(*JOB->current, reserved))
    JOB->current++;
}


bool TRANS_is_end_function(bool is_proc, PATTERN *look)
{
  if (PATTERN_is_newline(*look))
    return TRUE;

  if (is_proc)
    return PATTERN_is(*look, RS_PROCEDURE) || PATTERN_is(*look, RS_SUB);
  else
    return PATTERN_is(*look, RS_FUNCTION);
}

char *TRANS_get_num_desc(int num)
{
  static const char *num_desc[3] = { "first", "second", "third" };
  static char desc[6];

  if (num < 1)
    return NULL;

  if (num < 4)
    return (char *)num_desc[num - 1];

  snprintf(desc, sizeof(desc), "%dth", num);
  return desc;
}


