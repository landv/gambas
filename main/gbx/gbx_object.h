/***************************************************************************

  gbx_object.h

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

#ifndef __OBJECT_H
#define __OBJECT_H

#include "gbx_debug.h"
#include "gbx_value.h"
#include "gbx_class.h"

#ifndef __OBJECT_C
EXTERN void **OBJECT_set_pointer;
#endif

typedef
	struct {
		CLASS *class;
		intptr_t ref;
		}
	OBJECT;

typedef
	struct {
		OBJECT *parent;
		OBJECT *next;
		OBJECT *prev;
		void *observer;
		ushort event[0];
		}
	OBJECT_EVENT;

#define OBJECT_event(_object) ((OBJECT_EVENT *)((intptr_t *)_object + ((OBJECT *)(_object))->class->off_event / sizeof(intptr_t)))
#define OBJECT_is(_object, _class) (OBJECT_class(_object) == _class)
#define OBJECT_is_class(_object) OBJECT_is(_object, CLASS_Class)
#define OBJECT_class(_object) (((OBJECT *)_object)->class)
#define OBJECT_class_null(_object) ((_object) ? ((OBJECT *)_object)->class : NULL)
#define OBJECT_count(_object) (((OBJECT *)_object)->ref)

#define OBJECT_are_null(_o1, _o2) (((intptr_t)(_o1) | (intptr_t)(_o2)) == 0)
#define OBJECT_are_not_null(_o1, _o2) ((_o1) && (_o2))
	
void *OBJECT_new(CLASS *class, const char *name, OBJECT *parent);
void OBJECT_attach(OBJECT *ob, OBJECT *parent, const char *name);
void OBJECT_detach(OBJECT *ob);
void OBJECT_release(CLASS *class, OBJECT *ob);
void OBJECT_release_static(CLASS *class, CLASS_VAR *var, int nelt, char *data);
void OBJECT_free(CLASS *class, OBJECT *ob);
void OBJECT_lock(OBJECT *ob, bool block);
bool OBJECT_is_locked(OBJECT *ob);

//void *OBJECT_alloc(CLASS *class, size_t size);
bool OBJECT_comp_value(VALUE *ob1, VALUE *ob2);

void OBJECT_exit(void);
void *OBJECT_create(CLASS *class, const char *name, void *parent, int nparam);
void *OBJECT_create_native(CLASS *class, VALUE *param);

#define OBJECT_create_and_set(_ptr, _class, _name, _parent, _nparam) \
	((OBJECT_set_pointer = (_ptr)), \
	OBJECT_create((_class), (_name), (_parent), (_nparam)))

#define OBJECT_is_valid(_object) ((_object) && !(((OBJECT *)_object)->class->check && (*((OBJECT *)_object)->class->check)(_object)))
#define OBJECT_has_events(_object) (((OBJECT *)_object)->class->n_event != 0)

OBJECT *OBJECT_parent(void *object);
OBJECT *OBJECT_active_parent(void *object);

/*
static INLINE CLASS *OBJECT_class(void *object)
{
	if (object)
		return ((OBJECT *)object)->class;
	else
		return object;
}
*/

/*#define DEBUG_REF 1*/

#if DEBUG_REF

#if OS_64BITS
	#define FREE_MARK ((CLASS *)0x2323232323232323LL)
#else
	#define FREE_MARK ((CLASS *)0x23232323)
#endif

EXTERN const char *OBJECT_ref_where;

char *OBJECT_where_am_i(const char *file, int line, const char *func);

#define OBJECT_ref(_object) \
{ \
	if (_object) \
	{ \
		if (OBJECT_class(_object) == FREE_MARK) \
		{ \
			fprintf(stderr, "*ALREADY FREED* %p\n", (_object)); \
			fflush(NULL); \
			BREAKPOINT(); \
		} \
		CLASS_ref(_object); \
	} \
}


#define OBJECT_unref(_object) \
{ \
	if (_object) \
	{ \
		if (OBJECT_class(_object) == FREE_MARK) \
		{ \
			fprintf(stderr, "*ALREADY FREED* %p\n", (_object)); \
			fflush(NULL); \
			BREAKPOINT(); \
		} \
		if (CLASS_unref(_object, TRUE)) \
			_object = NULL; \
	} \
}

#define OBJECT_just_unref(_object) \
{ \
	if (OBJECT_class(_object) == FREE_MARK) \
	{ \
		fprintf(stderr, "*ALREADY FREED* %p\n", (_object)); \
		fflush(NULL); \
		BREAKPOINT(); \
	} \
	CLASS_unref(_object, TRUE); \
}


#define OBJECT_unref_keep(_object) \
{ \
	if (_object) \
	{ \
		if (OBJECT_class(_object) == FREE_MARK) \
		{ \
			fprintf(stderr, "*ALREADY FREED* %p\n", (_object)); \
			fflush(NULL); \
		} \
		CLASS_unref(_object, FALSE); \
	} \
}

#define OBJECT_REF(_ob) { OBJECT_ref_where = OBJECT_where_am_i(__FILE__, __LINE__, __func__); OBJECT_ref(_ob); }
#define OBJECT_UNREF(_ob) { OBJECT_ref_where = OBJECT_where_am_i(__FILE__, __LINE__, __func__); OBJECT_unref(_ob); }
#define OBJECT_UNREF_KEEP(_ob) { OBJECT_ref_where = OBJECT_where_am_i(__FILE__, __LINE__, __func__); OBJECT_unref_keep(_ob); }

#else /* DEBUG_REF */

#define OBJECT_ref(_object) \
{ \
	if (_object) \
		((OBJECT *)(_object))->ref++; \
}

#define OBJECT_unref(_object) \
{ \
	if (_object) \
	{ \
		if ((--((OBJECT *)(_object))->ref) <= 0) \
		{ \
			void *temp = (void *)(_object); \
			_object = NULL; \
			CLASS_free(temp); \
		} \
	} \
}

#define OBJECT_just_unref(_object) \
{ \
	if ((--((OBJECT *)(_object))->ref) == 0) \
	{ \
		void *temp = (void *)(_object); \
		_object = NULL; \
		CLASS_free(temp); \
	} \
}

#define OBJECT_unref_keep(_object) \
{ \
	if (_object) \
		--((OBJECT *)(_object))->ref; \
}


#define OBJECT_REF(_ob) OBJECT_ref(_ob)
#define OBJECT_UNREF(_ob) OBJECT_unref(_ob)
#define OBJECT_UNREF_KEEP(_ob) OBJECT_unref_keep(_ob)

#endif /* DEBUG_REF */

#define OBJECT_get_var_addr(_object, _desc) ((void *)((char *)(_object) + (_desc)->variable.offset))


static INLINE void OBJECT_null(VALUE *value, CLASS *class)
{
	value->_object.class = class;
	value->_object.object = NULL;
}


static INLINE void OBJECT_put(VALUE *value, void *object)
{
	value->_object.class = OBJECT_class(object);
	value->_object.object = object;
}

#endif
