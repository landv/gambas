/***************************************************************************

  Object.h

  Object management routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#ifndef __OBJECT_H
#define __OBJECT_H

#include "gbx_debug.h"
#include "gbx_value.h"
#include "gbx_class.h"

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

#define OBJECT_event(_object) ((OBJECT_EVENT *)((char *)_object + ((OBJECT *)(_object))->class->off_event))
#define OBJECT_is(_object, _class) (OBJECT_class(_object) == _class)
#define OBJECT_is_class(_object) OBJECT_is(_object, CLASS_Class)
#define OBJECT_class(_object) ((_object) ? ((OBJECT *)_object)->class : NULL)
#define OBJECT_count(_object) ((_object) ? ((OBJECT *)_object)->ref : 0)

void OBJECT_new(void **ptr, CLASS *class, const char *name, OBJECT *parent);
void OBJECT_attach(OBJECT *ob, OBJECT *parent, const char *name);
void OBJECT_detach(OBJECT *ob);
void OBJECT_release(CLASS *class, OBJECT *ob);
void OBJECT_free(CLASS *class, OBJECT *ob);
void OBJECT_lock(OBJECT *ob, bool block);
bool OBJECT_is_locked(OBJECT *ob);

void OBJECT_alloc(void **ptr, CLASS *class, size_t size);
bool OBJECT_comp_value(VALUE *ob1, VALUE *ob2);

void OBJECT_exit(void);
void OBJECT_create(void **object, CLASS *class, const char *name, void *parent, int nparam);
void OBJECT_create_native(void **object, CLASS *class, VALUE *param);

#define OBJECT_is_valid(_object) ((_object) && !(((OBJECT *)_object)->class->check && (*((OBJECT *)_object)->class->check)(_object)))
#define OBJECT_has_events(_object) ((_object) && (((OBJECT *)_object)->class->n_event != 0))

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

#define OBJECT_ref(_object) \
{ \
  if (_object) \
  { \
    if (OBJECT_class(_object) == (CLASS *)0x23232323) \
    { \
      fprintf(stderr, "Already freed! %p\n", (_object)); \
      fflush(NULL); \
    } \
    CLASS_ref(_object); \
  } \
}

#define OBJECT_unref(_pobject) \
{ \
  if (*(_pobject)) \
  { \
    if (OBJECT_class(*(_pobject)) == (CLASS *)0x23232323) \
    { \
      fprintf(stderr, "Already freed! %p\n", *(_pobject)); \
      fflush(NULL); \
    } \
    CLASS_unref((_pobject), TRUE); \
  } \
}

#define OBJECT_unref_keep(_pobject) \
{ \
  if (*(_pobject)) \
  { \
    if (OBJECT_class(*(_pobject)) == (CLASS *)0x23232323) \
    { \
      fprintf(stderr, "Already freed! %p\n", *(_pobject)); \
      fflush(NULL); \
    } \
    CLASS_unref((_pobject), FALSE); \
  } \
}

#define OBJECT_REF(_ob, _where) { printf("REF <" _where "> \n"); OBJECT_ref(_ob); }
#define OBJECT_UNREF(_ob, _where) { printf("UNREF <" _where "> \n"); OBJECT_unref(((void **)(void *)_ob)); }
#define OBJECT_UNREF_KEEP(_ob, _where) { printf("UNREF_KEEP <" _where "> \n"); OBJECT_unref_keep(((void **)(void *)_ob)); }

#else /* DEBUG_REF */

/*
#define OBJECT_ref(_object) \
{ \
  if (_object) \
    CLASS_ref((void *)(_object)); \
}

#define OBJECT_unref(_pobject) \
{ \
  if (*(_pobject)) \
    CLASS_unref((void **)(_pobject), TRUE); \
}

#define OBJECT_unref_keep(_pobject) \
{ \
  if (*(_pobject)) \
    CLASS_unref((void **)(_pobject), FALSE); \
}
*/


#define OBJECT_ref(_object) \
{ \
  if (_object) \
    ((OBJECT *)(void *)(_object))->ref++; \
}

#define OBJECT_unref(_pobject) \
{ \
  if (*(_pobject) && --(*((OBJECT **)(void *)(_pobject)))->ref <= 0) \
  { \
    CLASS_free((void **)(_pobject)); \
  } \
}

#define OBJECT_unref_keep(_pobject) \
{ \
  if (*(_pobject)) \
    --(*((OBJECT **)(void *)(_pobject)))->ref; \
}


#define OBJECT_REF(_ob, _where) OBJECT_ref(_ob)
#define OBJECT_UNREF(_ob, _where) OBJECT_unref(((void **)(void *)_ob))
#define OBJECT_UNREF_KEEP(_ob, _where) OBJECT_unref_keep(((void **)(void *)_ob))

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
