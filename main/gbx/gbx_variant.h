/***************************************************************************

  variant.h

  Variant management routines

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

#ifndef __VARIANT_H
#define __VARIANT_H


#include "gbx_string.h"
#include "gbx_object.h"

typedef
  struct {
    long type;
    char value[8];
    }
  VARIANT;

#define VARIANT_copy(src, dst)  (*((long long *)dst) = *((long long *)src))

static INLINE void VARIANT_undo(VALUE *val)
{
  if (val->type == T_VARIANT)
    VALUE_conv(val, val->_variant.vtype);
}

static INLINE void VARIANT_free(VARIANT *var)
{
  if (var->type == T_STRING)
  {
    STRING_unref((char **)var->value);
  }
  else if (TYPE_is_object(var->type))
  {
    OBJECT_UNREF((void **)var->value, "VARIANT_free");
  }
}

static INLINE void VARIANT_keep(VARIANT *var)
{
  if (var->type == T_STRING)
  {
    STRING_ref(*((char **)var->value));
  }
  else if (TYPE_is_object(var->type))
  {
    OBJECT_REF(*((void **)var->value), "VARIANT_keep");
  }
}

static INLINE boolean VARIANT_is_null(VARIANT *var)
{
  if (var->type == T_NULL)
    return TRUE;

  if (var->type == T_STRING && *((char **)var->value) == NULL)
    return TRUE;

  if (TYPE_is_object(var->type) && *((void **)var->value) == NULL)
    return TRUE;

  return FALSE;
}


static INLINE void VARIANT_clear(VARIANT *var)
{
  long *p = (long *)var;

  VARIANT_free(var);

  p[0] = 0;
  p[1] = 0;
  p[2] = 0;
}



#endif
