/***************************************************************************

  gbx_c_array.c

  The Array native classes.

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

#define __GBX_C_ARRAY_C

#include "gbx_info.h"

#ifndef GBX_INFO

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
#include "gbx_print.h"
#include "gbx_c_file.h"
#include "gbx_c_array.h"



static bool check_not_multi(CARRAY *_object)
{
	if (THIS->dim)
	{
		GB_Error("Array is multi-dimensional");
		return TRUE;
	}
	else
		return FALSE;
}

static int get_dim(CARRAY *_object)
{
	int d;

	if (!THIS->dim)
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
	if (!THIS->dim)
		return ARRAY_count(THIS);
	else
	{
		d = THIS->dim[d];
		if (d < 0)
			d = (-d);

		return d;
	}
}


void *CARRAY_out_of_bound()
{
	GB_Error((char *)E_BOUND);
	return NULL;
}

// static void *get_data(CARRAY *_object, int index)
// {
//   if ((index < 0) || (index >= ARRAY_count(THIS->data)))
//   {
//     return NULL;
//   }
// 
//   return (void *)((char *)(THIS->data) + index * TYPE_sizeof_memory(THIS->type));
// }

#define get_data_multi CARRAY_get_data_multi
#define get_data CARRAY_get_data

void *CARRAY_get_data_multi(CARRAY *_object, GB_INTEGER *arg, int nparam)
{
	int index;

	//fprintf(stderr, "get_data_multi: nparam = %d\n", nparam);

	if (THIS->dim)
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

			VALUE_conv((VALUE *)&arg[i], GB_T_INTEGER);
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

		if ((index < 0) || (index >= ARRAY_count(THIS->data)))
		{
			GB_Error((char *)E_BOUND);
			return NULL;
		}
	}

	return (void *)((char *)(THIS->data) + index * TYPE_sizeof_memory(THIS->type));
}


static void release_one(CARRAY *_object, int i)
{
	if (THIS->type == T_STRING)
		STRING_unref(&(((char **)(THIS->data))[i]));
	else if (THIS->type == T_VARIANT)
		VARIANT_free(&(((VARIANT *)(THIS->data))[i]));
	else if (TYPE_is_object(THIS->type))
		OBJECT_UNREF(((void **)(THIS->data))[i], "release_one");
}


static void release(CARRAY *_object, int start, int end)
{
	int i;

	if (end < 0)
		end = ARRAY_count(THIS->data);

	if (THIS->type == T_STRING)
	{
		for (i = start; i < end; i++)
			STRING_unref(&((char **)(THIS->data))[i]);
	}
	else if (THIS->type == T_VARIANT)
	{
		for (i = start; i < end; i++)
			VARIANT_free(&((VARIANT *)(THIS->data))[i]);
	}
	else if (TYPE_is_object(THIS->type))
	{
		for (i = start; i < end; i++)
			OBJECT_UNREF(((void **)(THIS->data))[i], "release");
	}
}


static void borrow(CARRAY *_object, int start, int end)
{
	int i;

	if (end < 0)
		end = ARRAY_count(THIS->data);

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
	release(THIS, 0, -1);
	if (THIS->dim)
	{
		memset(THIS->data, 0, TYPE_sizeof_memory(THIS->type) * ARRAY_count(THIS->data));
	}
	else
	{
		ARRAY_delete(&THIS->data);
		ARRAY_create_with_size(&THIS->data, TYPE_sizeof_memory(THIS->type), 8);
	}
}


static void *insert(CARRAY *_object, int index)
{
	return ARRAY_insert(&THIS->data, index);
}


void CARRAY_get_value(CARRAY *_object, int index, VALUE *value)
{
	VALUE_read(value, get_data(THIS, index), THIS->type);
}


BEGIN_METHOD(CARRAY_new, GB_INTEGER size)

	TYPE type;
	CLASS *klass;
	int size;
	int inc;
	GB_INTEGER *sizes = ARG(size);
	int nsize = GB_NParam() + 1;
	int i;

	klass = OBJECT_class(THIS);

	if (klass == CLASS_IntegerArray)
		type = T_INTEGER;
	else if (klass == CLASS_ShortArray)
		type = T_SHORT;
	else if (klass == CLASS_BooleanArray)
		type = T_BOOLEAN;
	else if (klass == CLASS_ByteArray)
		type = T_BYTE;
	else if (klass == CLASS_LongArray)
		type = T_LONG;
	else if (klass == CLASS_SingleArray)
		type = T_SINGLE;
	else if (klass == CLASS_FloatArray)
		type = T_FLOAT;
	else if (klass == CLASS_DateArray)
		type = T_DATE;
	else if (klass == CLASS_StringArray)
		type = T_STRING;
	else if (klass == CLASS_VariantArray)
		type = T_VARIANT;
	else if (klass == CLASS_PointerArray)
		type = T_POINTER;
	else if (klass == CLASS_ObjectArray)
		type = T_OBJECT;
	else
	{
		type = (TYPE)klass->array_type;
		if (!type)
		{
			GB_Error("Bad array type");
			return;
		}
	}

	//printf("CARRAY_new: type = %d nsize = %d\n", type, nsize);

	THIS->type = type;

	if (nsize <= 1)
	{
		size = VARGOPT(size, 0);
		if (size < 0)
			size = 0;

		inc = (size / 8) & ~7;
		if (inc < 8)
			inc = 8;

		ARRAY_create_with_size(&THIS->data, TYPE_sizeof_memory(type), inc);
		if (size > 0)
			ARRAY_add_many_void(&THIS->data, size);
	}
	else
	{
		if (nsize > MAX_ARRAY_DIM)
		{
			GB_Error("Too many dimensions");
			return;
		}

		size = 1;
		for (i = 0; i < nsize; i++)
		{
			VALUE_conv((VALUE *)&sizes[i], T_INTEGER);
			if (sizes[i].value < 1)
			{
				GB_Error("Bad dimension");
				return;
			}
			size *= sizes[i].value;
		}

		ALLOC_ZERO(&THIS->dim, nsize * sizeof(int), "CARRAY_new");

		for (i = 0; i < nsize; i++)
			THIS->dim[i] = sizes[i].value;
		THIS->dim[nsize - 1] = (-THIS->dim[nsize - 1]);

		ARRAY_create_with_size(&THIS->data, TYPE_sizeof_memory(type), 8);
		ARRAY_add_many_void(&THIS->data, size);
	}

END_METHOD


BEGIN_PROPERTY(CARRAY_type)

	GB_ReturnInteger(THIS->type);

END_PROPERTY

BEGIN_METHOD_VOID(CARRAY_free)

	release(THIS, 0, -1);
	ARRAY_delete(&THIS->data);

	if (THIS->dim)
		FREE(&THIS->dim, "CARRAY_free");

END_METHOD


BEGIN_METHOD_VOID(CARRAY_clear)

	clear(THIS);

END_METHOD


BEGIN_PROPERTY(CARRAY_data)

	GB_ReturnPointer(THIS->data);

END_PROPERTY


BEGIN_PROPERTY(CARRAY_count)

	GB_ReturnInt(ARRAY_count(THIS->data));

END_PROPERTY


BEGIN_PROPERTY(CARRAY_max)

	GB_ReturnInt(ARRAY_count(THIS->data) - 1);

END_PROPERTY


static bool copy_remove(CARRAY *_object, int start, int length, bool copy, bool remove)
{
	CARRAY *array;
	int count = ARRAY_count(THIS->data);
	void *data;
	int i, nsize;

	if (check_not_multi(THIS) && (start != 0 || length != -1))
		return TRUE;

	if (start < 0)
	{
		GB_Error((char *)E_BOUND);
		return TRUE;
	}

	if (start >= count)
		length = 0;

	if (length == -1)
		length = count - start;

	if (length < 0 || (start + length) > count)
	{
		GB_Error((char *)E_BOUND);
		return TRUE;
	}

	if (copy)
	{
		GB_ArrayNew((GB_ARRAY *)POINTER(&array), THIS->type, 0);
	
		if (length > 0)
		{
			data = ARRAY_insert_many(&array->data, 0, length);
			memmove(data, get_data(THIS, start), length * TYPE_sizeof_memory(THIS->type));
			borrow(array, 0, -1);
		}
		
		if (THIS->dim)
		{
			nsize = get_dim(THIS);
			ALLOC_ZERO(&array->dim, nsize * sizeof(int), "copy_remove");

			for (i = 0; i < nsize; i++)
				array->dim[i] = THIS->dim[i];
		}
	}

	if (remove)
	{
		release(THIS, start, start + length);
		ARRAY_remove_many(&THIS->data, start, length);
	}
	
	if (copy)
		GB_ReturnObject(array);
		
	return FALSE;
}

BEGIN_METHOD(CARRAY_remove, GB_INTEGER index; GB_INTEGER length)

	copy_remove(THIS, VARG(index), VARGOPT(length, 1), FALSE, TRUE);

END_METHOD


BEGIN_METHOD(CARRAY_copy, GB_INTEGER start; GB_INTEGER length)

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


BEGIN_METHOD(CARRAY_extract, GB_INTEGER start; GB_INTEGER length)

	copy_remove(THIS, VARG(start), VARGOPT(length, 1), TRUE, TRUE);

END_METHOD


BEGIN_METHOD(CARRAY_resize, GB_INTEGER size)

	int size = VARG(size);
	int count = ARRAY_count(THIS->data);

	if (check_not_multi(THIS))
		return;

	if (size > count)
	{
		ARRAY_add_many_void(&THIS->data, size - count);
	}
	else if ((size < count) && (size >= 0))
	{
		release(THIS, size, -1);
		ARRAY_remove_many(&THIS->data, size, count - size);
	}

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
BEGIN_METHOD(CARRAY_##_type##_add, GB_##_gtype value; GB_INTEGER index) \
\
	add(THIS, (GB_VALUE *)(void *)ARG(value), VARGOPT(index, -1)); \
	\
END_METHOD \
\
BEGIN_METHOD(CARRAY_##_type##_push, GB_##_gtype value) \
\
	add(THIS, (GB_VALUE *)(void *)ARG(value), -1); \
	\
END_METHOD

IMPLEMENT_add(integer, INTEGER)
IMPLEMENT_add(long, LONG)
IMPLEMENT_add(float, FLOAT)
IMPLEMENT_add(date, DATE)
IMPLEMENT_add(string, STRING)
IMPLEMENT_add(object, OBJECT)
IMPLEMENT_add(variant, VARIANT)

BEGIN_METHOD(CARRAY_add, GB_VARIANT value; GB_INTEGER index)
	
	GB_VALUE *value = (GB_VALUE *)(void *)ARG(value);
	GB_Conv(value, THIS->type);	
	add(THIS, value, VARGOPT(index, -1));
	
END_METHOD

BEGIN_METHOD(CARRAY_push, GB_VARIANT value)

	GB_VALUE *value = (GB_VALUE *)(void *)ARG(value);
	GB_Conv(value, THIS->type);	
	add(THIS, value, -1);
	
END_METHOD


#define IMPLEMENT_put(_type, _gtype) \
BEGIN_METHOD(CARRAY_##_type##_put, GB_##_gtype value; GB_INTEGER index) \
	\
	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1); \
	if (!data) return; \
	GB_Store(GB_T_##_gtype, (GB_VALUE *)(void *)ARG(value), data); \
	\
END_METHOD

#define IMPLEMENT_put2(_type, _gtype, _gstore) \
BEGIN_METHOD(CARRAY_##_type##_put, GB_##_gtype value; GB_INTEGER index) \
\
	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1); \
	if (!data) return; \
	GB_Store(GB_T_##_gstore, (GB_VALUE *)(void *)ARG(value), data); \
	\
END_METHOD

BEGIN_METHOD(CARRAY_put, GB_VARIANT value; GB_INTEGER index)
	
	GB_VALUE *value;
	void *data;
	
	value = (GB_VALUE *)(void *)ARG(value);
	GB_Conv(value, THIS->type);	
	
	data = get_data_multi(THIS, ARG(index), GB_NParam() + 1);
	if (!data) return;
	
	GB_Store(THIS->type, value, data);
	
END_METHOD

IMPLEMENT_put(integer, INTEGER)
IMPLEMENT_put2(short, INTEGER, SHORT)
IMPLEMENT_put2(byte, INTEGER, BYTE)
IMPLEMENT_put2(boolean, INTEGER, BOOLEAN)
IMPLEMENT_put(long, LONG)
IMPLEMENT_put(float, FLOAT)
IMPLEMENT_put2(single, FLOAT, SINGLE)
IMPLEMENT_put(date, DATE)
IMPLEMENT_put(string, STRING)
IMPLEMENT_put(object, OBJECT)
IMPLEMENT_put(variant, VARIANT)


BEGIN_METHOD(CARRAY_fill, GB_VALUE value; GB_INTEGER start; GB_INTEGER length)

	int count = ARRAY_count(THIS->data);
	int start = VARGOPT(start, 0);
	int length = VARGOPT(length, count);
	int i;
	void *data;
	int size;

	if ((start + length) > count)
		length = count - start;

	VALUE_conv((VALUE *)ARG(value), THIS->type);

	data = get_data(THIS, start);
	size = TYPE_sizeof_memory(THIS->type);

	for (i = 0; i < length; i++)
	{
		GB_Store(THIS->type, (GB_VALUE *)ARG(value), data);
		data += size;
	}

END_METHOD


BEGIN_METHOD(CARRAY_insert, GB_OBJECT array; GB_INTEGER pos)

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

	count = ARRAY_count(array->data);
	
	if (count > 0)
	{
		if (pos < 0)
			pos = ARRAY_count(THIS->data);

		data = ARRAY_insert_many(&THIS->data, pos, count);
		borrow(array, 0, -1);
		memmove(data, array->data, count * TYPE_sizeof_memory(THIS->type));
	}
	
	GB_ReturnObject(THIS);

END_METHOD


BEGIN_METHOD_VOID(CARRAY_pop)

	int index = ARRAY_count(THIS->data) - 1;

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
	UNBORROW(&TEMP);

END_METHOD


BEGIN_METHOD(CARRAY_get, GB_INTEGER index)

	void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1);

	if (data)
		GB_ReturnPtr(THIS->type, data);

END_METHOD


BEGIN_METHOD_VOID(CARRAY_next)

	int *index = (int *)GB_GetEnum();

	if (*index >= ARRAY_count(THIS->data))
		GB_StopEnum();
	else
	{
		GB_ReturnPtr(THIS->type, get_data(THIS, *index));
		(*index)++;
	}

END_METHOD


BEGIN_METHOD(CARRAY_sort, GB_INTEGER mode)

	COMPARE_FUNC compare;
	int mode = VARGOPT(mode, 0);
	void *data = THIS->data;

	if (ARRAY_count(data) > 1)
	{
		compare = COMPARE_get(THIS->type, mode);
		qsort(data, ARRAY_count(data), TYPE_sizeof_memory(THIS->type), compare);
	}

	GB_ReturnObject(THIS);

END_METHOD


static int find(CARRAY *_object, int mode, void *value)
{
	COMPARE_FUNC compare = COMPARE_get(THIS->type, mode);
	int i;

	for (i = 0; i < ARRAY_count(THIS->data); i++)
	{
		if ((*compare)(value, get_data(THIS, i)) == 0)
			return i;
	}

	return (-1);
}

#define IMPLEMENT_find(_type, _gtype) \
BEGIN_METHOD(CARRAY_##_type##_find, _gtype value) \
\
	GB_ReturnInt(find(THIS, 0, &VARG(value))); \
\
END_METHOD \
BEGIN_METHOD(CARRAY_##_type##_exist, _gtype value) \
\
	GB_ReturnBoolean(find(THIS, 0, &VARG(value)) >= 0); \
\
END_METHOD

IMPLEMENT_find(integer, GB_INTEGER)
/*IMPLEMENT_find(short, GB_INTEGER)
IMPLEMENT_find(byte, GB_INTEGER)*/
IMPLEMENT_find(long, GB_LONG)
IMPLEMENT_find(float, GB_FLOAT)
IMPLEMENT_find(date, GB_DATE)

