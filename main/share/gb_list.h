/***************************************************************************

  gb_list.h

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

#define LIST_for_each_name(_var, _first, _name) \
  for(_var = (_first); _var; _var = (_var)->_name.next)

#define LIST_for_each(_var, _first) LIST_for_each_name(_var, _first, list)
  
#endif
