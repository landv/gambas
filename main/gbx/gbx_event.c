/***************************************************************************

  gbx_event.c

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

#define __GBX_EVENT_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gbx_exec.h"
#include "gbx_api.h"

#include "gbx_event.h"

//#define DEBUG_ME 1

void *EVENT_Last = NULL;

char *EVENT_PreviousName = NULL;
char *EVENT_Name = NULL;

static EVENT_POST *_post_list = NULL;


static void check_event_method(CLASS *class, const char *name, CLASS_DESC_METHOD *desc, CLASS_EVENT *event)
{
	int n = event->n_param;
	
	if (n < desc->npmin)
	{
		/*printf("event->n_param = %d  desc->npmin = %d  desc->npmax = %d\n", event->n_param, desc->npmin, desc->npmax);*/
		THROW(E_EVENT, CLASS_get_name(class), name, "Too many arguments");
	}

	if (n > desc->npmax)
	{
		if (!desc->npvar)
			THROW(E_EVENT, CLASS_get_name(class), name, "Not enough arguments");
		
		n = desc->npmax;
	}

	if (TYPE_compare_signature(desc->signature, n, (TYPE *)event->param, n, FALSE))
		THROW(E_EVENT, CLASS_get_name(class), name, "Type mismatch");

	if (desc->type != T_VOID)
		THROW(E_EVENT, CLASS_get_name(class), name, "Not a procedure");
}


void EVENT_search(CLASS *class, ushort *event, const char *name, OBJECT *parent)
{
	//ushort *event;
	//ushort *self;
	CLASS *class_parent;
	int i, index;
	const char *event_name;
	CLASS_DESC *desc;
	int kind;
	int len_name, len_event_name;
	char *func_name = COMMON_buffer;
	char *pevent_name;

	#if DEBUG_ME
	fprintf(stderr, "EVENT_search: class = %s  name = %s  parent = %p\n", class->name, name, parent);
	#endif

	//event = OBJECT_event(object)->event;
	//self = OBJECT_event(object)->self;

	if (class->n_event <= 0)
		return;

	len_name = name ? strlen(name) : 0;
	
	if (len_name == 0 || len_name >= (COMMON_BUF_MAX - 4))
	{
		memset(event, 0, class->n_event * sizeof(ushort));
		return; 
	}

	kind = parent->class == CLASS_Class ? CD_STATIC_METHOD : CD_METHOD;

	if (kind == CD_STATIC_METHOD)
		class_parent = (CLASS *)parent;
	else
		class_parent = parent->class;
	
	strcpy(func_name, name);
	pevent_name = &func_name[len_name];
	*pevent_name++ = '_';

	for (i = 0; i < class->n_event; i++)
	{
		event[i] = 0;
					
		event_name = class->event[i].name;
		if (*event_name == ':')
			event_name++;
		
		len_event_name = strlen(event_name);
		if ((len_name + len_event_name + 1) >= COMMON_BUF_MAX)
			continue;
		
		strcpy(pevent_name, event_name);
		index = CLASS_find_symbol(class_parent, func_name);

		if (index != NO_SYMBOL)
		{
			desc = class_parent->table[index].desc;
			if (CLASS_DESC_get_type(desc) == kind)
			{
				#if DEBUG_ME
				fprintf(stderr, "EVENT_search: FOUND [%d] %s.%s  index = %d  parent = %s.%p\n", i, class_parent->name, func_name, index, parent->class->name, parent);
				#endif
				check_event_method(class_parent, func_name, &desc->method, &class->event[i]);
				event[i] = index + 1;
			}
		}
		
		#if 0
		snprintf(func_name, COMMON_BUF_MAX, "ME_%s", event_name);
		index = CLASS_find_symbol(class, func_name);		
	
		if (index != NO_SYMBOL)
		{
			desc = class->table[index].desc;
			if (CLASS_DESC_get_type(desc) == CD_METHOD)
			{
				#if DEBUG_ME
				printf("EVENT_search: FOUND [%d] %s.%s  index = %d  parent = %s.%p\n", i, class->name, func_name, index, parent->class->name, parent);
				#endif
				check_event_method(class, func_name, &desc->method, &class->event[i]);
				if (!self)
				{
					ALLOC_ZERO(&self, sizeof(ushort) * class->n_event, "EVENT_search");
					OBJECT_event(object)->self = self;
				}
				self[i] = index + 1;
			}
		}
		#endif		
	}
}



void EVENT_exit()
{
	EVENT_POST *ep;

	while (_post_list)
	{
		ep = _post_list;
		_post_list = _post_list->list.next;
		FREE(&ep);
	}
	
	STRING_free(&EVENT_Name);
}


static void post(void (*func)(), int nparam, intptr_t param, intptr_t param2)
{
	EVENT_POST *ep;

	/*printf("EVENT_post\n");
	fflush(NULL);*/

	ALLOC(&ep, sizeof(EVENT_POST));

	ep->func = func;
	ep->nparam = nparam;
	ep->param = param;
	ep->param2 = param2;

	LIST_insert(&_post_list, ep, &ep->list);

	HOOK(post)();
}


void EVENT_post(void (*func)(), intptr_t param)
{
	post(func, 1, param, 0);
}

void EVENT_post2(void (*func)(), intptr_t param, intptr_t param2)
{
	post(func, 2, param, param2);
}

static void post_event(void *object, int event)
{
	GB_Raise(object, event, 0);
	OBJECT_UNREF(object);
}

void EVENT_post_event(void *object, int event)
{
	OBJECT_REF(object);
	post((void (*)())post_event, 2, (intptr_t)object, (intptr_t)event);
}


bool EVENT_check_post(void)
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
		LIST_remove(&_post_list, _post_list, &_post_list->list);

		FREE(&ep);

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


char *EVENT_enter_name(const char *name)
{
	char *save = EVENT_PreviousName;
	EVENT_PreviousName = EVENT_Name;
	EVENT_Name = (char *)name;
	return save;
}

void EVENT_leave_name(const char *save)
{
	EVENT_Name = EVENT_PreviousName;
	EVENT_PreviousName = (char *)save;
}
