/***************************************************************************

  event.h

  Event management routines

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_EVENT_H
#define __GBX_EVENT_H

#include "gbx_class.h"
#include "gbx_object.h"
#include "gbx_list.h"


#ifndef __GBX_EVENT_C
extern void *EVENT_Last;
#endif

typedef
  struct _EVENT_POST {
    LIST list;
    void (*func)();
    int nparam;
    long param;
    long param2;
    }
  EVENT_POST;


PUBLIC void EVENT_search(CLASS *class, ushort *event, const char *name, OBJECT *parent);
PUBLIC void EVENT_post(void (*func)(), long param);
PUBLIC void EVENT_post2(void (*func)(), long param, long param2);
PUBLIC bool EVENT_check_post(void);
PUBLIC void EVENT_init(void);
PUBLIC void EVENT_exit(void);
PUBLIC void EVENT_post_event(void *object, int event);

#endif
