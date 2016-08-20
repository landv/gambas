/***************************************************************************

  gbx_event.h

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

#ifndef __GBX_EVENT_H
#define __GBX_EVENT_H

#include "gbx_class.h"
#include "gbx_object.h"
#include "gb_list.h"


#ifndef __GBX_EVENT_C
extern void *EVENT_Last;
extern char *EVENT_PreviousName;
extern char *EVENT_Name;
#endif

typedef
  struct _EVENT_POST {
    LIST list;
    void (*func)();
    int nparam;
    intptr_t param;
    intptr_t param2;
    }
  EVENT_POST;


void EVENT_search(CLASS *class, ushort *event, const char *name, OBJECT *parent);
void EVENT_post(void (*func)(), intptr_t param);
void EVENT_post2(void (*func)(), intptr_t param, intptr_t param2);
bool EVENT_check_post(void);
void EVENT_init(void);
void EVENT_exit(void);
void EVENT_post_event(void *object, int event);
char *EVENT_enter_name(const char *name);
void EVENT_leave_name(const char *save);

#endif
