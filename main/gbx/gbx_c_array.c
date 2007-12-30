/***************************************************************************

  gbx_c_array.c

  The Array native classes.

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

static long get_bound(CARRAY *_object, int d)
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


static void *get_data(CARRAY *_object, long index)
{
  if ((index < 0) || (index >= ARRAY_count(THIS->data)))
  {
    GB_Error((char *)E_BOUND);
    return NULL;
  }

  return (void *)((char *)(THIS->data) + index * TYPE_sizeof_memory(THIS->type));
}


static void *get_data_multi(CARRAY *_object, GB_INTEGER *arg, int nparam)
{
  long index;

  //fprintf(stderr, "get_data_multi: nparam = %d\n", nparam);

  if (THIS->dim)
  {
    long max;
    int i;
    long d;
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
    index = arg->value;

    if ((index < 0) || (index >= ARRAY_count(THIS->data)))
    {
      GB_Error((char *)E_BOUND);
      return NULL;
    }
  }

  return (void *)((char *)(THIS->data) + index * TYPE_sizeof_memory(THIS->type));
}


static void release_one(CARRAY *_object, long i)
{
  switch(THIS->type)
  {
    case T_STRING:
      STRING_unref(&(((char **)(THIS->data))[i]));
      break;

    case T_OBJECT:
      OBJECT_unref(&(((void **)(THIS->data))[i]));
      break;

    case T_VARIANT:
      VARIANT_free(&(((VARIANT *)(THIS->data))[i]));
      break;

    default:
      break;
  }
}


static void release(CARRAY *_object, int start, int end)
{
  int i;

  if (end < 0)
    end = ARRAY_count(THIS->data);

  switch(THIS->type)
  {
    case T_STRING:
      for (i = start; i < end; i++)
        STRING_unref(&((char **)(THIS->data))[i]);
      break;

    case T_OBJECT:
      for (i = start; i < end; i++)
        OBJECT_unref(&((void **)(THIS->data))[i]);
      break;

    case T_VARIANT:
      for (i = start; i < end; i++)
        VARIANT_free(&((VARIANT *)(THIS->data))[i]);
      break;

    default:
      break;
  }
}


static void borrow(CARRAY *_object, int start, int end)
{
  int i;

  if (end < 0)
    end = ARRAY_count(THIS->data);

  switch(THIS->type)
  {
    case T_STRING:
      for (i = start; i < end; i++)
        STRING_ref(((char **)(THIS->data))[i]);
      break;

    case T_OBJECT:
      for (i = start; i < end; i++)
        OBJECT_ref(((void **)(THIS->data))[i]);
      break;

    case T_VARIANT:
      for (i = start; i < end; i++)
        VARIANT_keep(&((VARIANT *)(THIS->data))[i]);
      break;

    default:
      break;
  }
}


static void clear(CARRAY *_object)
{
  release(THIS, 0, -1);
  ARRAY_delete(&THIS->data);
  ARRAY_create_with_size(&THIS->data, TYPE_sizeof_memory(THIS->type), 8);
}


static void *insert(CARRAY *_object, long index)
{
  return ARRAY_insert(&THIS->data, index);
}

PUBLIC void CARRAY_split(CARRAY *_object, const char *str, const char *sep, const char *esc, bool many_esc)
{
  char c;
  const char *p;
  long len;
  const char *elt = NULL;
  bool escape;
  char escl, escr;

  if (sep == NULL || *sep == 0)
    sep = ",";

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

  p = str;
  escape = FALSE;

  for(;;)
  {
    c = *str;

    if (escape)
    {
      if (c != 0 && c != escr)
        goto _NEXT;
      else
        c = *(++str);
    }
		else if (c != 0 && c == escl)
    {
      escape = TRUE;
      goto _NEXT;
    }

    if (c != 0 && index(sep, c) == NULL)
      goto _NEXT;

    if (escape)
    {
    	elt = p;
      len = str - p;
      if (len >=2  && elt[0] == escl && elt[len - 1] == escr)
      {
      	len -= 2; elt++;
			}
    }
    else
    {
      len = str - p;
      if (len > 0)
        elt = p;
    }

    if (len <= 0)
    {
      if (!many_esc)
        ARRAY_add_void(&THIS->data);
    }
    else
    {
      STRING_new((char **)ARRAY_add(&THIS->data), elt, len);
    }

    if (c == 0)
      break;

    p = str + 1;
    escape = FALSE;

  _NEXT:

    str++;
  }
}


PUBLIC void CARRAY_get_value(CARRAY *_object, long index, VALUE *value)
{
	VALUE_read(value, get_data(THIS, index), THIS->type);
}



BEGIN_METHOD(CARRAY_new, GB_INTEGER size)

  TYPE type;
  CLASS *klass;
  long size;
  long inc;
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
	else if (klass == CLASS_ObjectArray)
		type = T_OBJECT;
	else
	{
		GB_Error("Bad array type");
		return;
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

    ALLOC_ZERO(&THIS->dim, nsize * sizeof(long), "CARRAY_new");

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

  GB_ReturnInt((long)THIS->data);

END_PROPERTY


BEGIN_PROPERTY(CARRAY_count)

  GB_ReturnInt(ARRAY_count(THIS->data));

END_PROPERTY


BEGIN_PROPERTY(CARRAY_max)

  GB_ReturnInt(ARRAY_count(THIS->data) - 1);

END_PROPERTY


BEGIN_METHOD(CARRAY_remove, GB_INTEGER index; GB_INTEGER count)

  long index = VARG(index);
  long count = VARGOPT(count, 1);

  /* do it after 1.0
    if (index < 0)
    index = ARRAY_count(THIS->data) - index;*/

  if (check_not_multi(THIS))
    return;

  if ((index < 0) || (index >= ARRAY_count(THIS->data)))
  {
    GB_Error((char *)E_ARG);
    return;
  }

  if (count < 0)
    count = ARRAY_count(THIS->data) - index;

  release(THIS, index, index + count);
  ARRAY_remove_many(&THIS->data, index, count);

