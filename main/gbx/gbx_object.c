/***************************************************************************

  Object.c

  Object management routines

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

#define __OBJECT_C

#include "gb_common.h"
#include "gbx_class.h"
#include "gb_alloc.h"
#include "gbx_event.h"
#include "gbx_exec.h"
#include "gbx_compare.h"

#include "gbx_object.h"


static OBJECT *EventObject = NULL;

PUBLIC void OBJECT_new(void **ptr, CLASS *class, const char *name, OBJECT *parent)
{
  OBJECT_alloc(ptr, class, class->size);

  /* L'objet d�arre avec un compteur de r��ence �1
     Ce compteur devra �re remis �z�o par un OBJECT_unref_keep()
     lorsque l'initialisation de l'objet sera termin�
  */

  /*OBJECT_REF(*ptr, "OBJECT_new");*/

  #if DEBUG_EVENT
  printf("OBJECT_new: %s %p %d\n", class->name, *ptr, class->off_event);
  #endif

  OBJECT_attach(*ptr, parent, name);
}


PUBLIC void OBJECT_alloc(void **ptr, CLASS *class, size_t size)
{
  OBJECT *object;

  if (size < sizeof(OBJECT))
    ERROR_panic("OBJECT_alloc: size < %d", sizeof(OBJECT));

  ALLOC_ZERO(&object, size, "OBJECT_alloc");

  object->class = class;
  object->ref = 1;
  /* Lorsque les initialisations sont termin�s, remettre
     le compteur de r��ences �z�o */

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

PUBLIC void OBJECT_detach(OBJECT *ob)
{
  CLASS *class = OBJECT_class(ob);
  OBJECT *parent;
  OBJECT_EVENT *ev;
  bool lock;

  if (class->n_event <= 0)
    return;

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

  /* Avoids an infinite recursion, if freeing the parent implies freeing the object */
  parent = OBJECT_parent(ob);

  if (parent)
  {
  	lock = OBJECT_is_locked(ob);
    ev->parent = NULL;
    OBJECT_lock(ob, lock);
    #if DEBUG_REF
      printf("OBJECT_detach : Detach (%s %p) from (%s %p)\n",
        ob->class->name, ob, parent->class->name, parent);
    #endif
    OBJECT_UNREF(&parent, "OBJECT_detach");
  }
}


PUBLIC void OBJECT_attach(OBJECT *ob, OBJECT *parent, const char *name)
{
  CLASS *class = OBJECT_class(ob);
  OBJECT_EVENT *ev;
  bool lock;

  if (class->n_event == 0)
    return;

  if (name == NULL)
    return;

	lock = OBJECT_is_locked(ob);

  OBJECT_detach(ob);

  ev = (OBJECT_EVENT *)((char *)ob + class->off_event);

  ev->parent = parent;

	OBJECT_lock(ob, lock);

  #if DEBUG_REF
    printf("OBJECT_attach : Attach (%s %p) to (%s %p) as %s\n",
      ob->class->name, ob, parent->class->name, parent, name);
  #endif
  OBJECT_REF(parent, "OBJECT_attach");

  EVENT_search(class, ev->event, name, parent);

  ev->next = EventObject;
  ev->prev = NULL;

  if (EventObject)
    OBJECT_event(EventObject)->prev = ob;

  EventObject = ob;

	//dump_attach("OBJECT_attach");
}


PUBLIC void OBJECT_free(CLASS *class, OBJECT *ob)
{
  /* Il faut emp�her EXEC_leave() de relib�er l'objet */
  /*((OBJECT *)free_ptr)->ref = 1;*/

  /*OBJECT_REF(ob, "OBJECT_free");*/

  /* Ex�ution de la m�hode _free pour toutes les classes parentes
     puis pour la classe de l'objet */

  /*if (CLASS_is_native(class))
    EXEC_special_inheritance(SPEC_FREE, class, ob, 0, TRUE);*/

  OBJECT_detach(ob);

  class->count--;

  #if DEBUG_REF
  ob->class = (CLASS *)0x23232323;
  #endif

  FREE(&ob, "OBJECT_free");
}


PUBLIC bool OBJECT_comp_value(VALUE *ob1, VALUE *ob2)
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
    var = class->load->dyn;
    nelt = class->load->n_dyn;
    data = (char *)ob;
  }

  for (i = 0; i < nelt; i++)
  {
#if TRACE_MEMORY
    if (var->type.id == T_STRING || var->type.id == T_OBJECT)
    {
      printf("release: %s %p [%d] trying %p\n", class->name, ob, i, (*(void **)&data[var->pos]));
      fflush(NULL);
    }
#endif

    if (var->type.id == T_STRING)
      STRING_unref((char **)&data[var->pos]);
    else if (var->type.id == T_OBJECT)
    {
      OBJECT_unref((void **)&data[var->pos]);
    }
    else if (var->type.id == T_VARIANT)
      VARIANT_free((VARIANT *)&data[var->pos]);
    else if (var->type.id == T_ARRAY)
      ARRAY_free_data((void *)&data[var->pos], (ARRAY_DESC *)class->load->array[var->type.value]);

    var++;
  }

}


PUBLIC void OBJECT_release(CLASS *class, OBJECT *ob)
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


PUBLIC void OBJECT_exit(void)
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



PUBLIC void OBJECT_create(void **object, CLASS *class, const char *name, void *parent, int nparam)
{
  void *ob;

  if (class->new == NULL)
    THROW(E_CSTATIC, class->name);

  TRY
  {
    (*class->new)(&ob, class, name, parent);
    *object = ob;
		OBJECT_lock(ob, TRUE);
    EXEC_special_inheritance(SPEC_NEW, class, ob, nparam, TRUE);
		OBJECT_lock(ob, FALSE);
    OBJECT_UNREF_KEEP(&ob, "OBJECT_create");
  }
  CATCH
  {
    *object = NULL;
    OBJECT_UNREF_KEEP(&ob, "OBJECT_create");
    PROPAGATE();
  }
  END_TRY
}


PUBLIC void OBJECT_create_native(void **object, CLASS *class, VALUE *param)
{
  CLASS_DESC *desc;
  short index;

  (*class->new)(object, class, NULL, NULL);

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

  OBJECT_UNREF_KEEP(object, "OBJECT_create");
}

PUBLIC void OBJECT_lock(OBJECT *object, bool block)
{
  CLASS *class;

  if (!object)
    return;

  class = object->class;
  if (class->n_event == 0)
    return;

	// fprintf(stderr, "OBJECT_lock: (%s %p) %s\n", class->name, object, block ? "lock" : "unlock");

  if (block)
    OBJECT_event(object)->parent = (OBJECT *)((long)OBJECT_event(object)->parent | 1);
  else
    OBJECT_event(object)->parent = (OBJECT *)((long)OBJECT_event(object)->parent & ~1);
}


PUBLIC bool OBJECT_is_locked(OBJECT *object)
{
  CLASS *class;

  if (!object)
    return FALSE;

  class = object->class;
  if (class->n_event == 0)
    return FALSE;

	return (((long)OBJECT_event(object)->parent & 1) != 0);
}


PUBLIC bool OBJECT_is_valid(void *object)
{
  if (object == NULL)
    return FALSE;

   return !(*((OBJECT *)object)->class->check)(object);
}

