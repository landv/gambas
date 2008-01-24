/***************************************************************************

  Array.c

  Array management routines

  (c) 2000-2007 Benoît Minisini <gambas@users.sourceforge.net>

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

size_t ARRAY_get_size(ARRAY_DESC *desc)
{
  ssize_t size;
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

  size *= TYPE_sizeof_memory(desc->type.id);
  
  return (size_t)size;
}

void ARRAY_new(void **data, ARRAY_DESC *desc)
{
  ALLOC_ZERO(data, ARRAY_get_size(desc), "ARRAY_new");
}


void ARRAY_free_data(void *data, ARRAY_DESC *desc)
{
  ssize_t size;
  intptr_t i;
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

  if (desc->type.id == T_STRING)
  {
    for (i = 0; i < size; i++)
    {
      STRING_unref((char **)array);
      array += sizeof(char *);
    }
  }
  else if (desc->type.id == T_OBJECT)
  {
    for (i = 0; i < size; i++)
    {
      OBJECT_unref((void **)array);
      array += sizeof(void *);
    }
  }
  else if (desc->type.id == T_VARIANT)
  {
    for (i = 0; i < size; i++)
    {
      VARIANT_free((VARIANT *)array);
      array += sizeof(VARIANT);
    }
  }
}



void ARRAY_free(void **data, ARRAY_DESC *desc)
{
  if (*data == NULL)
    return;

  ARRAY_free_data(*data, desc);

  FREE(data, "ARRAY_free");
}

void *ARRAY_get_address(ARRAY_DESC *desc, void *addr, int nparam, int *param)
{
  int max;
  int i;
  boolean stop = FALSE;
  offset_t pos = 0;

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
  
  if (i < (nparam - 1))
  	THROW(E_NDIM);

  return (char *)addr + pos * TYPE_sizeof_memory(desc->type.id);
}



