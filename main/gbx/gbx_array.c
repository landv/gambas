/***************************************************************************

  Array.c

  Array management routines

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

#define __GB_ARRAY_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbx_api.h"
#include "gambas.h"
#include "gbx_class.h"
#include "gbx_exec.h"

#include "gbx_array.h"


PUBLIC void ARRAY_new(void **data, ARRAY_DESC *desc)
{
  long size;
  int i;

  size = 1;

  for (i = 0;; i++)
  {
    size *= desc->dim[i];
    if (size < 0)
    {
      size = (-size);
      break;
    }
  }

  size *= TYPE_sizeof_memory(desc->type);

  /*
  size += sizeof(TYPE);

  size += ndim * sizeof(long);
  */

  ALLOC_ZERO(data, size, "ARRAY_new");

  /*
  desc->type = type;

  for (i = 0; i < ndim; i++)
    desc->dim[i] = dim[i];

  *array = desc;
  *data = &desc->dim[ndim];
  */
}


PUBLIC void ARRAY_free_data(void *data, ARRAY_DESC *desc)
{
  long size;
  int i;
  char *array;

  size = 1;

  for (i = 0;; i++)
  {
    size *= desc->dim[i];
    if (size < 0)
    {
      size = (-size);
      break;
    }
  }

  array = data;

  if (desc->type == T_STRING)
  {
    for (i = 0; i < size; i++)
    {
      STRING_unref((char **)array);
      array += sizeof(char *);
    }
  }
  else if (TYPE_is_object(desc->type))
  {
    for (i = 0; i < size; i++)
    {
      OBJECT_unref((void **)array);
      array += sizeof(void *);
    }
  }
  else if (desc->type == T_VARIANT)
  {
    for (i = 0; i < size; i++)
    {
      VARIANT_free((VARIANT *)array);
      array += sizeof(VARIANT);
    }
  }
}



PUBLIC void ARRAY_free(void **data, ARRAY_DESC *desc)
{
  if (*data == NULL)
    return;

  ARRAY_free_data(*data, desc);

  FREE(data, "ARRAY_free");
}

PUBLIC void *ARRAY_get_address(ARRAY_DESC *desc, void *addr, int nparam, long *param)
{
  long max;
  int i;
  boolean stop = FALSE;
  long pos = 0;

  for (i = 0;; i++)
  {
    if (i >= nparam)
      THROW(E_NDIM);

    max = desc->dim[i];
    if (max < 0)
    {
      max = (-max);
      stop = TRUE;
    }

    if (param[i] < 0 || param[i] >= max)
      THROW(E_BOUND);

    pos *= max;
    pos += param[i];

    if (stop)
      break;
  }

  return (char *)addr + pos * TYPE_sizeof_memory(desc->type);
}


#if 0

static void array_free();



static long get_size(CARRAY *array)
{
 long size = array->size;
  int i;

  if (size < 0)
  {
    size = 1;
    for (i = 0; i < (-(array->size)); i++)
      size *= array->dim[i];
  }

  return size;
}


static void *get_data(CARRAY *array)
{
  if (array->size >= 0)
    return &array->dim[0];
  else
    return &array->dim[-array->size];
}


PUBLIC void *CARRAY_get_address(CARRAY *array, int nparam, long *param)
{
  /*static char array_size[16] = { 0, 1, 1, 2, 4, 8, 8, 8, 4, 0, 0, 12, 0, 0, 0, 4 };*/

  long dim, ndim, pos, index;
  char *data;

  if (array->size >= 0)
  {
    if (nparam != 1)
      THROW(E_NDIM);

    dim = *param;

    if (dim < 0 || dim >= array->size)
      THROW(E_BOUND);

    data = (char *)&array->dim[0];
    index = dim;
  }
  else
  {
    ndim = (-array->size);

    if (nparam != ndim)
      THROW(E_NDIM);

    for (dim = 0, pos = 0; dim < ndim; dim++)
    {
      if (param[dim] < 0 || param[dim] >= array->dim[dim])
        THROW(E_BOUND);

      pos *= array->dim[dim];
      pos += param[dim];
    }

    data = (char *)&array->dim[ndim];
    index = pos;
  }

  return data + index * TYPE_sizeof(array->type);
}


static void array_free(CLASS *class, CARRAY *array)
{
  long size, i;
  TYPE type;

  size = get_size(array);
  type = (array)->type;

  if (type == T_STRING)
  {
    char **str = (char **)get_data(array);

    for (i = 0; i < size; i++, str++)
      STRING_free(str);
  }
  else if (TYPE_is_object(type))
  {
    void **ob = (void **)get_data(array);

    for (i = 0; i < size; i++, ob++)
      OBJECT_unref(ob);
  }
  else if (TYPE_is_variant(type))
  {
    VARIANT *ob = (VARIANT *)get_data(array);

    for (i = 0; i < size; i++, ob++)
      VARIANT_free(ob);
  }
}


BEGIN_PROPERTY(array_count)

  GB_ReturnInt(get_size(OBJECT(CARRAY)));

END_PROPERTY


BEGIN_METHOD(array_length, long dim)

  int ndim;
  long dim;
  CARRAY *array = OBJECT(CARRAY);

  if (array->size < 0)
  {
    if (GB_IsMissing(1))
      THROW(E_NEPARAM);

    dim = PARAM(dim);
    ndim = (-array->size);
    if ((dim < 0) || (dim >= ndim))
      THROW(E_ARG);

    GB_ReturnInt(array->dim[dim]);
  }
  else
  {
    if (!GB_IsMissing(1) && (PARAM(dim) != 0))
      THROW(E_ARG);

    GB_ReturnInt(array->size);
  }

END_METHOD


BEGIN_PROPERTY(array_dim)

  CARRAY *array = OBJECT(CARRAY);

  if (array->size > 0)
    GB_ReturnInt(1);
  else
    GB_ReturnInt(-array->size);

END_PROPERTY



PUBLIC GB_DESC NATIVE_Array[] =
{
  GB_DECLARE("Array", 0, NULL),

  GB_HOOK_NEW(NULL),
  GB_HOOK_FREE(array_free),

  GB_METHOD("Length", "i", array_length, "[i]"),
  GB_PROPERTY_READ("Count", "i", array_count),
  GB_PROPERTY_READ("Dim", "i", array_dim),

  GB_CAST("Array"),

  GB_END_DECLARE
};

#endif

