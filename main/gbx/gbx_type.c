/***************************************************************************

  type.c

  Datatypes management routines

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __GBX_TYPE_C

#include <ctype.h>

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_limit.h"

#include "gbx_class.h"
#include "gambas.h"

#include "gbx_type.h"


PUBLIC char *TYPE_joker;

/* Permet de simplifier les tables de sauts associ�s aux types de donn�s */

#if 0
PUBLIC short TYPE_Index[] =
{
  0, /* T_NULL */
  1, /* T_BOOLEAN */
  2, /* T_BYTE */
  2, /* T_SHORT */
  2, /* T_INTEGER */
  3, /* T_LONG */
  4, /* T_FLOAT */
  5, /* T_DATE */
  6, /* T_STRING */
  6, /* T_CSTRING */
  7, /* T_FUNCTION */
  8, /* T_ARRAY */
  9  /* T_CLASS */
};
#endif


/* taille n�essaire au stockage des variables globales d'une classe */

PUBLIC size_t TYPE_sizeof(TYPE type)
{
  static size_t size[16] = { 0, 4, 4, 4, 4, 8, 8, 8, 8, 4, 4, 12, 0, 0, 0, 0 };

  if (TYPE_is_object(type))
    return sizeof(void *);
  else
    return size[type];
}


PUBLIC size_t TYPE_sizeof_native(TYPE type)
{
  static size_t size[16] =
  {
    0, 4, 4, 4, 4, 8, 8, 8, 8,
    sizeof(GB_STRING), sizeof(GB_STRING), 12, 0, 0, 0, 0
  };

  if (TYPE_is_object(type))
    return sizeof(void *);
  else
    return size[type];
}


PUBLIC size_t TYPE_sizeof_memory(TYPE type)
{
  static size_t size[16] = { 0, 1, 1, 2, 4, 8, 4, 8, 8, 4, 4, 12, 0, 0, 0, 0 };

  if (TYPE_is_object(type))
    return sizeof(void *);
  else
    return size[type];
}


PUBLIC const char *TYPE_get_name(TYPE type)
{
  static const char *name[17] =
  {
    "Void",
    "Boolean",
    "Byte",
    "Short",
    "Integer",
    "Long",
    "Single",
    "Float",
    "Date",
    "String",
    "String",
    "Variant",
    "Array",
    "Function",
    "Class",
    "Null",
    "Object"
  };


  if (TYPE_is_pure_object(type))
    return ((CLASS *)type)->name;
  else
    return name[type];
}


PUBLIC const char *TYPE_to_string(TYPE type)
{
  switch (type)
  {
    case T_BOOLEAN: return "b";
    case T_BYTE: return "c";
    case T_SHORT: return "h";
    case T_INTEGER: return "i";
    case T_LONG: return "l";
    case T_SINGLE: return "g";
    case T_FLOAT: return "f";
    case T_DATE: return "d";
    case T_STRING: return "s";
    case T_VARIANT: return "v";
    case T_OBJECT: return "o";

    default:
      if (TYPE_is_pure_object(type))
        return ((CLASS *)type)->name;
      else
        return "";

  }
}


PUBLIC void TYPE_signature_length(const char *sign, char *len_min, char *len_max, char *var)
{
  char c;
  int len = 0;
  boolean brace = FALSE;

  *len_min = 0;
  *len_max = 0;
  *var = 0;

  if (sign == NULL)
    return;

  for(;;)
  {
    c = *sign++;
    if (c == 0)
      break;

    if (c == '.')
    {
      *var = 1;
      break;
    }

    if (c == '[')
    {
      brace = TRUE;
      continue;
    }


    if (c == '\'' || c == '(' || c == '<')
    {
      for(;;)
      {
        c = *sign;
        if (c == 0)
          break;
        sign++;
        if (c == '\'' || c == ')' || c == '>')
          break;
      }
      continue;
    }

    if (c == ']')
      continue;

    if (!brace)
      *len_min = len + 1;

    if (islower(c))
    {
      len++;
      continue;
    }

    len++;

    for(;;)
    {
      c = *sign;
      if (c == 0)
        break;
      sign++;
      if (c == ';')
        break;
    }
  }

  *len_max = len;
}


PUBLIC TYPE TYPE_from_string(const char **ptype)
{
  const char *start;
  const char *type;
  boolean quote = FALSE;

  for(;;)
  {
    type = *ptype;

    if (type == NULL || *type == 0)
      return T_VOID;

    (*ptype)++;

    if (*type == '\'')
    {
      quote = !quote;
      continue;
    }

    if (*type == ')' || *type == '>')
    {
      quote = FALSE;
      continue;
    }

    if (quote)
      continue;

    if (*type == '(' || *type == '<')
    {
      quote = TRUE;
      continue;
    }

    if (index("[]<>", *type) == NULL)
      break;
  }

  switch(*type)
  {
    case 'b': return T_BOOLEAN;
    case 'c': return T_BYTE;
    case 'h': return T_SHORT;
    case 'i': return T_INTEGER;
    case 'l': return T_LONG;
    case 'g': return T_SINGLE;
    case 'f': return T_FLOAT;
    case 'd': return T_DATE;
    case 's': return T_STRING;
    case 'v': return T_VARIANT;
    case 'o': return T_OBJECT;

    default:

      start = type;

      while (*type != ';' && *type != 0)
        type++;

			if (*start == '*')
			{
				strcpy(COMMON_buffer, TYPE_joker);
				start++;
				if (type > start)
					strncat(COMMON_buffer, start, type - start);
			}
			else
			{
      	memcpy(COMMON_buffer, start, type - start);
      	COMMON_buffer[type - start] = 0;
			}

      *ptype = (char *)type + 1;
      return (TYPE)CLASS_find_global(NULL);
  }
}


PUBLIC TYPE *TYPE_transform_signature(TYPE **signature, const char *sign, int npmax)
{
  TYPE *tsign;
  int i;

  tsign = *signature;

  for (i = 0; i < npmax; i++)
    tsign[i] = TYPE_from_string(&sign);

  *signature += npmax;

  return tsign;
}


PUBLIC boolean TYPE_compare_signature(TYPE *sign1, int np1, TYPE *sign2, int np2)
{
  int i;

  if (np1 != np2)
    return TRUE;

  for (i = 0; i < np1; i++)
  {
    if (sign1[i] != sign2[i])
      return TRUE;
  }

  return FALSE;
}