END_METHOD


BEGIN_METHOD(CARRAY_resize, GB_INTEGER size)

  long size = VARG(size);
  long count = ARRAY_count(THIS->data);

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


static void add(CARRAY *_object, GB_VALUE *value, long index)
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
  add(THIS, (GB_VALUE *)ARG(value), VARGOPT(index, -1)); \
  \
END_METHOD \
\
BEGIN_METHOD(CARRAY_##_type##_push, GB_##_gtype value) \
\
  add(THIS, (GB_VALUE *)ARG(value), -1); \
  \
END_METHOD

IMPLEMENT_add(integer, INTEGER)
IMPLEMENT_add(long, LONG)
IMPLEMENT_add(float, FLOAT)
IMPLEMENT_add(date, DATE)
IMPLEMENT_add(string, STRING)
IMPLEMENT_add(object, OBJECT)
IMPLEMENT_add(variant, VARIANT)


#define IMPLEMENT_put(_type, _gtype) \
BEGIN_METHOD(CARRAY_##_type##_put, GB_##_gtype value; GB_INTEGER index) \
  \
  void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1); \
  if (!data) return; \
  GB_Store(GB_T_##_gtype, (GB_VALUE *)ARG(value), data); \
  \
END_METHOD

#define IMPLEMENT_put2(_type, _gtype, _gstore) \
BEGIN_METHOD(CARRAY_##_type##_put, GB_##_gtype value; GB_INTEGER index) \
\
  void *data = get_data_multi(THIS, ARG(index), GB_NParam() + 1); \
  if (!data) return; \
  GB_Store(GB_T_##_gstore, (GB_VALUE *)ARG(value), data); \
  \
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

  long count = ARRAY_count(THIS->data);
  long start = VARGOPT(start, 0);
  long length = VARGOPT(length, count);
  long i;
  void *data;
  long size;

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

  long pos = VARGOPT(pos, -1);
  CARRAY *array = (CARRAY *)VARG(array);
  int count = ARRAY_count(array->data);
  void *data;

  if (check_not_multi(THIS))
  {
  	GB_ReturnNull();
    return;
	}

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


BEGIN_METHOD(CARRAY_copy, GB_INTEGER start; GB_INTEGER length)

  CARRAY *array;
  int count = ARRAY_count(THIS->data);
  void *data;
  int start = VARGOPT(start, 0);
  int length = VARGOPT(length, -1);

  if (check_not_multi(THIS))
    return;

  if (start < 0)
  {
    GB_Error((char *)E_BOUND);
    return;
  }

  if (start >= count)
    length = 0;

  if (length == -1)
    length = count - start;

  if (length < 0 || (start + length) > count)
  {
    GB_Error((char *)E_BOUND);
    return;
  }

  GB_ArrayNew((GB_ARRAY)&array, THIS->type, 0);

  if (length > 0)
  {
    data = ARRAY_insert_many(&array->data, 0, length);
    memmove(data, get_data(THIS, start), length * TYPE_sizeof_memory(THIS->type));
    borrow(array, 0, -1);
  }

  GB_ReturnObject(array);

END_METHOD


BEGIN_METHOD_VOID(CARRAY_pop)

  long index = ARRAY_count(THIS->data) - 1;

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

  long *index = (long *)GB_GetEnum();

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
  long mode = VARGOPT(mode, 0);
  void *data = THIS->data;

  if (ARRAY_count(data) > 1)
  {
    compare = COMPARE_get(THIS->type, mode);
    qsort(data, ARRAY_count(data), TYPE_sizeof_memory(THIS->type), compare);
  }

  GB_ReturnObject(THIS);

END_METHOD


static void find(CARRAY *_object, int mode, void *value)
{
  COMPARE_FUNC compare = COMPARE_get(THIS->type, mode);
  int i;

  for (i = 0; i < ARRAY_count(THIS->data); i++)
  {
    if ((*compare)(value, get_data(THIS, i)) == 0)
    {
      GB_ReturnInt(i);
      return;
    }
  }

  GB_ReturnInt(-1);
}

#define IMPLEMENT_find(_type, _gtype) \
BEGIN_METHOD(CARRAY_##_type##_find, _gtype value) \
\
  find(THIS, 0, &VARG(value)); \
\
END_METHOD

IMPLEMENT_find(integer, GB_INTEGER)
/*IMPLEMENT_find(short, GB_INTEGER)
IMPLEMENT_find(byte, GB_INTEGER)*/
IMPLEMENT_find(long, GB_LONG)
IMPLEMENT_find(float, GB_FLOAT)
IMPLEMENT_find(date, GB_DATE)
IMPLEMENT_find(object, GB_OBJECT)


BEGIN_METHOD(CARRAY_string_find, GB_STRING value; GB_INTEGER mode)

  char *str = GB_ToZeroString(ARG(value));

  find(THIS, VARGOPT(mode, 0), &str);

END_METHOD


BEGIN_METHOD(CARRAY_string_join, GB_STRING sep; GB_STRING esc)

  char *sep = ",";
  int lsep = 1;
  char *esc = "";
  int lesc = 0;
  char escl, escr;
  char *res = NULL;
  int i;
  char **data = (char **)THIS->data;

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

  for (i = 0; i < ARRAY_count(data); i++)
  {
    if (i)
      STRING_add(&res, sep, lsep);
		if (lesc)
			STRING_add(&res, &escl, 1);
    STRING_add(&res, data[i], STRING_length(data[i]));
		if (lesc)
			STRING_add(&res, &escr, 1);
  }

  if (res)
  {
    STRING_extend_end(&res);
    GB_ReturnString(res);
  }
  else
    GB_ReturnNull();

END_METHOD


BEGIN_METHOD_VOID(CARRAY_reverse)

  size_t size;
  long count;
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

  long count = ARRAY_count(THIS->data);
  long start = VARGOPT(start, 0);
  long length = VARGOPT(length, count);

  if ((start + length) > count)
    length = count - start;

  STREAM_read(CSTREAM_stream(VARG(file)), get_data(THIS, start), length * TYPE_sizeof_memory(THIS->type));

END_METHOD


BEGIN_METHOD(CARRAY_write, GB_OBJECT file; GB_INTEGER start; GB_INTEGER length)

  long count = ARRAY_count(THIS->data);
  long start = VARGOPT(start, 0);
  long length = VARGOPT(length, count);

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


BEGIN_METHOD_VOID(CARRAY_print)

  long dim[MAX_ARRAY_DIM] = { 0 };
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

END_METHOD

#endif /* #ifndef GBX_INFO */


PUBLIC GB_DESC NATIVE_ArrayBounds[] =
{
  GB_DECLARE(".Array.Bounds", sizeof(CARRAY)), GB_NOT_CREATABLE(),

  GB_METHOD("_get", "i", CARRAY_bounds_get, "(Dimension)i"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_Array[] =
{
  GB_DECLARE("Array", sizeof(CARRAY)), GB_NOT_CREATABLE(),

  GB_METHOD("_free", NULL, CARRAY_free, NULL),

  GB_PROPERTY_READ("Type", "i", CARRAY_type),
  GB_PROPERTY_READ("Count", "i", CARRAY_count),
  GB_PROPERTY_READ("Max", "i", CARRAY_max),
  GB_PROPERTY_READ("Length", "i", CARRAY_count),
  GB_PROPERTY_READ("Dim", "i", CARRAY_dim),
  GB_PROPERTY_READ("Data", "i", CARRAY_data),
  GB_PROPERTY_SELF("Bounds", ".Array.Bounds"),

  GB_METHOD("Remove", NULL, CARRAY_remove, "(Index)i[(Count)i]"),
  GB_METHOD("Clear", NULL, CARRAY_clear, NULL),
  GB_METHOD("Resize", NULL, CARRAY_resize, "(Size)i"),
  GB_METHOD("Reverse", NULL, CARRAY_reverse, NULL),

  GB_METHOD("_print", NULL, CARRAY_print, NULL),
  //GB_METHOD("FromString", "i", CARRAY_from_string, "(String)s[(Separator)s(Escape)s]"),
  //GB_METHOD("Sort", NULL, CARRAY_sort, "[(Compare)i]"),
  //GB_METHOD("Find", "i", CARRAY_find, "(Value)s[(Compare)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_BooleanArray[] =
{
  GB_DECLARE("Boolean[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)b[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)b"),
  GB_METHOD("_put", NULL, CARRAY_boolean_put, "(Value)b(Index)i."),
  GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)b"),

  GB_METHOD("Pop", "b", CARRAY_pop, NULL),
  GB_METHOD("_get", "b", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "b", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Boolean[]", CARRAY_insert, "(Array)Boolean[];[(Pos)i]"),
  GB_METHOD("Copy", "Boolean[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Boolean[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)b[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_ByteArray[] =
{
  GB_DECLARE("Byte[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)i[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)i"),
  GB_METHOD("_put", NULL, CARRAY_byte_put, "(Value)i(Index)i."),
  GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)i"),

  GB_METHOD("Pop", "i", CARRAY_pop, NULL),
  GB_METHOD("_get", "i", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "i", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Byte[]", CARRAY_insert, "(Array)Byte[];[(Pos)i]"),
  GB_METHOD("Copy", "Byte[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Byte[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)i[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_ShortArray[] =
{
  GB_DECLARE("Short[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)i[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)i"),
  GB_METHOD("_put", NULL, CARRAY_short_put, "(Value)i(Index)i."),
  GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)i"),

  GB_METHOD("Pop", "i", CARRAY_pop, NULL),
  GB_METHOD("_get", "i", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "i", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Short[]", CARRAY_insert, "(Array)Short[];[(Pos)i]"),
  GB_METHOD("Copy", "Short[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Short[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)i[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_IntegerArray[] =
{
  GB_DECLARE("Integer[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_integer_add, "(Value)i[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_integer_push, "(Value)i"),
  GB_METHOD("_put", NULL, CARRAY_integer_put, "(Value)i(Index)i."),
  GB_METHOD("Find", "i", CARRAY_integer_find, "(Value)i"),

  GB_METHOD("Pop", "i", CARRAY_pop, NULL),
  GB_METHOD("_get", "i", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "i", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Integer[]", CARRAY_insert, "(Array)Integer[];[(Pos)i]"),
  GB_METHOD("Copy", "Integer[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Integer[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)i[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_LongArray[] =
{
  GB_DECLARE("Long[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_long_add, "(Value)l[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_long_push, "(Value)l"),
  GB_METHOD("_put", NULL, CARRAY_long_put, "(Value)l(Index)i."),
  GB_METHOD("Find", "i", CARRAY_long_find, "(Value)l"),

  GB_METHOD("Pop", "l", CARRAY_pop, NULL),
  GB_METHOD("_get", "l", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "l", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Long[]", CARRAY_insert, "(Array)Long[];[(Pos)i]"),

  GB_METHOD("Copy", "Long[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Long[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)l[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_StringArray[] =
{
  GB_DECLARE("String[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_string_add, "(Value)s[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_string_push, "(Value)s"),
  GB_METHOD("_put", NULL, CARRAY_string_put, "(Value)s(Index)i."),
  GB_METHOD("Find", "i", CARRAY_string_find, "(Value)s[(Mode)i]"),

  GB_METHOD("Pop", "s", CARRAY_pop, NULL),
  GB_METHOD("_get", "s", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "s", CARRAY_next, NULL),

  GB_METHOD("Insert", "String[]", CARRAY_insert, "(Array)String[];[(Pos)i]"),

  GB_METHOD("Join", "s", CARRAY_string_join, "[(Separator)s(Escape)s]"),

  /* Inutile ! NE sert qu'�faire appara�re la m�hode dans
     l'explorateur de composant ! */

  GB_METHOD("Copy", "String[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "String[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)s[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_FloatArray[] =
{
  GB_DECLARE("Float[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_float_add, "(Value)f[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_float_push, "(Value)f"),
  GB_METHOD("_put", NULL, CARRAY_float_put, "(Value)f(Index)i."),
  GB_METHOD("Find", "i", CARRAY_float_find, "(Value)f"),

  GB_METHOD("Pop", "f", CARRAY_pop, NULL),
  GB_METHOD("_get", "f", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "f", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Float[]", CARRAY_insert, "(Array)Float[];[(Pos)i]"),

  GB_METHOD("Copy", "Float[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Float[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)f[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_SingleArray[] =
{
  GB_DECLARE("Single[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_float_add, "(Value)f[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_float_push, "(Value)f"),
  GB_METHOD("_put", NULL, CARRAY_single_put, "(Value)f(Index)i."),
  GB_METHOD("Find", "i", CARRAY_float_find, "(Value)f"),

  GB_METHOD("Pop", "f", CARRAY_pop, NULL),
  GB_METHOD("_get", "f", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "f", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Single[]", CARRAY_insert, "(Array)Single[];[(Pos)i]"),

  GB_METHOD("Copy", "Single[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Single[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)f[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_DateArray[] =
{
  GB_DECLARE("Date[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_date_add, "(Value)d[(Index)i]"),
  GB_METHOD("Push", NULL, CARRAY_date_push, "(Value)d"),
  GB_METHOD("_put", NULL, CARRAY_date_put, "(Value)d(Index)i."),
  GB_METHOD("Find", "i", CARRAY_date_find, "(Value)d"),

  GB_METHOD("Pop", "d", CARRAY_pop, NULL),
  GB_METHOD("_get", "d", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "d", CARRAY_next, NULL),

  GB_METHOD("Read", NULL, CARRAY_read, "(Stream)Stream;[(Start)i(Length)i]"),
  GB_METHOD("Write", NULL, CARRAY_write, "(Stream)Stream;[(Start)i(Length)i]"),

  GB_METHOD("Insert", "Date[]", CARRAY_insert, "(Array)Date[];[(Pos)i]"),

  GB_METHOD("Copy", "Date[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Date[]", CARRAY_sort, "[(Mode)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)d[(Start)i(Length)i]"),

  GB_END_DECLARE
};


PUBLIC GB_DESC NATIVE_ObjectArray[] =
{
  GB_DECLARE("Object[]", sizeof(CARRAY)), GB_INHERITS("Array"),

  GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_object_add, "(Value)o[(Index)i.]"),
  GB_METHOD("Push", NULL, CARRAY_object_push, "(Value)o"),
  GB_METHOD("_put", NULL, CARRAY_object_put, "(Value)o(Index)i."),
  GB_METHOD("Find", "i", CARRAY_object_find, "(Value)o"),

  GB_METHOD("Pop", "o", CARRAY_pop, NULL),
  GB_METHOD("_get", "o", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "o", CARRAY_next, NULL),

  GB_METHOD("Insert", "Object[]", CARRAY_insert, "(Array)Object[];[(Pos)i]"),

  GB_METHOD("Copy", "Object[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)o[(Start)i(Length)i]"),
  GB_METHOD("Sort", "Object[]", CARRAY_sort, "[(Mode)i]"),

  GB_END_DECLARE
};

#ifndef GBX_INFO
PUBLIC GB_DESC NATIVE_TemplateArray[ARRAY_TEMPLATE_NDESC] =
{
  GB_DECLARE("*[]", sizeof(CARRAY)), GB_INHERITS("Object[]"),

  //GB_METHOD("_new", NULL, CARRAY_new, "[(Size)i.]"),

  GB_METHOD("Add", NULL, CARRAY_object_add, "(Value)*;[(Index)i.]"),
  GB_METHOD("Push", NULL, CARRAY_object_push, "(Value)*;"),
  GB_METHOD("_put", NULL, CARRAY_object_put, "(Value)*;(Index)i."),
  GB_METHOD("Find", "i", CARRAY_object_find, "(Value)*;"),

  GB_METHOD("Pop", "*", CARRAY_pop, NULL),
  GB_METHOD("_get", "*", CARRAY_get, "(Index)i."),
  GB_METHOD("_next", "*", CARRAY_next, NULL),

  GB_METHOD("Insert", "*", CARRAY_insert, "(Array)*[];[(Pos)i]"),

  GB_METHOD("Copy", "*[]", CARRAY_copy, "[(Start)i(Length)i]"),
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)*[(Start)i(Length)i]"),
  //GB_METHOD("Sort", "Object[]", CARRAY_sort, "[(Mode)i]"),

  GB_END_DECLARE
};
#endif

PUBLIC GB_DESC NATIVE_VariantArray[] =
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
  GB_METHOD("Fill", NULL, CARRAY_fill, "(Value)v[(Start)i(Length)i]"),

  GB_END_DECLARE
};


#ifndef GBX_INFO

/* Gambas API */

PUBLIC void GB_ArrayNew(GB_ARRAY *array, long type, long size)
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
    default: class = CLASS_VariantArray; break;
  }

  OBJECT_create((void **)array, class, NULL, NULL, np);
}

PUBLIC long GB_ArrayCount(GB_ARRAY array)
{
  return ARRAY_count(((CARRAY *)array)->data);
}

PUBLIC void *GB_ArrayAdd(GB_ARRAY array)
{
  return insert((CARRAY *)array, -1);
}

PUBLIC void *GB_ArrayGet(GB_ARRAY array, long index)
{
  return get_data((CARRAY *)array, index);
}

#endif
