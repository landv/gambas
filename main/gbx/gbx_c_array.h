/***************************************************************************

  CArray.h

  Native Array classes

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

#ifndef __GBX_C_ARRAY_H
#define __GBX_C_ARRAY_H

#include "gambas.h"

#include "gbx_variant.h"
#include "gbx_object.h"

typedef
  struct {
    OBJECT object;
    TYPE type;
    void *data;
    int *dim;
    }
  CARRAY;

#ifndef __GBX_C_ARRAY_C
extern GB_DESC NATIVE_ArrayBounds[];
extern GB_DESC NATIVE_Array[];
extern GB_DESC NATIVE_BooleanArray[];
extern GB_DESC NATIVE_ByteArray[];
extern GB_DESC NATIVE_ShortArray[];
extern GB_DESC NATIVE_IntegerArray[];
extern GB_DESC NATIVE_LongArray[];
extern GB_DESC NATIVE_SingleArray[];
extern GB_DESC NATIVE_FloatArray[];
extern GB_DESC NATIVE_StringArray[];
extern GB_DESC NATIVE_DateArray[];
extern GB_DESC NATIVE_VariantArray[];
extern GB_DESC NATIVE_ObjectArray[];
extern GB_DESC NATIVE_TemplateArray[];
#else

#define THIS ((CARRAY *)_object)

#endif

#define ARRAY_TEMPLATE_NDESC 13

/*CARRAY *CLIST_create(void);*/
void CARRAY_split(CARRAY *_object, const char *str, int lstr, const char *sep, const char *esc, bool many_esc);
void CARRAY_reverse(void *_object, void *_param);
void CARRAY_get_value(CARRAY *_object, int index, VALUE *value);
#define CARRAY_invert(_array) CARRAY_reverse(_array, NULL)

#endif
