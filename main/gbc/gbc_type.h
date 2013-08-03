/***************************************************************************

  gbc_type.h

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

#ifndef __GBC_TYPE_H
#define __GBC_TYPE_H

#define PROJECT_COMP
#include "gb_type_common.h"

/***************************************************

  Format d'un Type

  F K T T X X X X

  F : Flags (TF_*)
  K : Kind (TK_*)
  T : Type (T_*)
  X : index, pour T_OBJECT, T_ARRAY, T_STRUCT

***************************************************/

/*
typedef
  ulong TYPE;
*/

typedef
  unsigned char TYPE_ID;

typedef
  union {
    struct {
      unsigned char flag;
      TYPE_ID id;
      short value;
      } t;
    int l;
    }
  TYPE;

typedef
  struct {
    int value;
    TYPE type;
    }
  VALUE;

#ifndef __GBC_TYPE_C
EXTERN char *TYPE_name[]; 
#endif
  
/*#define TYPE_is_const(type)      (((type) >> 24) & TF_CONST)*/
#define TYPE_is_static(type)     ((type).t.flag & TF_STATIC)
#define TYPE_is_public(type)     ((type).t.flag & TF_PUBLIC)

#define TYPE_is_optional(type)   ((type).t.flag & TF_OPTIONAL)
/*#define TYPE_is_output(type)     (((type) >> 24) & TF_OUTPUT)*/

#define TYPE_is_array(type)      (TYPE_get_id(type) == T_ARRAY)
#define TYPE_is_object(type)     ((TYPE_get_id(type) == T_OBJECT) && (TYPE_get_value(type) >= 0))

#define TYPE_get_value(type)     ((type).t.value)
#define TYPE_get_kind(type)      ((type).t.flag & 0x7)
#define TYPE_get_id(type)        ((type).t.id)
#define TYPE_is_null(type)       ((type).l == 0)

#define TYPE_set_value(type, _value)     ((type)->t.value = (_value))
#define TYPE_set_id(type, _id)           ((type)->t.id = (_id))
#define TYPE_set_kind(type, _kind)       ((type)->t.flag |= (_kind))
#define TYPE_set_flag(type, _flag)       ((type)->t.flag |= (_flag))
#define TYPE_clear(type)                 ((type)->l = 0)

#define TYPE_can_be_long(type)  (TYPE_get_id(type) <= T_LONG)

#define TYPE_compare(_t1, _t2) ((_t1)->t.id == (_t2)->t.id && (_t1)->t.value == (_t2)->t.value)

/*PUBLIC long TYPE_get_class(TYPE type);*/
TYPE TYPE_make(TYPE_ID id, short value, int flag);
char *TYPE_get_desc(TYPE type);
const char *TYPE_get_short_desc(TYPE type);
size_t TYPE_sizeof(TYPE type);

#endif


