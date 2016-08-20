/***************************************************************************

  gbx_object.c

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

#define __OBJECT_C

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_list.h"
#include "gbx_class.h"
#include "gbx_event.h"
#include "gbx_exec.h"
#include "gbx_compare.h"
#include "gbx_c_observer.h"
#include "gbx_c_array.h"
#include "gbx_struct.h"
#include "gbx_object.h"

#if DEBUG_REF
const char *OBJECT_ref_where = 0;
#endif

void **OBJECT_set_pointer = NULL;

static OBJECT *_event_object_list = NULL;

#if DEBUG_REF
char *OBJECT_where_am_i(const char *file, int line, const char *func)
{
	static char buffer[256];

	snprintf(buffer, sizeof(buffer), "[%s] %s:%d", file, func, line);
	return buffer;
}
#endif

void *OBJECT_new(CLASS *class, const char *name, OBJECT *parent)
{
	OBJECT *object;

	ALLOC_ZERO(&object, class->size);

	object->class = class;
	#if DEBUG_REF
	object->ref = 0;
	OBJECT_REF(object);
	#else
	object->ref = 1;
	#endif

	class->count++;

	OBJECT_attach(object, parent, name);

	return object;
}


#if 0
static void dump_attach(char *title)
{
	void *ob;

	fprintf(stderr, ">>>> %s: ", title);
	for (ob = _event_object_list; ob; ob = OBJECT_event(ob)->next)
		fprintf(stderr, "%p -> ", ob);
	fprintf(stderr, "(nil)\n");

}
#endif

static void call_attach_special_method(CLASS *class, void *ob, void *parent, const char *name)
{
	STACK_check(2);

	SP->_object.class = OBJECT_class(parent);
	SP->_object.object = parent;
	PUSH();

	if (name)
	{
		SP->type = T_CSTRING;
		SP->_string.addr = (char *)name;
		SP->_string.start = 0;
		SP->_string.len = strlen(name);
	}
	else
	{
		SP->type = T_NULL;
	}
	SP++;

	EXEC_special(SPEC_ATTACH, class, ob, 2, TRUE);
}

static void insert_object(OBJECT *ob, OBJECT_EVENT *ev)
{
	ev->next = _event_object_list;
	ev->prev = NULL;

	if (_event_object_list)
		OBJECT_event(_event_object_list)->prev = ob;

	_event_object_list = ob;
}

static void remove_object(OBJECT *ob, OBJECT_EVENT *ev)
{
	if (ev->prev)
		OBJECT_event(ev->prev)->next = ev->next;

	if (ev->next)
		OBJECT_event(ev->next)->prev = ev->prev;

	if (ob == _event_object_list)
		_event_object_list = ev->next;

	ev->prev = NULL;
	ev->next = NULL;
}

void OBJECT_detach(OBJECT *ob)
{
	CLASS *class = OBJECT_class(ob);
	OBJECT *parent;
	OBJECT_EVENT *ev;

	if (!class->is_observer && class->n_event == 0)
		return;

	ev = (OBJECT_EVENT *)((char *)ob + class->off_event);

	//if (!ev->parent)
	//	return;

	// Do not free the observers there

	remove_object(ob, ev);

	//dump_attach("OBJECT_detach");

	/* Avoids an infinite recursion, if freeing the parent implies freeing the object */
	parent = ev->parent;

	if (parent)
	{
		ev->parent = NULL;

		if (class->special[SPEC_ATTACH] != NO_SYMBOL)
			call_attach_special_method(class, ob, parent, NULL);

		#if DEBUG_EVENT || DEBUG_REF
			fprintf(stderr, "OBJECT_detach : Detach (%s %p) from (%s %p)\n",
				ob->class->name, ob, parent->class->name, parent);
		#endif
		OBJECT_UNREF(parent);
	}
}

