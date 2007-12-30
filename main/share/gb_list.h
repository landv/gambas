/***************************************************************************

  list.h

  Linked lists management routines

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

#ifndef __GB_LIST_H
#define __GB_LIST_H

typedef
  struct
  {
    void *next;
    void *prev;
  }
  LIST;

void LIST_insert(void *p_first, void *node, LIST *list);
void LIST_remove(void *p_first, void *node, LIST *list);

#define LIST_for_each(_var, _first) \
  for(_var = (_first); _var; _var = (_var)->list.next)

#endif
