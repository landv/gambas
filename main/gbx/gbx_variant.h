/***************************************************************************

  gbx_variant.h

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

#ifndef __VARIANT_H
#define __VARIANT_H

#include "gbx_type.h"
#include "gbx_string.h"
#include "gbx_object.h"

// On ARM 32 bits, sizeof(VARIANT) = 12, so it must be packed like GB_VARIANT

typedef
  struct 
	{
    TYPE type;
		union {
			char _boolean;
			unsigned char _byte;
			short _short;
			int _integer;
			int64_t _long;
			float _single;
			double _float;
			GB_DATE_VALUE _date;
			char *_string;
			void *_object;
			void *_pointer;
			int64_t data;
			}
		value;
  }
  PACKED
  VARIANT;

#define VARIANT_copy_value(_dst, _src) (_dst)->value.data = (_src)->value.data

#define VARIANT_undo(_value) \
({ \
	if ((_value)->type == T_VARIANT) \
		VALUE_undo_variant(_value); \
})

#define VARIANT_free(_var) \
({ \
  if ((_var)->type == T_STRING) \
  { \
    STRING_unref(&(_var)->value._string); \
  } \
  else if (TYPE_is_object((_var)->type)) \
  { \
    OBJECT_UNREF((_var)->value._object); \
  } \
})

#define VARIANT_keep(_var) \
({ \
  if ((_var)->type == T_STRING) \
  { \
    STRING_ref((_var)->value._string); \
  } \
  else if (TYPE_is_object((_var)->type)) \
  { \
    OBJECT_REF((_var)->value._object); \
  } \
})

#define VARIANT_is_null(_var) \
  (((_var)->type == T_NULL) || ((_var)->type == T_STRING && !(_var)->value._string) || (TYPE_is_object((_var)->type) && !(_var)->value._object))

#define VARIANT_clear(_var) \
({ \
  VARIANT_free(_var); \
	(_var)->type = 0; \
	(_var)->value.data = 0; \
})

#endif

