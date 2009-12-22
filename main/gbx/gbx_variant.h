/***************************************************************************

  variant.h

  Variant management routines

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#include "gbx_type.h"
#include "gbx_string.h"
#include "gbx_object.h"

typedef
  struct 
	{
    TYPE type;
		union
		{
			char *_string;
			void *_object;
			int64_t data;
		}
		value;
  }
  VARIANT;

static INLINE void VARIANT_undo(VALUE *val)
{
  if (val->type == T_VARIANT)
    VALUE_conv(val, val->_variant.vtype);
}

static INLINE void VARIANT_free(VARIANT *var)
{
  if (var->type == T_STRING)
  {
    STRING_unref(&var->value._string);
  }
  else if (TYPE_is_object(var->type))
  {
    OBJECT_UNREF(var->value._object, "VARIANT_free");
  }
}

static INLINE void VARIANT_keep(VARIANT *var)
{
  if (var->type == T_STRING)
  {
    STRING_ref(var->value._string);
  }
  else if (TYPE_is_object(var->type))
  {
    OBJECT_REF(var->value._object, "VARIANT_keep");
  }
}

static INLINE boolean VARIANT_is_null(VARIANT *var)
{
  if (var->type == T_NULL)
    return TRUE;

  if (var->type == T_STRING && !var->value._string)
    return TRUE;

  if (TYPE_is_object(var->type) && !var->value._object)
    return TRUE;

  return FALSE;
}

static INLINE void VARIANT_clear(VARIANT *var)
{
  VARIANT_free(var);

	var->type = 0;
	var->value.data = 0;
}

#endif
