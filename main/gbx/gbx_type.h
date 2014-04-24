/***************************************************************************

  gbx_type.h

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
  CTYPE;

#define CTYPE_is_static(type)     ((type).flag & TF_STATIC)
#define CTYPE_is_public(type)     ((type).flag & TF_PUBLIC)
#define CTYPE_get_kind(type)      ((type).flag & 0x7)

// If type > T_OBJECT, then type is a pointer to the class

typedef
  uintptr_t TYPE;

typedef
  void (*TYPE_FUNC)();

typedef
  TYPE_FUNC TYPE_JUMP[T_OBJECT];


#ifndef __GBX_TYPE_C
EXTERN void *TYPE_joker;
EXTERN const size_t TYPE_sizeof_memory_tab[];
#endif

#define TYPE_is_void(type)         ((type) == T_VOID)
#define TYPE_is_null(type)         ((type) == T_NULL)
#define TYPE_is_object(type)       ((type) >= T_OBJECT)
#define TYPE_is_object_null(type)  ((type) >= T_NULL)
#define TYPE_is_pure_object(type)  ((type) > T_OBJECT)
#define TYPE_is_boolean(type)      ((type) == T_BOOLEAN)
#define TYPE_is_integer(type)      ((type) >= T_BOOLEAN && (type) <= T_INTEGER)
#define TYPE_is_integer_long(type) ((type) >= T_BOOLEAN && (type) <= T_LONG)
#define TYPE_is_long(type)         ((type) == T_LONG)
#define TYPE_is_single(type)       ((type) == T_SINGLE)
#define TYPE_is_float(type)        ((type) == T_FLOAT)
#define TYPE_is_variant(type)      ((type) == T_VARIANT)
#define TYPE_is_number(type)       ((type) >= T_BOOLEAN && (type) <= T_FLOAT)
#define TYPE_is_number_date(type)  ((type) >= T_BOOLEAN && (type) <= T_DATE)
#define TYPE_is_string(type)       ((type) == T_STRING || (type) == T_CSTRING)
#define TYPE_is_function(type)     ((type) == T_FUNCTION)
#define TYPE_is_pointer(type)      ((type) == T_POINTER)

#define TYPE_are_objects(_t1, _t2) (TYPE_is_object(_t1) && TYPE_is_object(_t2))
//#define TYPE_are_not_objects(_t1, _t2) (((_t1) | (_t2)) < T_OBJECT)

size_t TYPE_sizeof(TYPE type);
#define TYPE_sizeof_memory(_type) (TYPE_is_object(_type) ? sizeof(void *) : TYPE_sizeof_memory_tab[_type])
#define TYPE_is_value(_type) (TYPE_is_object(_type) || TYPE_is_null(_type) || TYPE_sizeof_memory_tab[_type] > 0)

const char *TYPE_get_name(TYPE type);

TYPE TYPE_from_string(const char **ptype);
const char *TYPE_to_string(TYPE type);
TYPE *TYPE_transform_signature(TYPE **signature, const char *sign, int nparam);
void TYPE_signature_length(const char *sign, char *len_min, char *len_max, char *var);
bool TYPE_are_compatible(TYPE type, TYPE ptype);
bool TYPE_compare_signature(TYPE *sign1, int np1, TYPE *sign2, int np2, bool check_compat);

#endif
