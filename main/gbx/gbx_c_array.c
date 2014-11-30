/***************************************************************************

  gbx_c_array.c

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

#define __GBX_C_ARRAY_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include <limits.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_array.h"
#include "gb_limit.h"
#include "gbx_class.h"

#include "gbx_exec.h"
#include "gbx_date.h"
#include "gbx_variant.h"
#include "gbx_compare.h"
#include "gbx_class.h"
#include "gbx_object.h"
#include "gbx_api.h"
#include "gbx_c_file.h"
#include "gbx_struct.h"
#include "gbx_c_array.h"

static bool _create_static_array;


static void THROW_static_array()
{
	GB_Error((char *)E_SARRAY);
	return;
}


static bool check_not_multi(CARRAY *_object)
{
	if (UNLIKELY(THIS->ref != NULL))
	{
		THROW_static_array();
		return TRUE;
	}
	else if (UNLIKELY(THIS->dim != NULL))
	{
		GB_Error("Array is multi-dimensional");
		return TRUE;
	}
	else
		return FALSE;
}

static bool check_start_length(int count, int *start, int *length)
{
	if (*start < 0)
	{
		GB_Error((char *)E_BOUND);
		return TRUE;
	}

	if (*start >= count)
		*length = 0;

	if (*length == -1)
		*length = count - *start;

	if (*length < 0 || (*start + *length) > count)
	{
		GB_Error((char *)E_BOUND);
		return TRUE;
	}
	
	return FALSE;
}

static int get_dim(CARRAY *_object)
{
	int d;

	if (THIS->dim == NULL)
		return 1;
	else
	{
		for(d = 0;; d++)
		{
			if (THIS->dim[d] < 0)
				return (d + 1);
		}
	}
}


static int get_bound(CARRAY *_object, int d)
{
	if (THIS->dim == NULL)
		return THIS->count;
	else
	{
		d = THIS->dim[d];
		if (d < 0)
			d = (-d);

		return d;
	}
}


CLASS *CARRAY_get_array_class(CLASS *class, CTYPE ctype)
{
	if (ctype.id == T_NULL)
	{
		if (TYPE_is_pure_object((TYPE)class))
			return CLASS_get_array_class(class);
		
		ctype.id = (unsigned char)(TYPE)class;
		ctype.value = -1;
	}
	
	switch (ctype.id)
	{
		case T_BOOLEAN: class = CLASS_BooleanArray; break;
		case T_BYTE: class = CLASS_ByteArray; break;
		case T_SHORT: class = CLASS_ShortArray; break;
		case T_INTEGER: class = CLASS_IntegerArray; break;
		case T_LONG: class = CLASS_LongArray; break;
		case T_SINGLE: class = CLASS_SingleArray; break;
		case T_FLOAT: class = CLASS_FloatArray; break;
		case T_STRING: class = CLASS_StringArray; break;
		case T_DATE: class = CLASS_DateArray; break;
		case T_POINTER: class = CLASS_PointerArray; break;
		
		case T_OBJECT:
			if (ctype.value >= 0)
				class = CLASS_get_array_class(class->load->class_ref[ctype.value]);
			else
				class = CLASS_ObjectArray; 
			break;
			
		case TC_STRUCT:
			class = CLASS_get_array_of_struct_class(class->load->class_ref[ctype.value]);
			break;
			
		default: 
			class = CLASS_VariantArray; 
			break;
	}

	return class;
}


void *CARRAY_out_of_bound()
{
	GB_Error((char *)E_BOUND);
	return NULL;
}


#define get_data_multi CARRAY_get_data_multi
#define get_data CARRAY_get_data

void *CARRAY_get_data_multi(CARRAY *_object, GB_INTEGER *arg, int nparam)
{
	int index;

	//fprintf(stderr, "get_data_multi: nparam = %d\n", nparam);

	if (UNLIKELY(THIS->dim != NULL))
	{
		int max;
		int i;
		int d;
		bool stop = FALSE;

		index = 0;
		nparam--;

		for (i = 0;; i++)
		{
			if (i > nparam)
				break;

			max = THIS->dim[i];
			if (max < 0)
			{
				max = (-max);
				stop = TRUE;
			}

			VALUE_conv_integer((VALUE *)&arg[i]);
			d = arg[i].value;

			if (d < 0 || d >= max)
			{
				GB_Error((char *)E_BOUND);
				return NULL;
			}

			index *= max;
			index += d;

			if (stop)
				break;
		}

		if (i != nparam)
		{
			GB_Error((char *)E_NDIM);
			return NULL;
		}
	}
	else
	{
		if (nparam != 1)
		{
			GB_Error((char *)E_NDIM);
			return NULL;
		}
		
		index = arg->value;

		if ((index < 0) || (index >= THIS->count))
		{
			GB_Error((char *)E_BOUND);
			return NULL;
		}
	}

	return (void *)((char *)(THIS->data) + index * THIS->size);
}


static int get_count(int *dim)
{
  int i, size;

  size = 1;

  for (i = 0;; i++)
  {
    size *= dim[i];
    if (size < 0)
    {
      size = (-size);
      break;
    }
  }
  
  return size;
}


static void release_one(CARRAY *_object, int i)
{
	if (THIS->type == T_STRING)
		STRING_unref(&(((char **)(THIS->data))[i]));
	else if (THIS->type == T_VARIANT)
		VARIANT_free(&(((VARIANT *)(THIS->data))[i]));
	else if (TYPE_is_object(THIS->type))
		OBJECT_UNREF(((void **)(THIS->data))[i]);
}

static void release_static(TYPE type, void *data, int start, int end)
{
	int i;

	if (type == T_STRING)
	{
		for (i = start; i < end; i++)
			STRING_unref(&((char **)data)[i]);
	}
	else if (type == T_VARIANT)
	{
		for (i = start; i < end; i++)
			VARIANT_free(&((VARIANT *)data)[i]);
	}
	else if (TYPE_is_object(type))
	{
		for (i = start; i < end; i++)
			OBJECT_UNREF(((void **)data)[i]);
	}
}


static void release(CARRAY *_object, int start, int end)
{
	if (end < 0)
		end = THIS->count;

	release_static(THIS->type, THIS->data, start, end);
}

static void borrow(CARRAY *_object, int start, int end)
{
	int i;

	if (end < 0)
		end = THIS->count;

	if (THIS->type == T_STRING)
	{
		for (i = start; i < end; i++)
			STRING_ref(((char **)(THIS->data))[i]);
	}
	else if (THIS->type == T_VARIANT)
	{
		for (i = start; i < end; i++)
			VARIANT_keep(&((VARIANT *)(THIS->data))[i]);
	}
	else if (TYPE_is_object(THIS->type))
	{
		for (i = start; i < end; i++)
			OBJECT_ref(((void **)(THIS->data))[i]);
	}
}


static void clear(CARRAY *_object)
{
	if (UNLIKELY(THIS->ref != NULL))
	{
		THROW_static_array();
		return;
	}
	
	release(THIS, 0, -1);
	if (UNLIKELY(THIS->dim != NULL))
	{
		memset(THIS->data, 0, THIS->size * THIS->count);
	}
	else
	{
		ARRAY_delete(&THIS->data);
		ARRAY_create_with_size(&THIS->data, THIS->size, 8);
		THIS->count = 0;
	}
}


static void *insert(CARRAY *_object, int index)
{
	THIS->count++;
	return ARRAY_insert(&THIS->data, index);
}


size_t CARRAY_get_static_size(CLASS *class, CLASS_ARRAY *desc)
{
  return (size_t)get_count(desc->dim) * CLASS_sizeof_ctype(class, desc->ctype);
}

int CARRAY_get_static_count(CLASS_ARRAY *desc)
{
  return get_count(desc->dim);
}


CARRAY *CARRAY_create_static(CLASS *class, void *ref, CLASS_ARRAY *desc, void *data)
{
	CARRAY *array;
	TYPE type;
	CLASS *aclass;
	
	type = CLASS_ctype_to_type(class, desc->ctype);
	_create_static_array = TRUE;
	aclass = CARRAY_get_array_class(class, desc->ctype);
	array = OBJECT_create_native(aclass, NULL);
	_create_static_array = FALSE;
	
	array->type = type;
	array->data = data;
	array->ref = ref;
	array->count = get_count(desc->dim);
	OBJECT_REF(ref);
	array->dim = desc->dim;
	array->size = CLASS_sizeof_ctype(class, desc->ctype);
	
	return array;
}

void CARRAY_release_static(CLASS *class, CLASS_ARRAY *desc, void *data)
{
	int count = get_count(desc->dim);
	
	if (desc->ctype.id == TC_STRUCT)
	{
		CLASS *sclass = class->load->class_ref[desc->ctype.value];
		int i;
		int size = CLASS_sizeof_ctype(class, desc->ctype);
		char *p = (char *)data;
		
		for (i = 0; i < count; i++)
		{
			OBJECT_release_static(sclass, sclass->load->dyn, sclass->load->n_dyn, p);
			p += size;
		}
	}
	else
		release_static(CLASS_ctype_to_type(class, desc->ctype), data, 0, count);
}

void CARRAY_get_value(CARRAY *_object, int index, VALUE *value)
{
	VALUE_read(value, get_data(THIS, index), THIS->type);
}

int *CARRAY_get_array_bounds(CARRAY *_object)
{
	return THIS->dim;
}


BEGIN_METHOD(Array_new, GB_INTEGER size)

	TYPE type;
	CLASS *klass;
	int max_size;
	int inc;
	GB_INTEGER *sizes = ARG(size);
	int nsize = GB_NParam() + 1;
	int i;
	
	if (_create_static_array)
		return;

	klass = OBJECT_class(THIS);

	type = (TYPE)klass->array_type;
	if (!type)
	{
		GB_Error("Bad array type");
		return;
	}

	/*if (TYPE_is_object(type))
		THIS->mode = T_OBJECT;
	else
		THIS->mode = (int)type;*/

	//printf("Array_new: type = %d nsize = %d\n", type, nsize);

	THIS->type = type;
	THIS->size = TYPE_sizeof_memory(type);
	max_size = INT_MAX / THIS->size;

	if (nsize <= 1)
	{
		int size = VARGOPT(size, 0);
		if (size > max_size)
			THROW(E_MEMORY);
		if (size < 0)
			size = 0;

		inc = (size / 8) & ~7;
		if (inc < 8)
			inc = 8;

		ARRAY_create_with_size(&THIS->data, THIS->size, inc);
		if (size > 0)
			ARRAY_add_many_void(&THIS->data, size);
		
		THIS->count = size;
	}
	else
	{
		if (nsize > MAX_ARRAY_DIM)
		{
			GB_Error("Too many dimensions");
			return;
		}

		uint64_t size = 1;
		for (i = 0; i < nsize; i++)
		{
			VALUE_conv_integer((VALUE *)&sizes[i]);
			if (sizes[i].value < 1)
			{
				GB_Error("Bad dimension");
				return;
			}
			size *= sizes[i].value;
			if (size > max_size)
				THROW(E_MEMORY);
		}

		ALLOC_ZERO(&THIS->dim, nsize * sizeof(int));

		for (i = 0; i < nsize; i++)
			THIS->dim[i] = sizes[i].value;
		THIS->dim[nsize - 1] = (-THIS->dim[nsize - 1]);

		ARRAY_create_with_size(&THIS->data, THIS->size, 8);
		ARRAY_add_many_void(&THIS->data, (int)size);
		
		THIS->count = size;
	}

	
