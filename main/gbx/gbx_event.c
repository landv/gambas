/***************************************************************************

  event.c

  Event management routines

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_EVENT_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gbx_exec.h"
#include "gbx_api.h"

#include "gbx_event.h"

//#define DEBUG_ME 1

PUBLIC void *EVENT_Last = NULL;
PUBLIC char *EVENT_Name = NULL;

static EVENT_POST *_post_list = NULL;


static void check_event_method(CLASS *class, const char *name, CLASS_DESC_METHOD *desc, CLASS_EVENT *event)
{
  if (event->n_param < desc->npmin)
  {
    /*printf("event->n_param = %d  desc->npmin = %d  desc->npmax = %d\n", event->n_param, desc->npmin, desc->npmax);*/
    THROW(E_EVENT, class->name, name, "Too many arguments");
  }

  if (event->n_param > desc->npmax)
    THROW(E_EVENT, class->name, name, "Not enough arguments");

  if (TYPE_compare_signature(desc->signature, event->n_param, (TYPE *)event->param, event->n_param))
    THROW(E_EVENT, class->name, name, "Type mismatch");

	if (desc->type != T_VOID)
		THROW(E_EVENT, class->name, name, "Not a procedure");
}


PUBLIC void EVENT_search(CLASS *class, ushort *event, const char *name, OBJECT *parent)
{
  CLASS *class_parent;
  int i, index;
  const char *event_name;
  CLASS_DESC *desc;
  int kind;
  char *func_name = COMMON_buffer;

  kind = parent->class == CLASS_Class ? CD_STATIC_METHOD : CD_METHOD;

  if (kind == CD_STATIC_METHOD)
    class_parent = (CLASS *)parent;
  else
    class_parent = parent->class;

  /*
  if (name == NULL)
    name = class->name;
  */

  #if DEBUG_ME
  printf("EVENT_search: class = %s  name = %s  parent = %p\n", class->name, name, parent);
  #endif

	STRING_free(&EVENT_Name);
	STRING_new(&EVENT_Name, name, 0);

  for (i = 0; i < class->n_event; i++)
  {
    event_name = class->event[i].name;
    if (*event_name == ':')
      event_name++;

    if (name)
    {
      snprintf(func_name, COMMON_BUF_MAX, "%s_%s", name, event_name);

      index = CLASS_find_symbol(class_parent, func_name);

      if (index != NO_SYMBOL)
      {
        desc = class_parent->table[index].desc;
        if (CLASS_DESC_get_type(desc) == kind)
          goto FOUND;
      }
    }

    /*snprintf(func_name, COMMON_BUF_MAX, "%s_%s", class->name, event_name);

    index = CLASS_find_symbol(class_parent, func_name);

    if (index != NO_SYMBOL)
    {
      desc = class_parent->table[index].desc;
      if (CLASS_DESC_get_type(desc) == kind)
        goto FOUND;
    }*/

    event[i] = 0;
    continue;

FOUND:

    #if DEBUG_ME
    printf("EVENT_search: FOUND [%d] %s.%s  index = %d  parent = %s.%p\n", i, class_parent->name, func_name, index, parent->class->name, parent);
    #endif
    check_event_method(class_parent, func_name, &desc->method, &class->event[i]);
    event[i] = index + 1;

  }
}



PUBLIC void EVENT_exit()
{
  EVENT_POST *ep;

  while (_post_list)
  {
    ep = _post_list;
    _post_list = _post_list->list.next;
    FREE(&ep, "EVENT_exit");
  }
	
	STRING_free(&EVENT_Name);
}


static void post(void (*func)(), int nparam, long param, long param2)
{
  EVENT_POST *ep;

  /*printf("EVENT_post\n");
  fflush(NULL);*/

  ALLOC(&ep, sizeof(EVENT_POST), "EVENT_post");

  ep->func = func;
  ep->nparam = nparam;
  ep->param = param;
  ep->param2 = param2;

  LIST_insert((void **)&_post_list, ep, &ep->list);

  HOOK(post)();
}


PUBLIC void EVENT_post(void (*func)(), long param)
{
	post(func, 1, param, 0);
}

PUBLIC void EVENT_post2(void (*func)(), long param, long param2)
{
	post(func, 2, param, param2);
}

static void post_event(void *object, int event)
{
	GB_Raise(object, event, 0);
	GB_Unref(&object);
}

PUBLIC void EVENT_post_event(void *object, int event)
{
	if (!GB_CanRaise(object, event))
		return;
	
	GB_Ref(object);
	post((void (*)())post_event, 2, (long)object, (long)event);
}


PUBLIC bool EVENT_check_post(void)
{
  EVENT_POST *ep;
  EVENT_POST save;
  bool ret = FALSE;

  #ifdef DEBUG_POST
  fprintf(stderr, "EVENT_check_post: START\n");
  #endif

  while (_post_list)
  {
    ret = TRUE;
    ep = _post_list;
    save = *ep;
    LIST_remove((void **)&_post_list, _post_list, &_post_list->list);

    FREE(&ep, "EVENT_check_post");

		if (save.nparam == 1)
    	(*save.func)(save.param);
		else
    	(*save.func)(save.param, save.param2);
  }

  /*norec = FALSE;*/

  #ifdef DEBUG_POST
  fprintf(stderr, "EVENT_check_post: END\n");
  #endif

  return ret;
}

