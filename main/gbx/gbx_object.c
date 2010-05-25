/***************************************************************************

  gbx_object.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __OBJECT_C

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_list.h"
#include "gbx_class.h"
#include "gbx_event.h"
#include "gbx_exec.h"
#include "gbx_compare.h"
#include "gbx_c_gambas.h"
#include "gbx_struct.h"
#include "gbx_object.h"

#if DEBUG_REF
const char *OBJECT_ref_where = 0;
#endif

static OBJECT *EventObject = NULL;

void OBJECT_new(void **ptr, CLASS *class, const char *name, OBJECT *parent)
{
  OBJECT_alloc(ptr, class, class->size);

  //#if DEBUG_EVENT
  //printf("OBJECT_new: %s %p %d\n", class->name, *ptr, class->off_event);
  //#endif

  OBJECT_attach(*ptr, parent, name);
}


void OBJECT_alloc(void **ptr, CLASS *class, size_t size)
{
  OBJECT *object;

  if (size < sizeof(OBJECT))
    size = sizeof(OBJECT);

  ALLOC_ZERO(&object, size, "OBJECT_alloc");

  object->class = class;
	#if DEBUG_REF
	object->ref = 0;
	OBJECT_REF(object, "OBJECT_alloc");
	#else
  object->ref = 1;
	#endif

  class->count++;

  *ptr = object;
	
}

#if 0
static void dump_attach(char *title)
{
	void *ob;

	fprintf(stderr, ">>>> %s: ", title);
	for (ob = EventObject; ob; ob = OBJECT_event(ob)->next)
		fprintf(stderr, "%p -> ", ob);
	fprintf(stderr, "(nil)\n");

}
#endif

void OBJECT_detach(OBJECT *ob)
{
  CLASS *class = OBJECT_class(ob);
  OBJECT *parent;
  OBJECT_EVENT *ev;
  bool lock;

  ev = (OBJECT_EVENT *)((char *)ob + class->off_event);

  //if (!ev->parent)
  //	return;

  if (ev->prev)
    OBJECT_event(ev->prev)->next = ev->next;

  if (ev->next)
    OBJECT_event(ev->next)->prev = ev->prev;

  if (ob == EventObject)
    EventObject = ev->next;

  ev->prev = NULL;
  ev->next = NULL;

	//dump_attach("OBJECT_detach");

  // Do not free the observers there anymore
  
  /* Avoids an infinite recursion, if freeing the parent implies freeing the object */
  parent = OBJECT_parent(ob);

  if (parent)
  {
		if (class->special[SPEC_ATTACH] != NO_SYMBOL)
		{
			STACK_check(2);
			
			SP->_object.class = OBJECT_class(parent);
			SP->_object.object = parent;
			PUSH();
			
			SP->type = T_NULL;
			SP++;
			
			EXEC_special(SPEC_ATTACH, class, ob, 2, TRUE);
		}
  	
		lock = OBJECT_is_locked(ob);
    ev->parent = NULL;
    OBJECT_lock(ob, lock);
    #if DEBUG_EVENT || DEBUG_REF
      fprintf(stderr, "OBJECT_detach : Detach (%s %p) from (%s %p)\n",
        ob->class->name, ob, parent->class->name, parent);
    #endif
    OBJECT_UNREF(parent, "OBJECT_detach");
  }  
}

static void remove_observers(OBJECT *ob)
{
	CLASS *class = OBJECT_class(ob);
  OBJECT_EVENT *ev;
  COBSERVER *obs, *next;

  ev = (OBJECT_EVENT *)((char *)ob + class->off_event);
  obs = ev->observer;
  
  while (obs)
  {
  	next = obs->list.next;
  	#if DEBUG_EVENT
  	fprintf(stderr, "Remove observer %p\n", obs);
  	#endif
  	OBJECT_UNREF(obs, "remove_observers");
  	obs = next;  	
	}
  
  ev->observer = NULL;
}

