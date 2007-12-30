/***************************************************************************

  gbx_c_list.c

  The List native class.

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

#define __GBX_C_LIST_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_error.h"

#include "gbx_variant.h"
#include "gambas.h"
#include "gbx_api.h"

#include "gbx_c_list.h"


static bool move_to(CLIST *_object, int index)
{
  if (index < 0 || index >= THIS->length)
    return TRUE;
    
  if (index == 0)
  {
    THIS->current = THIS->first;
    THIS->index = index;
  }
  else if (index == (THIS->length - 1))
  {
    THIS->current = THIS->last;
    THIS->index = index;
  }
  else if (index > THIS->index)
  {
    do
    {
      THIS->index++;
      THIS->current = THIS->current->next;
    }
    while (index > THIS->index);
  }
  else if (index < THIS->index)
  {
    do
    {
      THIS->index--;
      THIS->current = THIS->current->prev;
    }
    while (index < THIS->index);
  }
  
  return FALSE;
}

static void add_before(CLIST *_object, GB_VALUE *value, long index)
{
  CLIST_NODE *node;
  
  ALLOC_ZERO(&node, sizeof(CLIST_NODE), "add_before");
  
  if (move_to(THIS, index))
  {
    node->prev = THIS->last;
    node->next = NULL;
    index = THIS->length + 1;
  }
  else
  {
    node->prev = THIS->current->prev;
    node->next = THIS->current;
  }

  if (node->prev == NULL)
    THIS->first = node;
  else
    node->prev->next = node;
    
  if (node->next == NULL)
    THIS->last = node;
  else
    node->next->prev = node;
    
  THIS->length++;
  
  GB_Store(GB_T_VARIANT, value, &node->value);
  
  THIS->current = node;
  THIS->index = index;
}

static void remove_current(CLIST *_object)
{
  CLIST_NODE *node = THIS->current;
  
  if (!node)
    return;
  
  THIS->current = node->next;
  
  if (node->prev)  
    node->prev->next = node->next;
  else
    THIS->first = node->next;
    
  if (node->next)
    node->next->prev = node->prev;
  else
    THIS->last = node->prev;
    
  VARIANT_free(&node->value);
  FREE(&node, "remove_current");
  
  THIS->length--;
  if (THIS->length == 0)
    THIS->index = -1;
}

static void free_all(CLIST *_object)
{
  THIS->index = 0;
  
  while (THIS->first)
  {
    THIS->current = THIS->first;
    remove_current(THIS);
  }
}


BEGIN_METHOD_VOID(CLIST_new)

  THIS->index = -1;

END_METHOD


BEGIN_METHOD_VOID(CLIST_free)

  free_all(THIS);

END_METHOD


BEGIN_PROPERTY(CLIST_count)

  GB_ReturnInteger(THIS->length);

END_PROPERTY

BEGIN_PROPERTY(CLIST_max)

  GB_ReturnInteger(THIS->length - 1);

END_PROPERTY

#if 0
BEGIN_PROPERTY(CLIST_index)

  GB_ReturnInteger(THIS->index);

END_PROPERTY


BEGIN_PROPERTY(CLIST_current)

  if (!THIS->current)
  {
    GB_Error("No current node");
    return;
  }
  
  GB_ReturnPtr(T_VARIANT, &THIS->current->value);

END_PROPERTY


BEGIN_PROPERTY(CLIST_available)

  GB_ReturnBoolean(THIS->current != NULL);

END_PROPERTY
#endif

BEGIN_METHOD(CLIST_add, GB_VARIANT value; GB_INTEGER before)

  long before = VARGOPT(before, -1);
  
  add_before(THIS, (GB_VALUE *)ARG(value), before);  

END_METHOD


BEGIN_METHOD(CLIST_remove, GB_INTEGER index)

  if (move_to(THIS, VARG(index)))
  {
    GB_Error((char *)E_ARG);
    return;
  }
  
  remove_current(THIS);
      
END_METHOD


BEGIN_METHOD_VOID(CLIST_clear)

  free_all(THIS);

END_METHOD

#if 0
BEGIN_METHOD(CLIST_move_to, GB_INTEGER index)

  GB_ReturnBoolean(move_to(THIS, VARG(index)));

END_METHOD


BEGIN_METHOD_VOID(CLIST_move_first)

  GB_ReturnBoolean(move_to(THIS, 0));

END_METHOD


BEGIN_METHOD_VOID(CLIST_move_last)

  GB_ReturnBoolean(move_to(THIS, THIS->length - 1));

END_METHOD


BEGIN_METHOD_VOID(CLIST_move_previous)

  GB_ReturnBoolean(move_to(THIS, THIS->index - 1));

END_METHOD


BEGIN_METHOD_VOID(CLIST_move_next)

  GB_ReturnBoolean(move_to(THIS, THIS->index + 1));

END_METHOD
#endif

BEGIN_METHOD(CLIST_get, GB_INTEGER index)

  if (move_to(THIS, VARG(index)))
  {
    GB_Error((char *)E_BOUND);
    return;
  }
  
  GB_ReturnPtr(T_VARIANT, &THIS->current->value);

END_METHOD


BEGIN_METHOD(CLIST_put, GB_VARIANT value; GB_INTEGER index)

  if (move_to(THIS, VARG(index)))
  {
    GB_Error((char *)E_BOUND);
    return;
  }
  
  GB_Store(GB_T_VARIANT, (GB_VALUE *)ARG(value), &THIS->current->value);

END_METHOD


BEGIN_METHOD_VOID(CLIST_next)

  long *index = (long *)GB_GetEnum();

  if (move_to(THIS, *index))
    GB_StopEnum();
  else
  {
    GB_ReturnPtr(T_VARIANT, &THIS->current->value);
    (*index)++;
  }

END_METHOD

#endif

PUBLIC GB_DESC NATIVE_List[] =
{
  GB_DECLARE("List", sizeof(CLIST)),

  GB_METHOD("_new", NULL, CLIST_new, NULL),
  GB_METHOD("_free", NULL, CLIST_free, NULL),

  GB_PROPERTY_READ("Count", "i", CLIST_count),
  GB_PROPERTY_READ("Length", "i", CLIST_count),
  GB_PROPERTY_READ("Max", "i", CLIST_max),
  /*GB_PROPERTY_READ("Index", "i", CLIST_index),
  GB_PROPERTY_READ("Current", "v", CLIST_current),
  GB_PROPERTY_READ("Value", "v", CLIST_current),
  GB_PROPERTY_READ("Available", "b", CLIST_available),*/

  GB_METHOD("Add", NULL, CLIST_add, "(Value)v[(Index)i]"),
  GB_METHOD("Remove", NULL, CLIST_remove, "(Index)i"),
  GB_METHOD("Clear", NULL, CLIST_clear, NULL),
  
  GB_METHOD("_next", "v", CLIST_next, NULL),
  GB_METHOD("_get", "v", CLIST_get, "(Index)i"),
  GB_METHOD("_put", NULL, CLIST_put, "(Value)v(Index)i"),
  
  /*GB_METHOD("MoveTo", "b", CLIST_move_to, "(Index)i"),
  GB_METHOD("MoveFirst", "b", CLIST_move_first, NULL),
  GB_METHOD("MoveLast", "b", CLIST_move_last, NULL),
  GB_METHOD("MoveNext", "b", CLIST_move_next, NULL),
  GB_METHOD("MovePrevious", "b", CLIST_move_previous, NULL),*/

  GB_END_DECLARE
};


