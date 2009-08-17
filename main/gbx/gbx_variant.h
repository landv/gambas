/***************************************************************************

  gbx_variant.h

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

#ifndef __VARIANT_H
#define __VARIANT_H


#include "gbx_string.h"
#include "gbx_object.h"

typedef
  struct {
    intptr_t type;
    char value[8];
    }
  VARIANT;

#define VARIANT_copy_value(_dst, _src) (*((int64_t *)((_dst)->value)) = *((int64_t *)((_src)->value)))

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
    OBJECT_UNREF(*(void **)var->value, "VARIANT_free");
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

static INLINE bool VARIANT_is_null(VARIANT *var)
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
  VARIANT_free(var);

	var->type = 0;
	*((int64_t *)var->value) = 0;
}

#endif
