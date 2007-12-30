/***************************************************************************

  CList.h

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

#ifndef __GBX_C_LIST_H
#define __GBX_C_LIST_H

#include "gambas.h"

#include "gbx_variant.h"
#include "gbx_object.h"

typedef
  struct _clist_node {
    struct _clist_node *next;
    struct _clist_node *prev;
    VARIANT value;
    }
  CLIST_NODE;
    
typedef
  struct {
    OBJECT object;
    long length;
    long index;
    CLIST_NODE *first;
    CLIST_NODE *last;
    CLIST_NODE *current;
    }
  CLIST;
  
#ifndef __GBX_C_LIST_C

extern GB_DESC NATIVE_List[];

#else

#define THIS ((CLIST *)_object)

#endif

#endif