END_METHOD


BEGIN_PROPERTY(Array_Type)

	GB_ReturnInteger(THIS->type);

END_PROPERTY

BEGIN_METHOD_VOID(Array_free)

	if (UNLIKELY(THIS->ref != NULL))
	{
		OBJECT_UNREF(THIS->ref);
		return;
	}
	
	release(THIS, 0, -1);
	
	ARRAY_delete(&THIS->data);

	FREE(&THIS->dim);

END_METHOD


BEGIN_METHOD_VOID(Array_Clear)

	clear(THIS);

END_METHOD


BEGIN_PROPERTY(Array_Data)

	GB_ReturnPointer(THIS->data);

END_PROPERTY


BEGIN_PROPERTY(Array_Count)

	GB_ReturnInt(THIS->count);

END_PROPERTY


BEGIN_PROPERTY(Array_Max)

	GB_ReturnInt(THIS->count - 1);

END_PROPERTY


static bool copy_remove(CARRAY *_object, int start, int length, bool copy, bool remove)
{
	CARRAY *array;
	int count = THIS->count;
	void *data;
	int i, nsize;

	if (check_not_multi(THIS) && (start != 0 || length != -1))
		return TRUE;

	if (check_start_length(count, &start, &length))
		return TRUE;

	if (copy)
	{
		GB_ArrayNew((GB_ARRAY *)POINTER(&array), THIS->type, 0);
	
		if (length > 0)
		{
			data = ARRAY_insert_many(&array->data, 0, length);
			array->count += length;
			memmove(data, get_data(THIS, start), length * THIS->size);
			borrow(array, 0, -1);
		}
		
		if (THIS->dim)
		{
			nsize = get_dim(THIS);
			ALLOC_ZERO(&array->dim, nsize * sizeof(int));

			for (i = 0; i < nsize; i++)
				array->dim[i] = THIS->dim[i];
		}
	}

	if (remove)
	{
		release(THIS, start, start + length);
		ARRAY_remove_many(&THIS->data, start, length);
		THIS->count -= length;
	}
	
	if (copy)
		GB_ReturnObject(array);
		
	return FALSE;
}