static void remove_observers(OBJECT *ob)
{
	CLASS *class = OBJECT_class(ob);
	OBJECT_EVENT *ev;
	COBSERVER *obs, *next;

	//fprintf(stderr, "Remove observers: %s %p\n", class->name, ob);

	if (!class->is_observer && class->n_event == 0)
		return;

	ev = (OBJECT_EVENT *)((char *)ob + class->off_event);
	obs = ev->observer;
	ev->observer = NULL;

	while (obs)
	{
		next = obs->list.next;
		#if DEBUG_EVENT
		fprintf(stderr, "Remove observer %p %d: %p: %p\n", obs, (int)obs->ob.ref, ob, obs->object);
		#endif
		OBJECT_UNREF(obs);
		obs = next;
	}

	//ev->observer = NULL;
}

void OBJECT_attach(OBJECT *ob, OBJECT *parent, const char *name)
{
	CLASS *class = OBJECT_class(ob);
	OBJECT_EVENT *ev;

	if (!name)
		return;

	if (!class->is_observer && class->n_event == 0)
		return;

	OBJECT_detach(ob);

	ev = (OBJECT_EVENT *)((char *)ob + class->off_event);
	ev->parent = parent;

	#if DEBUG_EVENT || DEBUG_REF
		fprintf(stderr, "OBJECT_attach : Attach (%s %p) to (%s %p) as %s\n",
			ob->class->name, ob, parent->class->name, parent, name);
	#endif

	OBJECT_REF(parent);

	EVENT_search(class, ev->event, name, parent);

	insert_object(ob, ev);

	if (class->special[SPEC_ATTACH] != NO_SYMBOL)
		call_attach_special_method(class, ob, parent, name);

	//dump_attach("OBJECT_attach");
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

void OBJECT_release_static(CLASS *class, CLASS_VAR *var, int nelt, char *data)
{
	static void *jump[17] = {
		&&__NEXT, &&__NEXT, &&__NEXT, &&__NEXT, &&__NEXT, &&__NEXT, &&__NEXT, &&__NEXT, &&__NEXT,
		&&__STRING, &&__NEXT, &&__NEXT, &&__VARIANT, &&__ARRAY, &&__STRUCT, &&__NEXT, &&__OBJECT
		};

	CTYPE type;

	while (nelt--)
	{
#if TRACE_MEMORY
		if (var->type.id == T_STRING || var->type.id == T_OBJECT)
			fprintf(stderr, "release_static: %s [%d] trying %p\n", class->name, i, (*(void **)&data[var->pos]));
#endif

		type = var->type;
		goto *jump[type.id];

	__STRING:
		STRING_unref((char **)&data[var->pos]);
		goto __NEXT;

	__OBJECT:
		OBJECT_UNREF(*((void **)&data[var->pos]));
		goto __NEXT;

	__VARIANT:
		VARIANT_free((VARIANT *)&data[var->pos]);
		goto __NEXT;

	__ARRAY:
		CARRAY_release_static(class, class->load->array[type.value], &data[var->pos]);
		goto __NEXT;

	__STRUCT:
		{
			CLASS *sclass = class->load->class_ref[type.value];
			OBJECT_release_static(sclass, sclass->load->dyn, sclass->load->n_dyn, &data[var->pos]);
		}

	__NEXT:
		var++;
	}
}

static void release(CLASS *class, OBJECT *ob)
{
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
		if (CLASS_is_struct(class))
		{
			if (((CSTRUCT *)ob)->ref)
			{
				CSTRUCT_release((CSTRUCT *)ob);
				return;
			}
			data = (char *)ob + sizeof(CSTRUCT);
		}
		else
			data = (char *)ob;

		var = class->load->dyn;
		nelt = class->load->n_dyn;
	}

	OBJECT_release_static(class, var, nelt, data);
}


