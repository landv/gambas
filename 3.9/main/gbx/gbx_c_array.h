/***************************************************************************

  gbx_c_array.h

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

#ifndef __GBX_C_ARRAY_H
#define __GBX_C_ARRAY_H

#ifndef GBX_INFO

#include "gambas.h"

#include "gbx_variant.h"
#include "gbx_object.h"
#include "gbx_type.h"
#include "gbx_class.h"

// Do not forget to modify GB_ARRAY_BASE in gambas.h

typedef
	struct {
		OBJECT object;
		int size;
		int count;
		TYPE type;
		void *data;
		int *dim;
		void *ref;
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
extern GB_DESC NATIVE_PointerArray[];
extern GB_DESC NATIVE_SingleArray[];
extern GB_DESC NATIVE_FloatArray[];
extern GB_DESC NATIVE_StringArray[];
extern GB_DESC NATIVE_DateArray[];
extern GB_DESC NATIVE_VariantArray[];
extern GB_DESC NATIVE_ObjectArray[];
extern GB_DESC NATIVE_TemplateArray[];
extern GB_DESC NATIVE_TemplateArrayOfStruct[];
#else

#define THIS ((CARRAY *)_object)

#endif

void CARRAY_split(CARRAY *_object, const char *str, int lstr, const char *sep, const char *esc, bool no_void, bool keep_esc);
void CARRAY_reverse(void *_object, void *_param);
void CARRAY_get_value(CARRAY *_object, int index, VALUE *value);
#define CARRAY_invert(_array) CARRAY_reverse(_array, NULL)
void *CARRAY_get_data_multi(CARRAY *_object, GB_INTEGER *arg, int nparam);
void *CARRAY_out_of_bound();
CLASS *CARRAY_get_array_class(CLASS *class, CTYPE ctype);
int *CARRAY_get_array_bounds(CARRAY *_object);
void CARRAY_resize(CARRAY *_object, int size);

CARRAY *CARRAY_create_static(CLASS *class, void *ref, CLASS_ARRAY *desc, void *data);
int CARRAY_get_static_count(CLASS_ARRAY *desc);
size_t CARRAY_get_static_size(CLASS *class, CLASS_ARRAY *desc);
void CARRAY_release_static(CLASS *class, CLASS_ARRAY *desc, void *data);

#define CARRAY_get_data(_array, _index) \
({ \
	int __index = (_index); \
	CARRAY *__array = (CARRAY *)(_array); \
	void *__data; \
  if ((__index < 0) || (__index >= __array->count)) \
  	__data = CARRAY_out_of_bound(); \
  else \
 		__data = (void *)((char *)(__array->data) + __index * __array->size); \
 	__data; \
})

#define CARRAY_get_data_throw(_array, _index) \
({ \
	int __index = (_index); \
	CARRAY *__array = (CARRAY *)(_array); \
  if ((__index < 0) || (__index >= __array->count)) \
  	THROW(E_BOUND); \
	(void *)((char *)(__array->data) + __index * __array->size); \
})


#endif  // #ifndef __GBX_CLASS_INFO_C 

#define ARRAY_TEMPLATE_NDESC 21
#define ARRAY_OF_STRUCT_TEMPLATE_NDESC 13

#endif