BEGIN_METHOD(Array_Remove, GB_INTEGER index; GB_INTEGER length)

	copy_remove(THIS, VARG(index), VARGOPT(length, 1), FALSE, TRUE);

END_METHOD


BEGIN_METHOD(Array_Copy, GB_INTEGER start; GB_INTEGER length)

	int start, length;
	
	if (MISSING(start) && MISSING(length))
	{
		start = 0;
		length = -1;
	}
	else
	{
		start = VARGOPT(start, 0);
		length = VARGOPT(length, 1);
	}

	copy_remove(THIS, start, length, TRUE, FALSE);

END_METHOD


BEGIN_METHOD(Array_Extract, GB_INTEGER start; GB_INTEGER length)

	copy_remove(THIS, VARG(start), VARGOPT(length, 1), TRUE, TRUE);

END_METHOD


void CARRAY_resize(CARRAY *_object, int size)
{
	int count = THIS->count;

	if (size > count)
	{
		ARRAY_add_many_void(&THIS->data, size - count);
	}
	else if ((size < count) && (size >= 0))
	{
		release(THIS, size, -1);
		ARRAY_remove_many(&THIS->data, size, count - size);
	}

	THIS->count = size;
}


BEGIN_METHOD(Array_Resize, GB_INTEGER size)

	int size = VARG(size);

	if (check_not_multi(THIS))
		return;

	CARRAY_resize(THIS, size);
	
END_METHOD


static void add(CARRAY *_object, GB_VALUE *value, int index)
{
	void *data;

	if (check_not_multi(THIS))
		return;

	data = insert(THIS, index);
	/*GB_Conv(value, THIS->type);*/
	GB_Store(THIS->type, value, data);
}