void OBJECT_release(CLASS *class, OBJECT *ob)
{
#if TRACE_MEMORY
	printf("> OBJECT_release %s %p\n", class->name, ob);
#endif

	if (ob)
	{
		ob->ref = 1; // Prevents anybody from freeing the object!
		OBJECT_detach(ob);
		remove_observers(ob);
		ob->ref = 0;
	}

	release(class, ob);

	if (ob)
	{
		class->count--;

		#if DEBUG_REF
		ob->class = FREE_MARK;
		#endif

		IFREE(ob);
	}

#if TRACE_MEMORY
	printf("< OBJECT_release %s %p\n", class->name, ob);
#endif
}


void OBJECT_exit(void)
{
	#if DEBUG_LOAD
	fprintf(stderr, "------------ OBJECT_exit - BEGIN---------\n");
	#endif
	while (_event_object_list)
		OBJECT_detach(_event_object_list);
	#if DEBUG_LOAD
	fprintf(stderr, "------------ OBJECT_exit - END ----------\n");
	#endif
}

static void *_object;
static char *_object_name;

static void error_OBJECT_create(void)
{
	OBJECT_UNREF_KEEP(_object);
	EVENT_leave_name(_object_name);
}

void *OBJECT_create(CLASS *class, const char *name, void *parent, int nparam)
{
	void *object;
	void *save;
	char *save_name;

	// The "no create" flag only concerns users of NEW
	//if (class->no_create)
	//	THROW(E_CSTATIC, CLASS_get_name(class));

	save = _object;
	save_name = _object_name;

	ON_ERROR(error_OBJECT_create)
	{
		_object_name = EVENT_enter_name(name);
		_object = object = OBJECT_new(class, name, parent);
		if (OBJECT_set_pointer)
		{
			*OBJECT_set_pointer = object;
			OBJECT_ref(object);
			OBJECT_set_pointer = NULL;
		}

		OBJECT_lock(object, TRUE);
		EXEC_special_inheritance(SPEC_NEW, class, object, nparam, TRUE);
		OBJECT_lock(object, FALSE);

		EXEC_special(SPEC_READY, class, object, 0, TRUE);

		error_OBJECT_create();
	}
	END_ERROR

	_object = save;
	_object_name = save_name;

	return object;
}


/* FIXME: The _new methods are called differently from EXEC_special_inheritance */

void *OBJECT_create_native(CLASS *class, VALUE *param)
{
	CLASS_DESC *desc;
	short index;
	void *object;

	object = OBJECT_new(class, NULL, NULL);

	for(;;)
	{
		index = class->special[SPEC_NEW];
		if (index != NO_SYMBOL)
		{
			desc = CLASS_get_desc(class, index);
			EXEC_call_native(desc->method.exec, object, desc->method.type, param);
		}
		class = class->parent;
		if (!class)
			break;
	}

	EXEC_special(SPEC_READY, OBJECT_class(object), object, 0, TRUE);
	OBJECT_UNREF_KEEP(object);
	return object;
}

void OBJECT_lock(OBJECT *object, bool lock)
{
	CLASS *class;
	OBJECT_EVENT *ev;

	if (!object)
		return;

	class = object->class;

	if (class->is_observer)
	{
		COBSERVER_lock((COBSERVER *)object, lock);
		return;
	}

	if (class->n_event == 0)
		return;

	ev = (OBJECT_EVENT *)((char *)object + class->off_event);

	if (lock)
		ev->locked++;
	else
		ev->locked--;
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

	return OBJECT_event(object)->locked > 0;
}


OBJECT *OBJECT_parent(void *object)
{
	CLASS *class = OBJECT_class(object);

	if (!class->is_observer && class->n_event == 0)
		return NULL;

	//return ((OBJECT *)((intptr_t)OBJECT_event(object)->parent & ~1));
	return OBJECT_event(object)->parent;
}


OBJECT *OBJECT_active_parent(void *object)
{
	OBJECT *parent = OBJECT_parent(object);

	if (!parent || OBJECT_is_locked((OBJECT *)object) || OBJECT_is_locked(parent))
		return NULL;

	return parent;
}