static int find_object(CARRAY *_object, void *value)
{
	int i;

	for (i = 0; i < ARRAY_count(THIS->data); i++)
	{
		if (*((void **)get_data(THIS, i)) == value)
			return i;
	}

	return (-1);
}

BEGIN_METHOD(CARRAY_object_find, GB_OBJECT value)

	GB_ReturnInt(find_object(THIS, VARG(value)));

END_METHOD

BEGIN_METHOD(CARRAY_object_exist, GB_OBJECT value)

	GB_ReturnBoolean(find_object(THIS, VARG(value)) >= 0);

END_METHOD


BEGIN_METHOD(CARRAY_string_find, GB_STRING value; GB_INTEGER mode)

	char *str = GB_ToZeroString(ARG(value));

	GB_ReturnInteger(find(THIS, VARGOPT(mode, 0), &str));

END_METHOD

BEGIN_METHOD(CARRAY_string_exist, GB_STRING value; GB_INTEGER mode)

	char *str = GB_ToZeroString(ARG(value));

	GB_ReturnInteger(find(THIS, VARGOPT(mode, 0), &str) >= 0);

END_METHOD


BEGIN_METHOD(CARRAY_string_join, GB_STRING sep; GB_STRING esc)

	char *sep = ",";
	int lsep = 1;
	char *esc = "";
	int lesc = 0;
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

	if (!lesc)
	{
		max = 0;
		for (i = 0; i < ARRAY_count(data); i++)
			max += STRING_length(data[i]) + lsep;
		if (ARRAY_count(data))
			max -= lsep;
		
		STRING_start_len(max);
	}
	else
		STRING_start();
	
	for (i = 0; i < ARRAY_count(data); i++)
	{
		p = data[i];
		l = STRING_length(data[i]);
		
		if (i)
			STRING_make(sep, lsep);
		if (lesc)
		{
			STRING_make(&escl, 1);
			if (l)
			{
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
			}
			STRING_make_char(escr);
		}
		else if (l)
			STRING_make(p, l);
	}

	GB_ReturnString(STRING_end_temp());

