/***************************************************************************

  gb_list_temp.h

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

#define __GB_LIST_C

#include "gb_common.h"
#include "gb_list.h"

/*#define DEBUG*/

#define TO_LIST(_node) ((LIST *)(((char *)_node) + ((char *)list - (char *)node)))


void LIST_insert(void *p_first, void *node, LIST *list)
{
  void **first = (void **)p_first;
  void *last;

  if (*first == NULL)
  {
    *first = node;
    TO_LIST(node)->prev = node;
    TO_LIST(node)->next = NULL;
    return;
  }

  last = TO_LIST(*first)->prev;

  TO_LIST(last)->next = node;

  TO_LIST(node)->prev = last;
  TO_LIST(node)->next = NULL;

  TO_LIST(*first)->prev = node;
}


void LIST_remove(void *p_first, void *node, LIST *list)
{
  void **first = (void **)p_first;
  void *next, *prev, *last;

  next = TO_LIST(node)->next;
  prev = TO_LIST(node)->prev;

  if (*first == node)
  {
    if (next)
      TO_LIST(next)->prev = prev;

    *first = next;
  }
  else
  {
    last = TO_LIST(*first)->prev;

    if (node == last)
      TO_LIST(*first)->prev = prev;

    if (prev)
      TO_LIST(prev)->next = next;

    if (next)
      TO_LIST(next)->prev = prev;
  }

  TO_LIST(node)->prev = NULL;
  TO_LIST(node)->next = NULL;
}