#define IMPLEMENT_add(_type, _gtype) \
BEGIN_METHOD(Array_##_type##_Add, GB_##_gtype value; GB_INTEGER index) \
\
	add(THIS, (GB_VALUE *)(void *)ARG(value), VARGOPT(index, -1)); \
	\
END_METHOD \
\
BEGIN_METHOD(Array_##_type##_Push, GB_##_gtype value) \
\
	add(THIS, (GB_VALUE *)(void *)ARG(value), -1); \
	\
END_METHOD

IMPLEMENT_add(Integer, INTEGER)
IMPLEMENT_add(Long, LONG)
IMPLEMENT_add(Float, FLOAT)
IMPLEMENT_add(Single, SINGLE)
IMPLEMENT_add(Date, DATE)
IMPLEMENT_add(String, STRING)
IMPLEMENT_add(Object, OBJECT)
IMPLEMENT_add(Variant, VARIANT)

BEGIN_METHOD(Array_Add, GB_VARIANT value; GB_INTEGER index)
	
	GB_VALUE *value = (GB_VALUE *)(void *)ARG(value);
	GB_Conv(value, THIS->type);	
	add(THIS, value, VARGOPT(index, -1));
	
END_METHOD

BEGIN_METHOD(Array_Push, GB_VARIANT value)

	GB_VALUE *value = (GB_VALUE *)(void *)ARG(value);
	GB_Conv(value, THIS->type);	
	add(THIS, value, -1);
	
END_METHOD


#define IMPLEMENT_put(_type, _gtype) \
BEGIN_METHOD(Array_##_type##_put, GB_##_gtype value; GB_INTEGER index) \
	\
	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1); \
	if (!data) return; \
	GB_Store(GB_T_##_gtype, (GB_VALUE *)(void *)ARG(value), data); \
	\
END_METHOD

#define IMPLEMENT_put2(_type, _gtype, _gstore) \
BEGIN_METHOD(Array_##_type##_put, GB_##_gtype value; GB_INTEGER index) \
\
	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1); \
	if (!data) return; \
	GB_Store(GB_T_##_gstore, (GB_VALUE *)(void *)ARG(value), data); \
	\
END_METHOD

BEGIN_METHOD(Array_put, GB_VARIANT value; GB_INTEGER index)
	
	GB_VALUE *value;
	void *data;
	
	value = (GB_VALUE *)(void *)ARG(value);
	GB_Conv(value, THIS->type);	
	
	data = get_data_multi(THIS, ARG(index), GB_NParam() + 1);
	if (!data) return;
	
	GB_Store(THIS->type, value, data);
	
END_METHOD

IMPLEMENT_put(Integer, INTEGER)
IMPLEMENT_put2(Short, INTEGER, SHORT)
IMPLEMENT_put2(Byte, INTEGER, BYTE)
IMPLEMENT_put2(Boolean, INTEGER, BOOLEAN)
IMPLEMENT_put(Long, LONG)
IMPLEMENT_put(Float, FLOAT)
IMPLEMENT_put(Single, SINGLE)
IMPLEMENT_put(Date, DATE)
IMPLEMENT_put(String, STRING)
IMPLEMENT_put(Object, OBJECT)
IMPLEMENT_put(Variant, VARIANT)


BEGIN_METHOD(Array_Fill, GB_VALUE value; GB_INTEGER start; GB_INTEGER length)

	int count = THIS->count;
	int start = VARGOPT(start, 0);
	int length = VARGOPT(length, count);
	int i;
	void *data;
	int size;
	
	if (check_start_length(count, &start, &length))
		return;

	VALUE_conv((VALUE *)ARG(value), THIS->type);

	data = get_data(THIS, start);
	size = THIS->size;

	for (i = 0; i < length; i++)
	{
		GB_Store(THIS->type, (GB_VALUE *)ARG(value), data);
		data += size;
	}

END_METHOD


BEGIN_METHOD(Array_Insert, GB_OBJECT array; GB_INTEGER pos)

	int pos = VARGOPT(pos, -1);
	CARRAY *array = (CARRAY *)VARG(array);
	void *data;
	int count;

	if (GB_CheckObject(array))
		return;

	if (check_not_multi(THIS))
	{
		GB_ReturnNull();
		return;
	}

	count = array->count;
	
	if (count > 0)
	{
		if (pos < 0)
			pos = THIS->count;

		data = ARRAY_insert_many(&THIS->data, pos, count);
		THIS->count += count;
		borrow(array, 0, -1);
		memmove(data, array->data, count * THIS->size);
	}
	
	GB_ReturnObject(THIS);

END_METHOD


BEGIN_METHOD_VOID(Array_Pop)

	int index = THIS->count - 1;

	if (check_not_multi(THIS))
		return;

	if (index < 0)
	{
		GB_Error((char *)E_ARG);
		return;
	}

	GB_ReturnPtr(THIS->type, get_data(THIS, index));

	BORROW(&TEMP);
	release_one(THIS, index);
	ARRAY_remove(&THIS->data, index);
	THIS->count--;
	UNBORROW(&TEMP);

END_METHOD


BEGIN_METHOD(Array_get, GB_INTEGER index)

	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1);

	if (data)
		GB_ReturnPtr(THIS->type, data);

END_METHOD

#if 0
BEGIN_PROPERTY(Array_Last)

	void *data;
	int index = THIS->count - 1;

	if (check_not_multi(THIS))
		return;

	if (index < 0)
	{
		GB_Error((char *)E_ARG);
		return;
	}

	data = get_data(THIS, index);
	
	if (READ_PROPERTY)
	{
		GB_ReturnPtr(THIS->type, data);
	}
	else
	{
		GB_VALUE *value;
		
		value = (GB_VALUE *)(void *)ARG(value);
		GB_Conv(value, THIS->type);	
		
		if (!data) return;
		
		GB_Store(THIS->type, value, data);
	}

END_PROPERTY
#endif

BEGIN_METHOD_VOID(Array_next)

	int *index = (int *)GB_GetEnum();

	if (*index >= THIS->count)
		GB_StopEnum();
	else
	{
		GB_ReturnPtr(THIS->type, get_data(THIS, *index));
		(*index)++;
	}

END_METHOD

BEGIN_METHOD(Array_Sort, GB_INTEGER mode)

	COMPARE_FUNC compare;
	int mode = VARGOPT(mode, 0);
	void *data = THIS->data;

	if (THIS->count > 1)
	{
		compare = COMPARE_get(THIS->type, mode);
		qsort(data, THIS->count, THIS->size, compare);
	}

	GB_ReturnObject(THIS);

END_METHOD


static int find(CARRAY *_object, int mode, void *value, int start)
{
	COMPARE_FUNC compare = COMPARE_get(THIS->type, mode);
	int i;

	if (start < 0)
		start = 0;
	else if (start >= THIS->count)
		return (-1);
	
	for (i = start; i < THIS->count; i++)
	{
		if ((*compare)(get_data(THIS, i), value) == 0)
			return i;
	}

	return (-1);
}

static int find_string(CARRAY *_object, int mode, const char *value, int len_value, int start)
{
	char **data;
	char *str;
	int i;
	int len;
	
	//fprintf(stderr, "find_string: %p %d: %.*s | %s\n", THIS, THIS->count, len_value, value, DEBUG_get_current_position());
	
	if (start < 0)
		start = 0;
	else if (start >= THIS->count)
		return (-1);
	
	data = ((char **)THIS->data);
	
	if (mode == GB_COMP_BINARY)
	{
		for (i = start; i < THIS->count; i++)
		{
			str = data[i];
			len = STRING_length(str);
			if (STRING_equal(str, len, value, len_value))
				return i;
		}
	}
	else if (mode == GB_COMP_NOCASE)
	{
		for (i = start; i < THIS->count; i++)
		{
			str = data[i];
			len = STRING_length(str);
			if (STRING_equal_ignore_case(str, len, value, len_value))
				return i;
		}
	}
	else
	{
		COMPARE_FUNC compare = COMPARE_get(THIS->type, mode);
		
		for (i = start; i < THIS->count; i++)
		{
			if ((*compare)(&data[i], &value) == 0)
				return i;
		}
	}

	return (-1);
}

#define IMPLEMENT_find(_type, _gtype) \
BEGIN_METHOD(Array_##_type##_Find, _gtype value; GB_INTEGER start) \
\
	GB_ReturnInt(find(THIS, 0, &VARG(value), VARGOPT(start, 0))); \
\
END_METHOD \
BEGIN_METHOD(Array_##_type##_Exist, _gtype value) \
\
	GB_ReturnBoolean(find(THIS, 0, &VARG(value), 0) >= 0); \
\
END_METHOD

IMPLEMENT_find(Integer, GB_INTEGER)
/*IMPLEMENT_find(short, GB_INTEGER)
IMPLEMENT_find(byte, GB_INTEGER)*/
IMPLEMENT_find(Long, GB_LONG)
IMPLEMENT_find(Float, GB_FLOAT)
IMPLEMENT_find(Single, GB_SINGLE)
IMPLEMENT_find(Date, GB_DATE)
IMPLEMENT_find(Variant, GB_VARIANT)

static int find_object(CARRAY *_object, void *value, int start, bool byref)
{
	int i;
	void **data;

	if (start < 0)
		start = 0;
	else if (start >= THIS->count)
		return (-1);

	data = get_data(THIS, 0);
	
	if (byref)
	{
		for (i = 0; i < THIS->count; i++)
		{
			if (data[i] == value)
				return i;
		}
	}
	else
	{
		for (i = 0; i < THIS->count; i++)
		{
			//if (*((void **)get_data(THIS, i)) == value)
			if (!COMPARE_object(&data[i], &value))
				return i;
		}
	}

	return (-1);
}

BEGIN_METHOD(Array_Object_Find, GB_OBJECT value; GB_INTEGER start)

	GB_ReturnInt(find_object(THIS, VARG(value), VARGOPT(start, 0), FALSE));

END_METHOD

BEGIN_METHOD(Array_Object_FindByRef, GB_OBJECT value; GB_INTEGER start)

	GB_ReturnInt(find_object(THIS, VARG(value), VARGOPT(start, 0), TRUE));

END_METHOD

BEGIN_METHOD(Array_Object_Exist, GB_OBJECT value)

	GB_ReturnBoolean(find_object(THIS, VARG(value), 0, FALSE) >= 0);

END_METHOD

BEGIN_METHOD(Array_Object_ExistByRef, GB_OBJECT value)

	GB_ReturnBoolean(find_object(THIS, VARG(value), 0, TRUE) >= 0);

END_METHOD


BEGIN_METHOD(Array_String_Find, GB_STRING value; GB_INTEGER mode; GB_INTEGER start)

	GB_ReturnInteger(find_string(THIS, VARGOPT(mode, GB_COMP_BINARY), STRING(value), LENGTH(value), VARGOPT(start, 0)));

END_METHOD

BEGIN_METHOD(Array_String_Exist, GB_STRING value; GB_INTEGER mode)

	//fprintf(stderr, "%s\n", DEBUG_get_current_position());
	GB_ReturnBoolean(find_string(THIS, VARGOPT(mode, GB_COMP_BINARY), STRING(value), LENGTH(value), 0) >= 0);

END_METHOD


BEGIN_METHOD(Array_String_join, GB_STRING sep; GB_STRING esc)

	char *sep = ",";
	uint lsep = 1;
	char *esc = "";
	uint lesc = 0;
	char escl, NO_WARNING(escr);
	int i;
	char **data = (char **)THIS->data;
	char *p, *p2;
	int l, max;
	
	if (!MISSING(sep))
	{
		sep = STRING(sep);
		lsep = LENGTH(sep);
	}

	if (!MISSING(esc))
	{
		esc = STRING(esc);
		lesc = LENGTH(esc);

		if (lesc == 1)
			escl = escr = esc[0];
		else if (lesc >= 2)
		{
			escl = esc[0];
			escr = esc[1];
		}
	}

	if (lesc == 0)
	{
		max = 0;
		for (i = 0; i < THIS->count; i++)
			max += STRING_length(data[i]) + lsep;
		if (THIS->count)
			max -= lsep;
		
		STRING_start_len(max);

		for (i = 0; i < THIS->count; i++)
		{
			p = data[i];
			l = STRING_length(data[i]);

			if (i)
				STRING_make(sep, lsep);
			if (l)
				STRING_make(p, l);
		}
	}
	else if (*sep && escr == *sep)
	{
		STRING_start();

		for (i = 0; i < THIS->count; i++)
		{
			p = data[i];
			l = STRING_length(data[i]);

			if (i)
				STRING_make(sep, lsep);

			if (l == 0)
				continue;

			for(;;)
			{
				p2 = index(p, escr);
				if (p2)
				{
					STRING_make(p, p2 - p);
					STRING_make_char(escl);
					STRING_make_char(escr);
					p = p2 + 1;
				}
				else
				{
					STRING_make(p, l + data[i] - p);
					break;
				}
			}
		}
	}
	else
	{
		STRING_start();

		for (i = 0; i < THIS->count; i++)
		{
			p = data[i];
			l = STRING_length(data[i]);

			if (i)
				STRING_make(sep, lsep);

			if (l == 0)
				continue;

			STRING_make_char(escl);
			for(;;)
			{
				p2 = index(p, escr);
				if (p2)
				{
					STRING_make(p, p2 - p + 1);
					STRING_make_char(escr);
					p = p2 + 1;
				}
				else
				{
					STRING_make(p, l + data[i] - p);
					break;
				}
			}
			STRING_make_char(escr);
		}
	}

	GB_ReturnString(STRING_end_temp());

END_METHOD


BEGIN_METHOD_VOID(CARRAY_reverse)

	size_t size;
	int count;
	char *buffer[16];
	char *pi, *pj;

	count = THIS->count;
	if (count > 1)
	{
		size = THIS->size;
		pi = get_data(THIS, 0);
		pj = get_data(THIS, count - 1);

		do
		{
			memcpy(buffer, pi, size);
			memcpy(pi, pj, size);
			memcpy(pj, buffer, size);
			pi += size;
			pj -= size;
		}
		while (pi < pj);
	}
	
	GB_ReturnObject(THIS);

END_METHOD


BEGIN_METHOD(Array_Read, GB_OBJECT file; GB_INTEGER start; GB_INTEGER length)

	int count = THIS->count;
	int start = VARGOPT(start, 0);
	int length = VARGOPT(length, count);

	if (check_start_length(count, &start, &length))
		return;

	STREAM_read(CSTREAM_stream(VARG(file)), get_data(THIS, start), length * THIS->size);

END_METHOD


BEGIN_METHOD(Array_Write, GB_OBJECT file; GB_INTEGER start; GB_INTEGER length)

	int count = THIS->count;
	int start = VARGOPT(start, 0);
	int length = VARGOPT(length, count);

	if (check_start_length(count, &start, &length))
		return;

	STREAM_write(CSTREAM_stream(VARG(file)), get_data(THIS, start), length * THIS->size);

END_METHOD


BEGIN_PROPERTY(Array_Dim)

	GB_ReturnInteger(get_dim(THIS));

END_PROPERTY


BEGIN_METHOD(Array_Bounds_get, GB_INTEGER index)

	int dim = get_dim(THIS);
	int index = VARG(index);

	if (index < 0 || index >= dim)
	{
		GB_Error((char *)E_BOUND);
		return;
	}

	GB_ReturnInteger(get_bound(THIS, index));

END_PROPERTY

BEGIN_METHOD(Array_Byte_ToString, GB_INTEGER start; GB_INTEGER length)

	int start, length;
	int count = THIS->count;
	char *p;
	
	start = VARGOPT(start, 0);
	
	if (start < 0)
	{
		GB_Error((char *)E_BOUND);
		return;
	}

	if (start >= count)
	{
		GB_ReturnVoidString();
		return;
	}

	length = VARGOPT(length, -1);
	if (length < 0)
	{
		p = memchr((char *)THIS->data + start, 0, count - start);
		if (!p)
			length = count - start;
		else
			length = p - ((char *)THIS->data + start);
	}
	else
	{
		if ((start + length) > count)
			length = count - start;
	}
	
	GB_ReturnNewString((char *)THIS->data + start, length);

END_METHOD


BEGIN_METHOD(Array_Byte_FromString, GB_STRING string)

	CARRAY *array;
	char *string = STRING(string);
	int length = LENGTH(string);
	
	GB_ArrayNew((GB_ARRAY *)POINTER(&array), T_BYTE, length);
	memcpy(array->data, string, length);
	GB_ReturnObject(array);

END_METHOD


BEGIN_METHOD(ArrayOfStruct_get, GB_INTEGER index)

	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1);

	if (data)
		GB_ReturnObject(CSTRUCT_create_static(THIS, (CLASS *)THIS->type, data));

END_METHOD


BEGIN_METHOD(ArrayOfStruct_put, GB_OBJECT value; GB_INTEGER index)

	int i;
	CLASS *class = (CLASS *)THIS->type;
	char *addr;
	void *object = VARG(value);
	void *data;
	VALUE temp;
	CLASS_DESC *desc;
	
	if (GB_CheckObject(object))
		return;
	
	data = get_data_multi(THIS, ARG(index), GB_NParam() + 1);
	if (!data)
		return;
	
	for (i = 0; i < class->n_desc; i++)
	{
		desc = class->table[i].desc;
		
		if (((CSTRUCT *)object)->ref)
			addr = (char *)((CSTATICSTRUCT *)object)->addr + desc->variable.offset;
		else
			addr = (char *)object + sizeof(CSTRUCT) + desc->variable.offset;
	
		VALUE_class_read(desc->variable.class, &temp, (void *)addr, desc->variable.ctype, object);

		addr = (char *)data + desc->variable.offset;
		VALUE_write(&temp, (void *)addr, desc->variable.type);
	}

END_METHOD


#if 0
BEGIN_PROPERTY(ArrayOfStruct_Last)

	int index = THIS->count - 1;

	if (check_not_multi(THIS))
		return;

	if (index < 0)
	{
		GB_Error((char *)E_ARG);
		return;
	}

	void *data = get_data(THIS, index);

	if (data)
		GB_ReturnObject(CSTRUCT_create_static(THIS, (CLASS *)THIS->type, data));
	
END_PROPERTY
#endif

	
BEGIN_METHOD_VOID(ArrayOfStruct_next)

	int *index = (int *)GB_GetEnum();

	if (*index >= THIS->count)
		GB_StopEnum();
	else
	{
		GB_ReturnObject(CSTRUCT_create_static(THIS, (CLASS *)THIS->type, get_data(THIS, *index)));
		(*index)++;
	}

END_METHOD

static CARRAY *_converted_array;

static void error_convert()
{
	OBJECT_UNREF(_converted_array);
}

static bool _convert(CARRAY *src, CLASS *class, VALUE *conv)
{
	CARRAY *array;
	int i;
	void *data;
	VALUE temp;
	int dim;
	
	if (!src || !TYPE_is_pure_object((TYPE)class))
		return TRUE;
	
	CLASS_load(class); // Force creation of array classes
	
	if (!class->is_array) //CLASS_inherits(class, CLASS_Array))
		return TRUE;
	
	_converted_array = array = OBJECT_create(class, NULL, NULL, 0);
	
	if (src->count)
	{
		ARRAY_add_many_void(&array->data, src->count);
		array->count = src->count;
		
		ON_ERROR(error_convert)
		{
			for (i = 0; i < src->count; i++)
			{
				data = CARRAY_get_data(src, i);
				VALUE_read(&temp, data, src->type);
				BORROW(&temp);
				data = CARRAY_get_data(array, i);
				VALUE_write(&temp, data, array->type);
				RELEASE(&temp);
			}
		}
		END_ERROR
	}
	
	dim = get_dim(src);
	if (dim > 1)
	{
		ALLOC(&array->dim, dim * sizeof(int));
		for (i = 0; i < dim; i++)
			array->dim[i] = src->dim[i];
	}
	
	conv->_object.object = array;
	return FALSE;
}

#else

#include "gbx_c_array.h"

#define _convert NULL

#endif /* #ifndef GBX_INFO */


GB_DESC NATIVE_ArrayBounds[] =
{
	GB_DECLARE(".Array.Bounds", sizeof(CARRAY)), GB_NOT_CREATABLE(),

	GB_METHOD("_get", "i", Array_Bounds_get, "(Dimension)i"),
	GB_PROPERTY_READ("Count", "i", Array_Dim),

	GB_END_DECLARE
};


GB_DESC NATIVE_Array[] =
{
	GB_DECLARE("Array", sizeof(CARRAY)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, Array_free, NULL),

	GB_PROPERTY_READ("Type", "i", Array_Type),
	GB_PROPERTY_READ("Count", "i", Array_Count),
	GB_PROPERTY_READ("Max", "i", Array_Max),
	GB_PROPERTY_READ("Length", "i", Array_Count),
	GB_PROPERTY_READ("Dim", "i", Array_Dim),
	GB_PROPERTY_READ("Data", "p", Array_Data),
	GB_PROPERTY_SELF("Bounds", ".Array.Bounds"),

	GB_METHOD("Remove", NULL, Array_Remove, "(Index)i[(Length)i]"),
	GB_METHOD("Clear", NULL, Array_Clear, NULL),
	GB_METHOD("Resize", NULL, Array_Resize, "(Size)i"),

	//GB_METHOD("Add", NULL, Array_Add, "(Value)v[(Index)i]"),
	//GB_METHOD("Push", NULL, Array_Push, "(Value)v"),
	//GB_METHOD("_put", NULL, Array_put, "(Value)v(Index)i."),

	//GB_METHOD("Pop", "v", Array_Pop_variant, NULL), // Does not work
	//GB_METHOD("_get", "v", Array_get_variant, "(Index)i."),
	//GB_METHOD("_next", "v", Array_next_variant, NULL),
	//GB_PROPERTY_READ("Last", "v", Array_Last),

	/*GB_METHOD("Copy", "Array", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Array", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Array", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)v[(Start)i(Length)i]"),*/
	
	GB_INTERFACE("_convert", _convert),

	GB_END_DECLARE
};


GB_DESC NATIVE_BooleanArray[] =
{
	GB_DECLARE("Boolean[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Integer_Add, "(Value)b[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Integer_Push, "(Value)b"),
	GB_METHOD("_put", NULL, Array_Boolean_put, "(Value)b(Index)i."),
	GB_METHOD("Find", "i", Array_Integer_Find, "(Value)b[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Integer_Exist, "(Value)b"),

	GB_METHOD("Pop", "b", Array_Pop, NULL),
	GB_METHOD("_get", "b", Array_get, "(Index)i."),
	GB_METHOD("_next", "b", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "b", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Boolean[]", Array_Insert, "(Array)Boolean[];[(Pos)i]"),
	GB_METHOD("Copy", "Boolean[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Boolean[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Boolean[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Boolean[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Boolean[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)b[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_ByteArray[] =
{
	GB_DECLARE("Byte[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Integer_Add, "(Value)c[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Integer_Push, "(Value)c"),
	GB_METHOD("_put", NULL, Array_Byte_put, "(Value)c(Index)i."),
	GB_METHOD("Find", "i", Array_Integer_Find, "(Value)c[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Integer_Exist, "(Value)c"),

	GB_METHOD("Pop", "c", Array_Pop, NULL),
	GB_METHOD("_get", "c", Array_get, "(Index)i."),
	GB_METHOD("_next", "c", Array_next, NULL),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Byte[]", Array_Insert, "(Array)Byte[];[(Pos)i]"),
	GB_METHOD("Copy", "Byte[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Byte[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Byte[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Byte[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Byte[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)c[(Start)i(Length)i]"),

	GB_METHOD("ToString", "s", Array_Byte_ToString, "[(Start)i(Length)i]"),
	GB_STATIC_METHOD("FromString", "Byte[]", Array_Byte_FromString, "(String)s"),

	GB_END_DECLARE
};


GB_DESC NATIVE_ShortArray[] =
{
	GB_DECLARE("Short[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Integer_Add, "(Value)h[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Integer_Push, "(Value)h"),
	GB_METHOD("_put", NULL, Array_Short_put, "(Value)h(Index)i."),
	GB_METHOD("Find", "i", Array_Integer_Find, "(Value)h[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Integer_Exist, "(Value)h"),

	GB_METHOD("Pop", "h", Array_Pop, NULL),
	GB_METHOD("_get", "h", Array_get, "(Index)i."),
	GB_METHOD("_next", "h", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "h", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Short[]", Array_Insert, "(Array)Short[];[(Pos)i]"),
	GB_METHOD("Copy", "Short[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Short[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Short[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Short[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Short[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)h[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_IntegerArray[] =
{
	GB_DECLARE("Integer[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Integer_Add, "(Value)i[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Integer_Push, "(Value)i"),
	GB_METHOD("_put", NULL, Array_Integer_put, "(Value)i(Index)i."),
	GB_METHOD("Find", "i", Array_Integer_Find, "(Value)i[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Integer_Exist, "(Value)i"),

	GB_METHOD("Pop", "i", Array_Pop, NULL),
	GB_METHOD("_get", "i", Array_get, "(Index)i."),
	GB_METHOD("_next", "i", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "i", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Integer[]", Array_Insert, "(Array)Integer[];[(Pos)i]"),
	GB_METHOD("Copy", "Integer[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Integer[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Integer[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Integer[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Integer[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)i[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_LongArray[] =
{
	GB_DECLARE("Long[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Long_Add, "(Value)l[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Long_Push, "(Value)l"),
	GB_METHOD("_put", NULL, Array_Long_put, "(Value)l(Index)i."),
	GB_METHOD("Find", "i", Array_Long_Find, "(Value)l[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Long_Exist, "(Value)l"),

	GB_METHOD("Pop", "l", Array_Pop, NULL),
	GB_METHOD("_get", "l", Array_get, "(Index)i."),
	GB_METHOD("_next", "l", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "l", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Long[]", Array_Insert, "(Array)Long[];[(Pos)i]"),

	GB_METHOD("Copy", "Long[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Long[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Long[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Long[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Long[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)l[(Start)i(Length)i]"),

	GB_END_DECLARE
};

#ifdef OS_64BITS
#define Array_Pointer_Add Array_Long_Add
#define Array_Pointer_Push Array_Long_Push
#define Array_Pointer_put Array_Long_put
#define Array_Pointer_Find Array_Long_Find
#define Array_Pointer_Exist Array_Long_Exist
#else
#define Array_Pointer_Add Array_Integer_Add
#define Array_Pointer_Push Array_Integer_Push
#define Array_Pointer_put Array_Integer_put
#define Array_Pointer_Find Array_Integer_Find
#define Array_Pointer_Exist Array_Integer_Exist
#endif

GB_DESC NATIVE_PointerArray[] =
{
	GB_DECLARE("Pointer[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Pointer_Add, "(Value)p[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Pointer_Push, "(Value)p"),
	GB_METHOD("_put", NULL, Array_Pointer_put, "(Value)p(Index)i."),
	GB_METHOD("Find", "i", Array_Pointer_Find, "(Value)p[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Pointer_Exist, "(Value)p"),

	GB_METHOD("Pop", "p", Array_Pop, NULL),
	GB_METHOD("_get", "p", Array_get, "(Index)i."),
	GB_METHOD("_next", "p", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "p", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Pointer[]", Array_Insert, "(Array)Pointer[];[(Pos)i]"),

	GB_METHOD("Copy", "Pointer[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Pointer[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Pointer[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Pointer[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Pointer[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)p[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_StringArray[] =
{
	GB_DECLARE("String[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_String_Add, "(Value)s[(Index)i]"),
	GB_METHOD("Push", NULL, Array_String_Push, "(Value)s"),
	GB_METHOD("_put", NULL, Array_String_put, "(Value)s(Index)i."),
	GB_METHOD("Find", "i", Array_String_Find, "(Value)s[(Mode)i(Start)i]"),
	GB_METHOD("Exist", "b", Array_String_Exist, "(Value)s[(Mode)i]"),

	GB_METHOD("Pop", "s", Array_Pop, NULL),
	GB_METHOD("_get", "s", Array_get, "(Index)i."),
	GB_METHOD("_next", "s", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "s", Array_Last),

	GB_METHOD("Insert", "String[]", Array_Insert, "(Array)String[];[(Pos)i]"),

	GB_METHOD("Join", "s", Array_String_join, "[(Separator)s(Escape)s]"),

	GB_METHOD("Copy", "String[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "String[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "String[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "String[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "String[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)s[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_FloatArray[] =
{
	GB_DECLARE("Float[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Float_Add, "(Value)f[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Float_Push, "(Value)f"),
	GB_METHOD("_put", NULL, Array_Float_put, "(Value)f(Index)i."),
	GB_METHOD("Find", "i", Array_Float_Find, "(Value)f[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Float_Exist, "(Value)f"),

	GB_METHOD("Pop", "f", Array_Pop, NULL),
	GB_METHOD("_get", "f", Array_get, "(Index)i."),
	GB_METHOD("_next", "f", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "f", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Float[]", Array_Insert, "(Array)Float[];[(Pos)i]"),

	GB_METHOD("Copy", "Float[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Float[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Float[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Float[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Float[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)f[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_SingleArray[] =
{
	GB_DECLARE("Single[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Single_Add, "(Value)g[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Single_Push, "(Value)g"),
	GB_METHOD("_put", NULL, Array_Single_put, "(Value)g(Index)i."),
	GB_METHOD("Find", "i", Array_Single_Find, "(Value)g[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Single_Exist, "(Value)g"),

	GB_METHOD("Pop", "g", Array_Pop, NULL),
	GB_METHOD("_get", "g", Array_get, "(Index)i."),
	GB_METHOD("_next", "g", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "g", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Single[]", Array_Insert, "(Array)Single[];[(Pos)i]"),

	GB_METHOD("Copy", "Single[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Single[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Single[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Single[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Single[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)g[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_DateArray[] =
{
	GB_DECLARE("Date[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Date_Add, "(Value)d[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Date_Push, "(Value)d"),
	GB_METHOD("_put", NULL, Array_Date_put, "(Value)d(Index)i."),
	GB_METHOD("Find", "i", Array_Date_Find, "(Value)d[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Date_Exist, "(Value)d"),

	GB_METHOD("Pop", "d", Array_Pop, NULL),
	GB_METHOD("_get", "d", Array_get, "(Index)i."),
	GB_METHOD("_next", "d", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "d", Array_Last),

	GB_METHOD("Read", NULL, Array_Read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, Array_Write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Date[]", Array_Insert, "(Array)Date[];[(Pos)i]"),

	GB_METHOD("Copy", "Date[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Date[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Date[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Date[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "Date[]", CARRAY_reverse, NULL),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)d[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_ObjectArray[] =
{
	GB_DECLARE("Object[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Object_Add, "(Value)o[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Object_Push, "(Value)o"),
	GB_METHOD("_put", NULL, Array_Object_put, "(Value)o(Index)i."),
	GB_METHOD("Find", "i", Array_Object_Find, "(Value)o[(Start)i]"),
	GB_METHOD("FindByRef", "i", Array_Object_FindByRef, "(Value)o[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Object_Exist, "(Value)o"),
	GB_METHOD("ExistByRef", "b", Array_Object_ExistByRef, "(Value)o"),

	GB_METHOD("Pop", "o", Array_Pop, NULL),
	GB_METHOD("_get", "o", Array_get, "(Index)i."),
	GB_METHOD("_next", "o", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "o", Array_Last),

	GB_METHOD("Insert", "Object[]", Array_Insert, "(Array)Object[];[(Pos)i]"),

	GB_METHOD("Copy", "Object[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Object[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Object[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)o[(Start)i(Length)i]"),
	GB_METHOD("Reverse", "Object[]", CARRAY_reverse, NULL),
	GB_METHOD("Sort", "Object[]", Array_Sort, "[(Mode)i]"),

	GB_END_DECLARE
};

GB_DESC NATIVE_VariantArray[] =
{
	GB_DECLARE("Variant[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Variant_Add, "(Value)v[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Variant_Push, "(Value)v"),
	GB_METHOD("_put", NULL, Array_Variant_put, "(Value)v(Index)i."),
	GB_METHOD("Find", "i", Array_Variant_Find, "(Value)v[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Variant_Exist, "(Value)v"),

	GB_METHOD("Pop", "v", Array_Pop, NULL),
	GB_METHOD("_get", "v", Array_get, "(Index)i."),
	GB_METHOD("_next", "v", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "v", Array_Last),

	GB_METHOD("Insert", "Variant[]", Array_Insert, "(Array)Variant[];[(Pos)i]"),

	GB_METHOD("Copy", "Variant[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Variant[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Variant[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)v[(Start)i(Length)i]"),
	GB_METHOD("Reverse", "Variant[]", CARRAY_reverse, NULL),

	GB_END_DECLARE
};

// Beware: if this declaration is modified, the ARRAY_TEMPLATE_NDESC constant must be modified accordingly.

GB_DESC NATIVE_TemplateArray[ARRAY_TEMPLATE_NDESC] =
{
	GB_DECLARE("*[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, Array_Object_Add, "(Value)*;[(Index)i]"),
	GB_METHOD("Push", NULL, Array_Object_Push, "(Value)*;"),
	GB_METHOD("_put", NULL, Array_Object_put, "(Value)*;(Index)i."),
	GB_METHOD("Find", "i", Array_Object_Find, "(Value)*;[(Start)i]"),
	GB_METHOD("FindByRef", "i", Array_Object_FindByRef, "(Value)*;[(Start)i]"),
	GB_METHOD("Exist", "b", Array_Object_Exist, "(Value)*;"),
	GB_METHOD("ExistByRef", "b", Array_Object_ExistByRef, "(Value)*;"),

	GB_METHOD("Pop", "*", Array_Pop, NULL),
	GB_METHOD("_get", "*", Array_get, "(Index)i."),
	GB_METHOD("_next", "*", Array_next, NULL),
	//GB_PROPERTY_READ("Last", "*", Array_Last),

	GB_METHOD("Insert", "*", Array_Insert, "(Array)*[];[(Pos)i]"),

	GB_METHOD("Copy", "*[]", Array_Copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "*[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "*[]", Array_Extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, Array_Fill, "(Value)*;[(Start)i(Length)i]"),
	GB_METHOD("Sort", "*[]", Array_Sort, "[(Mode)i]"),
	GB_METHOD("Reverse", "*[]", CARRAY_reverse, NULL),

	GB_END_DECLARE
};

// Beware: if this declaration is modified, the ARRAY_OF_STRUCT_TEMPLATE_NDESC constant must be modified accordingly.

GB_DESC NATIVE_TemplateArrayOfStruct[ARRAY_OF_STRUCT_TEMPLATE_NDESC] =
{
	GB_DECLARE("$*[]", sizeof(CARRAY)), GB_NOT_CREATABLE(),

	//GB_METHOD("_new", NULL, Array_new, "[(Size)i.]"),
	GB_METHOD("_free", NULL, Array_free, NULL),

	GB_PROPERTY_READ("Count", "i", Array_Count),
	GB_PROPERTY_READ("Max", "i", Array_Max),
	GB_PROPERTY_READ("Length", "i", Array_Count),
	GB_PROPERTY_READ("Dim", "i", Array_Dim),
	GB_PROPERTY_READ("Data", "p", Array_Data),
	GB_PROPERTY_SELF("Bounds", ".Array.Bounds"),

	GB_METHOD("_get", "*", ArrayOfStruct_get, "(Index)i."),
	GB_METHOD("_put", NULL, ArrayOfStruct_put, "(Value)*;(Index)i."),
	GB_METHOD("_next", "*", ArrayOfStruct_next, NULL),
	//GB_PROPERTY_READ("Last", "b", ArrayOfStruct_Last),

	GB_END_DECLARE
};


#ifndef GBX_INFO

/* Gambas API */

void GB_ArrayNew(GB_ARRAY *array, TYPE type, int size)
{
	int np;
	CTYPE ctype;

	if (size > 0)
	{
		GB_Push(1, GB_T_INTEGER, size);
		np = 1;
	}
	else
	{
		np = 0;
	}
	
	ctype.id = T_NULL;
	*array = OBJECT_create(CARRAY_get_array_class((CLASS *)type, ctype), NULL, NULL, np);
}

int GB_ArrayCount(GB_ARRAY array)
{
	return ((CARRAY *)array)->count;
}

void *GB_ArrayAdd(GB_ARRAY array)
{
	return insert((CARRAY *)array, -1);
}

void *GB_ArrayGet(GB_ARRAY array, int index)
{
	return get_data((CARRAY *)array, index);
}

TYPE GB_ArrayType(GB_ARRAY array)
{
	return ((CARRAY *)array)->type;
}

void GB_ArrayRemove(GB_ARRAY array, int index)
{
	copy_remove((CARRAY *)array, index, 1, FALSE, TRUE);
}



#endif