END_METHOD


BEGIN_METHOD_VOID(CARRAY_reverse)

	size_t size;
	int count;
	char *buffer[16];
	char *pi, *pj;

	count = ARRAY_count(THIS->data);
	if (count <= 1)
		return;

	size = TYPE_sizeof_memory(THIS->type);
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

END_METHOD


BEGIN_METHOD(CARRAY_read, GB_OBJECT file; GB_INTEGER start; GB_INTEGER length)

	int count = ARRAY_count(THIS->data);
	int start = VARGOPT(start, 0);
	int length = VARGOPT(length, count);

	if ((start + length) > count)
		length = count - start;

	STREAM_read(CSTREAM_stream(VARG(file)), get_data(THIS, start), length * TYPE_sizeof_memory(THIS->type));

END_METHOD


BEGIN_METHOD(CARRAY_write, GB_OBJECT file; GB_INTEGER start; GB_INTEGER length)

	int count = ARRAY_count(THIS->data);
	int start = VARGOPT(start, 0);
	int length = VARGOPT(length, count);

	if ((start + length) > count)
		length = count - start;

	STREAM_write(CSTREAM_stream(VARG(file)), get_data(THIS, start), length * TYPE_sizeof_memory(THIS->type));

END_METHOD


BEGIN_PROPERTY(CARRAY_dim)

	GB_ReturnInteger(get_dim(THIS));