void OBJECT_attach(OBJECT *ob, OBJECT *parent, const char *name)
{
  CLASS *class = OBJECT_class(ob);
  OBJECT_EVENT *ev;
  bool lock;

	if (EVENT_Name != name)
	{
		STRING_unref(&EVENT_Name);
		if (name)
			STRING_new_zero(&EVENT_Name, name);
		else
			EVENT_Name = NULL;
	}

  if (!name)
		return;
		
	lock = OBJECT_is_locked(ob);

  OBJECT_detach(ob);

  ev = (OBJECT_EVENT *)((char *)ob + class->off_event);

  ev->parent = parent;

	OBJECT_lock(ob, lock);

  #if DEBUG_EVENT || DEBUG_REF
    fprintf(stderr, "OBJECT_attach : Attach (%s %p) to (%s %p) as %s\n",
      ob->class->name, ob, parent->class->name, parent, name);
  #endif
  OBJECT_REF(parent, "OBJECT_attach");

  EVENT_search(class, ev->event, name, parent);

  ev->next = EventObject;
  ev->prev = NULL;

  if (EventObject)
    OBJECT_event(EventObject)->prev = ob;

  EventObject = ob;

	if (class->special[SPEC_ATTACH] != NO_SYMBOL)
	{
		STACK_check(2);
		
		SP->_object.class = OBJECT_class(parent);
		SP->_object.object = parent;
		PUSH();
		
		SP->type = T_STRING;
		SP->_string.addr = EVENT_Name;
		SP->_string.start = 0;
		SP->_string.len = STRING_length(EVENT_Name);
		PUSH();
		
		EXEC_special(SPEC_ATTACH, class, ob, 2, TRUE);
	}
	
	//dump_attach("OBJECT_attach");
}


void OBJECT_free(CLASS *class, OBJECT *ob)
{
  /* Il faut emp�her EXEC_leave() de relib�er l'objet */
  /*((OBJECT *)free_ptr)->ref = 1;*/

  /*OBJECT_REF(ob, "OBJECT_free");*/

  /* Ex�ution de la m�hode _free pour toutes les classes parentes
     puis pour la classe de l'objet */

  /*if (CLASS_is_native(class))
    EXEC_special_inheritance(SPEC_FREE, class, ob, 0, TRUE);*/

  OBJECT_detach(ob);
  remove_observers(ob);

  class->count--;

  #if DEBUG_REF
  ob->class = (CLASS *)0x23232323;
  #endif

  FREE(&ob, "OBJECT_free");
}


bool OBJECT_comp_value(VALUE *ob1, VALUE *ob2)
{
  if (ob1->type == T_NULL && ob2->type == T_NULL)
    return FALSE;
  else if (ob1->type == T_NULL)
    return ob2->_object.object != NULL;
  else if (ob2->type == T_NULL)
    return ob1->_object.object != NULL;
  else
    return COMPARE_object(&ob1->_object.object, &ob2->_object.object);
}


static void release(CLASS *class, OBJECT *ob)
{
  int i;
  CLASS_VAR *var;
  int nelt;
  char *data;

  if (class->parent != NULL && ob)
    release(class->parent, ob);

  if (CLASS_is_native(class))
    return;
	
  if (ob == NULL)
  {
    var = class->load->stat;
    nelt = class->load->n_stat;
    data = class->stat;
  }
  else
  {
		if (CLASS_is_struct(class) && ((CSTRUCT *)ob)->ref)
			return;

    var = class->load->dyn;
    nelt = class->load->n_dyn;
    data = (char *)ob;
  }

  for (i = 0; i < nelt; i++)
  {
#if TRACE_MEMORY
    if (var->type.id == T_STRING || var->type.id == T_OBJECT)
      fprintf(stderr, "release: %s %p [%d] trying %p\n", class->name, ob, i, (*(void **)&data[var->pos]));
#endif

		if (var->type.id == T_STRING)
      STRING_unref((char **)&data[var->pos]);
    else if (var->type.id == T_OBJECT)
    {
      OBJECT_UNREF(*((void **)&data[var->pos]), "release");
    }
    else if (var->type.id == T_VARIANT)
      VARIANT_free((VARIANT *)&data[var->pos]);

    var++;
  }

}


