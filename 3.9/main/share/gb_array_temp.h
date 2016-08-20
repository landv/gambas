/***************************************************************************

  gb_array_temp.h

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

#define __ARRAY_C

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_array.h"

void ARRAY_create_with_size(void *p_data, size_t size, int inc)
{
  ARRAY *array;

  ALLOC(&array, sizeof(ARRAY));

  array->count = 0;
  array->max = 0;
  array->size = (int)size;
  if (size > 2 && (size & 3))
    fprintf(stderr, "WARNING: ARRAY_create_with_size: size = %zi\n", size);
  array->inc = inc;

  *((void **)p_data) = ARRAY_TO_DATA(array);
}


void ARRAY_delete(void *p_data)
{
  void **data = (void **)p_data;
  ARRAY *alloc = DATA_TO_ARRAY(*data);

  if (!*data)
    return;

  FREE(&alloc);

  *data = NULL;
}

void *ARRAY_add_data(void *p_data, int num, bool zero)
{
  void **data = (void **)p_data;
  register ARRAY *array = DATA_TO_ARRAY(*data);
  ARRAY *new_array;
  char *ptr;

  array->count += num;

  if (array->count > array->max)
  {
    array->max = array->inc + ((array->count + array->inc) / array->inc) * array->inc;
    new_array = array;
    REALLOC(&new_array, sizeof(ARRAY) + array->max * array->size);
    array = new_array;
    *data = ARRAY_TO_DATA(array);
  }

  ptr = (char *)array + sizeof(ARRAY) + array->size * (array->count - num);

  if (zero) memset(ptr, 0, array->size * num);

  return ptr;
}

void ARRAY_realloc(void *p_data) //, bool zero)
{
  void **data = (void **)p_data;
  ARRAY *array = DATA_TO_ARRAY(*data);
  ARRAY *new_array;
	//int old_max = array->max;
	int size = array->size;
	
	array->max = array->inc + ((array->count + array->inc) / array->inc) * array->inc;
	new_array = array;
	REALLOC(&new_array, sizeof(ARRAY) + array->max * size);
  *data = ARRAY_TO_DATA(new_array);
	//fprintf(stderr, "ARRAY_realloc: %p (%d) -> %p (%d) [%d]\n", array, old_max, new_array, new_array->max, size);
	/*if (zero)
	{
		//fprintf(stderr, "ARRAY_realloc: memset(%p, 0, %d)\n", ARRAY_TO_DATA(new_array) + old_max * size, (new_array->max - old_max) * size);
		memset(ARRAY_TO_DATA(new_array) + old_max * size, 0, (new_array->max - old_max) * size);
	}*/
}


void *ARRAY_add_data_one(void *p_data, bool zero)
{
  void **data = (void **)p_data;
  register ARRAY *array = DATA_TO_ARRAY(*data);
  ARRAY *new_array;
	int size = array->size;
  char *ptr;

  array->count++;

  if (array->count > array->max)
  {
    array->max = array->inc + ((array->count + array->inc) / array->inc) * array->inc;
    new_array = array;
    REALLOC(&new_array, sizeof(ARRAY) + array->max * size);
    array = new_array;
    *data = ARRAY_TO_DATA(array);
  }

  ptr = (char *)array + sizeof(ARRAY) + size * (array->count - 1);
  
  if (zero) memset(ptr, 0, size);

  return ptr;
}



void ARRAY_remove_last(void *p_data)
{
  void **data = (void **)p_data;
  ARRAY *array = DATA_TO_ARRAY(*data);

  if (!array->count)
    return;

  array->count--;
}

#if 0
void *ARRAY_get(void *data, int pos)
{
  ARRAY *array = DATA_TO_ARRAY(data);

  return (char *)data + array->size * pos;
}
#endif

void *ARRAY_insert_many(void *p_data, int pos, int count)
{
  void **data;
  ARRAY *array;
  char *addr;
  int len;

  data = (void **)p_data;
  array = DATA_TO_ARRAY(*data);

  if ((pos < 0) || (pos > array->count))
    pos = array->count;

  ARRAY_add_many(p_data, count);
  array = DATA_TO_ARRAY(*data);

  addr = ((char *)(*data)) + array->size * pos;
  len = (array->count - pos - count) * array->size;

  if (len > 0)
    memmove(addr + array->size * count, addr, len);

  memset(addr, 0, array->size * count);

  return addr;
}


void ARRAY_remove_many(void *p_data, int pos, int count)
{
  void **data = (void **)p_data;
  ARRAY *array = DATA_TO_ARRAY(*data);
  char *addr;
  int len;

  if ((pos < 0) || (pos >= array->count))
    return;

  if (count < 0 || count > (array->count - pos))
    count = array->count - pos;

  addr = ((char *)(*data)) + array->size * pos;
  len = (array->count - pos - count) * array->size;

  if (len > 0)
    memmove(addr, addr + array->size * count, len);

  array->count -= count;

  if (array->max <= array->inc)
    return;

  if (array->count > (array->max / 2))
    return;

  array->max = ((array->count + array->inc) / array->inc) * array->inc;
  REALLOC(&array, sizeof(ARRAY) + array->max * array->size);
  *data = ARRAY_TO_DATA(array);
}


void ARRAY_qsort(void *data, ARRAY_COMP_FUNC cmp)
{
  ARRAY *array = DATA_TO_ARRAY(data);

  if (array->count)
    qsort(data, array->count, array->size, cmp);
}