END_PROPERTY


BEGIN_METHOD(CARRAY_bounds_get, GB_INTEGER index)

	int dim = get_dim(THIS);
	int index = VARG(index);

	if (index < 0 || index >= dim)
	{
		GB_Error((char *)E_BOUND);
		return;
	}

	GB_ReturnInteger(get_bound(THIS, index));

END_PROPERTY


/*BEGIN_METHOD_VOID(CARRAY_print)

	int dim[MAX_ARRAY_DIM] = { 0 };
	char *data = THIS->data;
	int ndim = get_dim(THIS);
	int i, j;

	if (ARRAY_count(THIS) == 0)
	{
		GB_PrintString("[]", 2);
		return;
	}

	for (i = 0; i < ndim; i++)
		GB_PrintString("[", 1);

	for(;;)
	{
		GB_PrintData(THIS->type, data);

		for (i = 0; i < ndim; i++)
		{
			dim[i]++;
			if (dim[i] < get_bound(THIS, i))
				break;
			dim[i] = 0;
		}

		for (j = 0; j < i; j++)
			GB_PrintString("]", 1);

		if (i == ndim)
			break;

		for (j = 0; j < i; j++)
			GB_PrintString("[", 1);

		if (i == 0)
			GB_PrintString(",", 1);

		data += TYPE_sizeof_memory(THIS->type);
	}

END_METHOD*/