void OBJECT_release(CLASS *class, OBJECT *ob)
{
#if TRACE_MEMORY
  printf("> OBJECT_release %s %p\n", class->name, ob);
#endif

  release(class, ob);

  if (ob)
    OBJECT_free(class, ob);

#if TRACE_MEMORY
  printf("< OBJECT_release %s %p\n", class->name, ob);
#endif
}


void OBJECT_exit(void)
{
	#if DEBUG_LOAD
	fprintf(stderr, "------------ OBJECT_exit - BEGIN---------\n");
	#endif
  while (EventObject)
    OBJECT_detach(EventObject);
	#if DEBUG_LOAD
	fprintf(stderr, "------------ OBJECT_exit - END ----------\n");
	#endif
}



void OBJECT_create(void **object, CLASS *class, const char *name, void *parent, int nparam)
{
  void *ob;

  if (class->no_create)
    THROW(E_CSTATIC, class->name);

  TRY
  {
    OBJECT_new(&ob, class, name, parent);
    *object = ob;
		OBJECT_lock(ob, TRUE);
    EXEC_special_inheritance(SPEC_NEW, class, ob, nparam, TRUE);
		OBJECT_lock(ob, FALSE);
    OBJECT_UNREF_KEEP(ob, "OBJECT_create");
  }
  CATCH
  {
    *object = NULL;
    OBJECT_UNREF_KEEP(ob, "OBJECT_create");
    PROPAGATE();
  }
  END_TRY
}


/* FIXME: The _new are methods called differently from EXEC_special_inheritance */

void OBJECT_create_native(void **object, CLASS *class, VALUE *param)
{
  CLASS_DESC *desc;
  short index;

  OBJECT_new(object, class, NULL, NULL);

  for(;;)
  {
    index = class->special[SPEC_NEW];
    if (index != NO_SYMBOL)
    {
      desc = CLASS_get_desc(class, index);
      EXEC_call_native(desc->method.exec, *object, desc->method.type, param);
    }
    class = class->parent;
    if (!class)
      break;
  }

  OBJECT_UNREF_KEEP(*object, "OBJECT_create");
}

void OBJECT_lock(OBJECT *object, bool block)
{
  CLASS *class;

  if (!object)
    return;

  class = object->class;
	
	if (class->is_observer)
	{
		COBSERVER_lock((COBSERVER *)object, block);
		return;
	}
	
  if (class->n_event == 0)
    return;

	// fprintf(stderr, "OBJECT_lock: (%s %p) %s\n", class->name, object, block ? "lock" : "unlock");

  if (block)
    OBJECT_event(object)->parent = (OBJECT *)((intptr_t)OBJECT_event(object)->parent | 1);
  else
    OBJECT_event(object)->parent = (OBJECT *)((intptr_t)OBJECT_event(object)->parent & ~1);
}


bool OBJECT_is_locked(OBJECT *object)
{
  CLASS *class;

  if (!object)
    return FALSE;

  class = object->class;
	
	if (class->is_observer)
		return COBSERVER_is_locked((COBSERVER *)object);
	
  if (class->n_event == 0)
    return FALSE;

	return (((intptr_t)OBJECT_event(object)->parent & 1) != 0);
}


OBJECT *OBJECT_parent(void *object)
{
	//if (!OBJECT_has_events(object))
	//	return NULL;
	
  return ((OBJECT *)((intptr_t)OBJECT_event(object)->parent & ~1));
}


OBJECT *OBJECT_active_parent(void *object)
{
	OBJECT *parent = OBJECT_parent(object);
	
	if (!parent || OBJECT_is_locked((OBJECT *)object) || OBJECT_is_locked(parent))
		return NULL;

	return parent;
}
