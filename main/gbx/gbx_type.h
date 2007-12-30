/***************************************************************************

  type.h

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

#ifndef __GBX_TYPE_H
#define __GBX_TYPE_H

#include "gb_type_common.h"

/* Types du compilateur */

#define MAX_TYPE 16

typedef
  struct {
    unsigned char flag;
    unsigned char id;
    short value;
    }
  PACKED
  CTYPE;

#define CTYPE_is_static(type)     ((type).flag & TF_STATIC)
#define CTYPE_is_public(type)     ((type).flag & TF_PUBLIC)
#define CTYPE_get_kind(type)      ((type).flag & 0x7)

/*
enum {
  TI_NULL,
  TI_BOOLEAN,
  TI_INTEGER,
  TI_LONG,
  TI_FLOAT,
  TI_DATE,
  TI_STRING,
  TI_FUNCTION,
  TI_ARRAY,
  TI_CLASS
  };
*/

/* Si type >= T_OBJECT, il s'agit de l'identificateur de classe */

typedef
  ulong TYPE;

typedef
  void (*TYPE_FUNC)();

typedef
  TYPE_FUNC TYPE_JUMP[T_OBJECT];


#ifndef __GBX_TYPE_C
EXTERN char *TYPE_joker;
#endif

#define TYPE_is_object(type)       ((type) >= T_OBJECT)
#define TYPE_is_pure_object(type)  ((type) > T_OBJECT)
#define TYPE_is_integer(type)      ((type) >= T_BOOLEAN && (type) <= T_INTEGER)
#define TYPE_is_integer_long(type) ((type) >= T_BOOLEAN && (type) <= T_LONG)
#define TYPE_is_long(type)         ((type) == T_LONG)
#define TYPE_is_float(type)        ((type) == T_FLOAT || (type) == T_SINGLE)
#define TYPE_is_variant(type)      ((type) == T_VARIANT)
#define TYPE_is_number(type)       ((type) >= T_BOOLEAN && (type) <= T_FLOAT)
#define TYPE_is_number_date(type)  ((type) >= T_BOOLEAN && (type) <= T_DATE)
#define TYPE_is_string(type)       ((type) == T_STRING || (type) == T_CSTRING)
#define TYPE_is_function(type)     ((type) == T_FUNCTION)
#define TYPE_is_null(type)         ((type) == T_NULL)
#define TYPE_is_object_null(type)  ((type) >= T_OBJECT || (type) == T_NULL)

PUBLIC size_t TYPE_sizeof(TYPE type);
PUBLIC size_t TYPE_sizeof_native(TYPE type);
PUBLIC size_t TYPE_sizeof_memory(TYPE type);

PUBLIC const char *TYPE_get_name(TYPE type);

PUBLIC TYPE TYPE_from_string(const char **ptype);
PUBLIC const char *TYPE_to_string(TYPE type);
PUBLIC TYPE *TYPE_transform_signature(TYPE **signature, const char *sign, int nparam);
PUBLIC boolean TYPE_compare_signature(TYPE *sign1, int np1, TYPE *sign2, int np2);
PUBLIC void TYPE_signature_length(const char *sign, char *len_min, char *len_max, char *var);

#endif