#else

#include "gbx_c_array.h"

#endif /* #ifndef GBX_INFO */


GB_DESC NATIVE_ArrayBounds[] =
{
	GB_DECLARE(".Array.Bounds", sizeof(CARRAY)), GB_NOT_CREATABLE(),

	GB_METHOD("_get", "i", CARRAY_bounds_get, "(Dimension)i"),

	GB_END_DECLARE
};


GB_DESC NATIVE_Array[] =
{
	GB_DECLARE("Array", sizeof(CARRAY)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, CARRAY_free, NULL),

	GB_PROPERTY_READ("Type", "i", CARRAY_type),
	GB_PROPERTY_READ("Count", "i", CARRAY_count),
	GB_PROPERTY_READ("Max", "i", CARRAY_max),
	GB_PROPERTY_READ("Length", "i", CARRAY_count),
	GB_PROPERTY_READ("Dim", "i", CARRAY_dim),
	GB_PROPERTY_READ("Data", "p", CARRAY_data),
	GB_PROPERTY_SELF("Bounds", ".Array.Bounds"),

	GB_METHOD("Remove", NULL, CARRAY_remove, "(Index)i[(Length)i]"),
	GB_METHOD("Clear", NULL, CARRAY_clear, NULL),
	GB_METHOD("Resize", NULL, CARRAY_resize, "(Size)i"),
	GB_METHOD("Reverse", NULL, CARRAY_reverse, NULL),

	GB_METHOD("Add", NULL, CARRAY_add, "(Value)v[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_push, "(Value)v"),
	GB_METHOD("_put", NULL, CARRAY_put, "(Value)v(Index)i."),

	GB_METHOD("Pop", "v", CARRAY_pop, NULL),
	GB_METHOD("_get", "v", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "v", CARRAY_next, NULL),

	GB_METHOD("Copy", "Array", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Array", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Array", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)v[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_BooleanArray[] =
{
	GB_DECLARE("Boolean[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)b[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)b"),
	GB_METHOD("_put", NULL, CARRAY_boolean_put, "(Value)b(Index)i."),
	GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)b"),
	GB_METHOD("Exist", "b", CARRAY_integer_exist, "(Value)b"),

	GB_METHOD("Pop", "b", CARRAY_pop, NULL),
	GB_METHOD("_get", "b", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "b", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Boolean[]", CARRAY_insert, "(Array)Boolean[];[(Pos)i]"),
	GB_METHOD("Copy", "Boolean[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Boolean[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Boolean[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Boolean[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)b[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_ByteArray[] =
{
	GB_DECLARE("Byte[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)c[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)c"),
	GB_METHOD("_put", NULL, CARRAY_byte_put, "(Value)c(Index)i."),
	GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)c"),
	GB_METHOD("Exist", "b", CARRAY_integer_exist, "(Value)c"),

	GB_METHOD("Pop", "c", CARRAY_pop, NULL),
	GB_METHOD("_get", "c", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "c", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Byte[]", CARRAY_insert, "(Array)Byte[];[(Pos)i]"),
	GB_METHOD("Copy", "Byte[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Byte[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Byte[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Byte[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)c[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_ShortArray[] =
{
	GB_DECLARE("Short[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)h[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)h"),
	GB_METHOD("_put", NULL, CARRAY_short_put, "(Value)h(Index)i."),
	GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)h"),
	GB_METHOD("Exist", "b", CARRAY_integer_exist, "(Value)h"),

	GB_METHOD("Pop", "h", CARRAY_pop, NULL),
	GB_METHOD("_get", "h", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "h", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Short[]", CARRAY_insert, "(Array)Short[];[(Pos)i]"),
	GB_METHOD("Copy", "Short[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Short[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Short[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Short[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)h[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_IntegerArray[] =
{
	GB_DECLARE("Integer[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)i[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)i"),
	GB_METHOD("_put", NULL, CARRAY_integer_put, "(Value)i(Index)i."),
	GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)i"),
	GB_METHOD("Exist", "b", CARRAY_integer_exist, "(Value)i"),

	GB_METHOD("Pop", "i", CARRAY_pop, NULL),
	GB_METHOD("_get", "i", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "i", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Integer[]", CARRAY_insert, "(Array)Integer[];[(Pos)i]"),
	GB_METHOD("Copy", "Integer[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Integer[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Integer[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Integer[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)i[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_LongArray[] =
{
	GB_DECLARE("Long[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_long_add, "(Value)l[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_long_push, "(Value)l"),
	GB_METHOD("_put", NULL, CARRAY_long_put, "(Value)l(Index)i."),
	GB_METHOD("Find", "i", CARRAY_long_find, "(Value)l"),
	GB_METHOD("Exist", "b", CARRAY_long_exist, "(Value)l"),

	GB_METHOD("Pop", "l", CARRAY_pop, NULL),
	GB_METHOD("_get", "l", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "l", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Long[]", CARRAY_insert, "(Array)Long[];[(Pos)i]"),

	GB_METHOD("Copy", "Long[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Long[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Long[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Long[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)l[(Start)i(Length)i]"),

	GB_END_DECLARE
};

#ifdef OS_64BITS
#define CARRAY_pointer_add CARRAY_long_add
#define CARRAY_pointer_push CARRAY_long_push
#define CARRAY_pointer_put CARRAY_long_put
#define CARRAY_pointer_find CARRAY_long_find
#define CARRAY_pointer_exist CARRAY_long_exist
#else
#define CARRAY_pointer_add CARRAY_integer_add
#define CARRAY_pointer_push CARRAY_integer_push
#define CARRAY_pointer_put CARRAY_integer_put
#define CARRAY_pointer_find CARRAY_integer_find
#define CARRAY_pointer_exist CARRAY_integer_exist
#endif

GB_DESC NATIVE_PointerArray[] =
{
	GB_DECLARE("Pointer[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_long_add, "(Value)p[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_long_push, "(Value)p"),
	GB_METHOD("_put", NULL, CARRAY_long_put, "(Value)p(Index)i."),
	GB_METHOD("Find", "i", CARRAY_long_find, "(Value)p"),
	GB_METHOD("Exist", "b", CARRAY_long_exist, "(Value)p"),

	GB_METHOD("Pop", "p", CARRAY_pop, NULL),
	GB_METHOD("_get", "p", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "p", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Pointer[]", CARRAY_insert, "(Array)Pointer[];[(Pos)i]"),

	GB_METHOD("Copy", "Pointer[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Pointer[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Pointer[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Pointer[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)p[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_StringArray[] =
{
	GB_DECLARE("String[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_string_add, "(Value)s[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_string_push, "(Value)s"),
	GB_METHOD("_put", NULL, CARRAY_string_put, "(Value)s(Index)i."),
	GB_METHOD("Find", "i", CARRAY_string_find, "(Value)s[(Mode)i]"),
	GB_METHOD("Exist", "b", CARRAY_string_exist, "(Value)s[(Mode)i]"),

	GB_METHOD("Pop", "s", CARRAY_pop, NULL),
	GB_METHOD("_get", "s", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "s", CARRAY_next, NULL),

	GB_METHOD("Insert", "String[]", CARRAY_insert, "(Array)String[];[(Pos)i]"),

	GB_METHOD("Join", "s", CARRAY_string_join, "[(Separator)s(Escape)s]"),

	GB_METHOD("Copy", "String[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "String[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "String[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "String[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)s[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_FloatArray[] =
{
	GB_DECLARE("Float[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_float_add, "(Value)f[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_float_push, "(Value)f"),
	GB_METHOD("_put", NULL, CARRAY_float_put, "(Value)f(Index)i."),
	GB_METHOD("Find", "i", CARRAY_float_find, "(Value)f"),
	GB_METHOD("Exist", "b", CARRAY_float_exist, "(Value)f"),

	GB_METHOD("Pop", "f", CARRAY_pop, NULL),
	GB_METHOD("_get", "f", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "f", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Float[]", CARRAY_insert, "(Array)Float[];[(Pos)i]"),

	GB_METHOD("Copy", "Float[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Float[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Float[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Float[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)f[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_SingleArray[] =
{
	GB_DECLARE("Single[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_float_add, "(Value)g[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_float_push, "(Value)g"),
	GB_METHOD("_put", NULL, CARRAY_single_put, "(Value)g(Index)i."),
	GB_METHOD("Find", "i", CARRAY_float_find, "(Value)g"),
	GB_METHOD("Exist", "b", CARRAY_float_exist, "(Value)g"),

	GB_METHOD("Pop", "g", CARRAY_pop, NULL),
	GB_METHOD("_get", "g", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "g", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Single[]", CARRAY_insert, "(Array)Single[];[(Pos)i]"),

	GB_METHOD("Copy", "Single[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Single[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Single[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Single[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)g[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_DateArray[] =
{
	GB_DECLARE("Date[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_date_add, "(Value)d[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_date_push, "(Value)d"),
	GB_METHOD("_put", NULL, CARRAY_date_put, "(Value)d(Index)i."),
	GB_METHOD("Find", "i", CARRAY_date_find, "(Value)d"),
	GB_METHOD("Exist", "b", CARRAY_date_exist, "(Value)d"),

	GB_METHOD("Pop", "d", CARRAY_pop, NULL),
	GB_METHOD("_get", "d", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "d", CARRAY_next, NULL),

	GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
	GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

	GB_METHOD("Insert", "Date[]", CARRAY_insert, "(Array)Date[];[(Pos)i]"),

	GB_METHOD("Copy", "Date[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Date[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Date[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Sort", "Date[]", CARRAY_sort, "[(Mode)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)d[(Start)i(Length)i]"),

	GB_END_DECLARE
};


GB_DESC NATIVE_ObjectArray[] =
{
	GB_DECLARE("Object[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_object_add, "(Value)o[(Index)i.]"),
	GB_METHOD("Push", NULL, CARRAY_object_push, "(Value)o"),
	GB_METHOD("_put", NULL, CARRAY_object_put, "(Value)o(Index)i."),
	GB_METHOD("Find", "i", CARRAY_object_find, "(Value)o"),
	GB_METHOD("Exist", "b", CARRAY_object_exist, "(Value)o"),

	GB_METHOD("Pop", "o", CARRAY_pop, NULL),
	GB_METHOD("_get", "o", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "o", CARRAY_next, NULL),

	GB_METHOD("Insert", "Object[]", CARRAY_insert, "(Array)Object[];[(Pos)i]"),

	GB_METHOD("Copy", "Object[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Object[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Object[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)o[(Start)i(Length)i]"),
	GB_METHOD("Sort", "Object[]", CARRAY_sort, "[(Mode)i]"),

	GB_END_DECLARE
};

// Beware: if this declaration is modified, the ARRAY_TEMPLATE_NDESC constant must be modified accordingly.

GB_DESC NATIVE_TemplateArray[ARRAY_TEMPLATE_NDESC] =
{
	GB_DECLARE("*[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_object_add, "(Value)*;[(Index)i.]"),
	GB_METHOD("Push", NULL, CARRAY_object_push, "(Value)*;"),
	GB_METHOD("_put", NULL, CARRAY_object_put, "(Value)*;(Index)i."),
	GB_METHOD("Find", "i", CARRAY_object_find, "(Value)*;"),
	GB_METHOD("Exist", "b", CARRAY_object_exist, "(Value)*;"),

	GB_METHOD("Pop", "*", CARRAY_pop, NULL),
	GB_METHOD("_get", "*", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "*", CARRAY_next, NULL),

	GB_METHOD("Insert", "*", CARRAY_insert, "(Array)*[];[(Pos)i]"),

	GB_METHOD("Copy", "*[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "*[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "*[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)*;[(Start)i(Length)i]"),
	GB_METHOD("Sort", "*[]", CARRAY_sort, "[(Mode)i]"),

	GB_END_DECLARE
};

GB_DESC NATIVE_VariantArray[] =
{
	GB_DECLARE("Variant[]", sizeof(CARRAY)), GB_INHERITS("Array"),

	GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

	GB_METHOD("Add", NULL, CARRAY_variant_add, "(Value)v[(Index)i]"),
	GB_METHOD("Push", NULL, CARRAY_variant_push, "(Value)v"),
	GB_METHOD("_put", NULL, CARRAY_variant_put, "(Value)v(Index)i."),

	GB_METHOD("Pop", "v", CARRAY_pop, NULL),
	GB_METHOD("_get", "v", CARRAY_get, "(Index)i."),
	GB_METHOD("_next", "v", CARRAY_next, NULL),

	GB_METHOD("Insert", "Variant[]", CARRAY_insert, "(Array)Variant[];[(Pos)i]"),

	GB_METHOD("Copy", "Variant[]", CARRAY_copy, "[(Start)i(Length)i]"),
	GB_METHOD("Extract", "Variant[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Delete", "Variant[]", CARRAY_extract, "(Start)i[(Length)i]"),
	GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)v[(Start)i(Length)i]"),

	GB_END_DECLARE
};


#ifndef GBX_INFO


static CARRAY *_array;
static bool _novoid;
static char *_entry;
static const char *_ptr;
static int _lptr;

static void add_char(const char *p)
{
	//fprintf(stderr, "add_char: %p %c\n", p, p ? *p : 0);
	
	if (p && p == (_ptr + _lptr))
	{
		_lptr++;
		return;
	}
	
	if (_lptr)
	{
		//fprintf(stderr, "STRING_add: '%.*s'\n", _lptr, _ptr);
		STRING_add(&_entry, _ptr, _lptr);
	}
	
	_ptr = p;
	_lptr = p ? 1 : 0;
}

static void add_entry()
{
	add_char(NULL);
	
	if (!_entry)
	{
		if (!_novoid)
			ARRAY_add_void(&_array->data);
	}
	else
	{
		*((char **)ARRAY_add(&_array->data)) = _entry;
		_entry = NULL;
	}
	
	//fprintf(stderr, "** add_entry\n");
}


void CARRAY_split(CARRAY *_object, const char *str, int lstr, const char *sep, const char *esc, bool no_void)
{
	int i;
	char c;
	bool escape;
	char escl, escr;
	bool lsep;
	
	if (sep == NULL || *sep == 0)
		sep = ",";

	lsep = sep[1];

	if (esc == NULL || *esc == 0)
		escl = escr = 0;
	else
	{
		escl = *esc++;
		if (*esc)
			escr = *esc;
		else
			escr = escl;
	}

	clear(THIS);
	
	_array = _object;
	_entry = NULL;
	_novoid = no_void;
	_ptr = NULL;
	_lptr = 0;
	
	escape = FALSE;
	
	for (i = 0; i < lstr; i++)
	{
		c = *str;
		
		if (escape)
		{
			if (c != escr)
				add_char(str);
			else if ((i < (lstr - 1)) && str[1] == escr)
			{
				add_char(str);
				str++;
				i++;
			}
			else
				escape = FALSE;
		}
		else if (c == escl)
		{
			escape = TRUE;
		}
		else if (c == *sep || (lsep && index(&sep[1], c)))
		{
			add_entry();
		}
		else
			add_char(str);
			
		str++;
	}
	
	add_entry();
	
	_array = NULL;
}

/* Gambas API */

void GB_ArrayNew(GB_ARRAY *array, intptr_t type, int size)
{
	CLASS *class;
	int np;

	if (size > 0)
	{
		GB_Push(1, GB_T_INTEGER, size);
		np = 1;
	}
	else
	{
		np = 0;
	}

	if (TYPE_is_pure_object(type))
		class = CLASS_get_array_class((CLASS *)type);
	else
	{
		switch(type)
		{
			case GB_T_BOOLEAN: class = CLASS_BooleanArray; break;
			case GB_T_BYTE: class = CLASS_ByteArray; break;
			case GB_T_SHORT: class = CLASS_ShortArray; break;
			case GB_T_INTEGER: class = CLASS_IntegerArray; break;
			case GB_T_LONG: class = CLASS_LongArray; break;
			case GB_T_SINGLE: class = CLASS_SingleArray; break;
			case GB_T_FLOAT: class = CLASS_FloatArray; break;
			case GB_T_STRING: class = CLASS_StringArray; break;
			case GB_T_DATE: class = CLASS_DateArray; break;
			case GB_T_OBJECT: class = CLASS_ObjectArray; break;
			case GB_T_POINTER: class = CLASS_PointerArray; break;
			default: class = CLASS_VariantArray; break;
		}
	}

	OBJECT_create((void **)array, class, NULL, NULL, np);
}

int GB_ArrayCount(GB_ARRAY array)
{
	return ARRAY_count(((CARRAY *)array)->data);
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

#endif
