/***************************************************************************

  class_native.c

  Native class loader

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

#define __GBX_CLASS_NATIVE_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_alloc.h"
#include "gb_error.h"
#include "gb_limit.h"

#include <ctype.h>

#include "gb_buffer.h"
#include "gb_file.h"
#include "gbx_type.h"
#include "gbx_exec.h"
#include "gbx_debug.h"
#include "gb_magic.h"
#include "gbx_stream.h"

#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_variant.h"
#include "gbx_number.h"

#include "gambas.h"
#include "gbx_api.h"

#include "gbx_class.h"

//extern long total;

BEGIN_PROPERTY(class_self_property)

  if (_object)
    GB_ReturnObject(_object);
  else
    GB_Return(T_CLASS, GAMBAS_ReturnType);

END_PROPERTY

PUBLIC CLASS *CLASS_register_class(GB_DESC *ptr, CLASS *class)
{
  int i, n_desc;

  CLASS_DESC *desc;
  CLASS_DESC *start;
  CLASS_DESC_GAMBAS *gambas = (CLASS_DESC_GAMBAS *)ptr;

  CLASS_EVENT *event;
  const char *ptype;
  const char *type;
  int first_event, nsign;
  TYPE *sign;
  long first;
  long size_dynamic;

  #if DEBUG_LOAD
    printf("Registering native class %s (%p)...\n", class->name, class);
  #endif

  /* Initialisation de la classe */
  /*
  init = (void (*)())gambas->kind;
  if (init != NULL) (*init)();
  */

  if (gambas->type != GB_VERSION)
    return NULL;

  class->load = NULL;
  class->data = NULL;
  class->component = COMPONENT_current;
  #if DEBUG_COMP
  if (class->component)
    fprintf(stderr, "class %s -> component %s\n", class->name, class->component->name);
  else
    fprintf(stderr, "class %s -> no component\n", class->name);
  #endif
  class->is_virtual = *class->name == '.';

  size_dynamic = gambas->val1;

  class->n_desc = 0;
  class->n_event = 0;
  nsign = 0;

  /* D�ompte du nombre de descriptions, transformations, et d�ompte des signatures */

  desc = (CLASS_DESC *)&gambas[1];

  for (start = NULL; start == NULL; desc++)
  {
    if (desc->gambas.name == NULL)
    {
      start = desc;
      break;
    }

    switch ((long)desc->gambas.name)
    {
      case (long)GB_INHERITS_ID:
        CLASS_inheritance(class, CLASS_find((const char *)desc->gambas.type));
        break;

      case (long)GB_AUTO_CREATABLE_ID:
        class->auto_create = TRUE;
        break;

      case (long)GB_NOT_CREATABLE_ID:
      case (long)GB_VIRTUAL_CLASS_ID:
        class->no_create = TRUE;
        break;

      /*case (long)GB_HOOK_NEW_ID:
        class->new = desc->hook.func;
        SET_IF_NULL(class->new, DO_ERROR);
        break;

      case (long)GB_HOOK_FREE_ID:
        class->free = desc->hook.func;
        SET_IF_NULL(class->new, DO_ERROR);
        break;*/

      case (long)GB_HOOK_CHECK_ID:
        class->check = (int (*)())(desc->hook.func);
        SET_IF_NULL(class->check, CLASS_return_zero);
        break;

      default:
        start = desc;
        break;
    }
  }

	/* If there is a parent class, and if the size is zero, then inherits the size */
	if (class->parent && size_dynamic == 0)
		size_dynamic = class->parent->size;

  /* Si la classe n'a pas de parent et si elle n'a pas d�ini certains hooks */

  //SET_IF_NULL(class->new, (void (*)())OBJECT_new);
  //SET_IF_NULL(class->free, (void (*)())OBJECT_free);
  SET_IF_NULL(class->check, (int (*)())CLASS_return_zero);

  /* On transforme les DO_ERROR en NULL */

  //if (class->new == DO_ERROR)
  //  class->new = NULL;

  //if (class->free == DO_ERROR)
  //  class->free = NULL;

  /* calcul du nombre de descriptions */

  for(desc = start, n_desc = 0; desc->gambas.name != NULL; desc++, n_desc++);

  CLASS_make_description(class, start, n_desc, &first);

  /* Analyse de la description :
     - Calcul du nombre de signatures
     - Calcul du nombre d'��ements
  */

  for (i = first; i < class->n_desc; i++)
  {
    desc = class->table[i].desc;

    ptype = (char *)desc->gambas.type;
    type = ptype;
    desc->gambas.type = TYPE_from_string(&ptype);

    switch (CLASS_DESC_get_type(desc))
    {
      case CD_CONSTANT:

        if (desc->constant.type == T_STRING)
          desc->constant.type = T_CSTRING;
        break;

      case CD_PROPERTY:
      case CD_STATIC_PROPERTY:
      case CD_PROPERTY_READ:
      case CD_STATIC_PROPERTY_READ:

        if ((long)desc->property.read == CLASS_DESC_SELF)
          desc->property.read = (void (*)())class_self_property;

        desc->property.write = desc->property.read;

        desc->property.native = TRUE;
        //desc->property.help = (char *)type;
        break;

      case CD_METHOD:
      case CD_STATIC_METHOD:

        //desc->method.help = (char *)desc->method.signature;
        desc->method.native = TRUE;

        if (desc->method.signature)
        {
          TYPE_signature_length((char *)desc->method.signature, &desc->method.npmin, &desc->method.npmax, &desc->method.npvar);
          nsign += desc->method.npmax;
        }

        break;

      case CD_EVENT:

        class->n_event++;

        //desc->event.help = (char *)desc->event.signature;

        if (desc->event.signature)
        {
          TYPE_signature_length((char *)desc->event.signature, &desc->event.npmin, &desc->event.npmax, &desc->method.npvar);
          desc->event.npmin = desc->event.npmax;
          nsign += desc->event.npmax;
        }

        break;
    }

    desc->method.class = class;
  }

  CLASS_calc_info(class, class->n_event, size_dynamic, TRUE, 0);

  CLASS_make_event(class, &first_event);

  /* Transfert des évènements et signatures */

  if (nsign)
  {
    ALLOC(&class->signature, sizeof(TYPE) * nsign, "CLASS_register");
    sign = class->signature;
  }

  for (i = first; i < class->n_desc; i++)
  {
    desc = class->table[i].desc;

    switch (CLASS_DESC_get_type(desc))
    {
      case CD_METHOD:
      case CD_STATIC_METHOD:

        if (desc->method.npmax)
        {
          desc->method.signature =
            TYPE_transform_signature(&sign, (char *)desc->method.signature, desc->method.npmax);

        }
        break;

      case CD_EVENT:

        if (desc->event.npmax)
        {
          desc->event.signature =
            TYPE_transform_signature(&sign, (char *)desc->event.signature, desc->event.npmax);
        }

        event = &class->event[first_event];
        event->name = class->table[i].name;
        if (desc->event.index)
          *desc->event.index = first_event;

        event->type = desc->event.type;
        event->param = (CLASS_PARAM *)desc->event.signature;
        event->n_param = desc->event.npmax;

        first_event++;

        break;
    }
  }

  /* Tri */

  CLASS_sort(class);

  /* Fonctions sp�iales */

  CLASS_search_special(class);

  /* Fonction d'initialisation statique */

  class->state = CS_LOADED;
  EXEC_public(class, NULL, "_init", 0);

  /* La classe est pr�e */

  class->state = CS_READY;

  //total += MEMORY_size - alloc;
  //printf("%s: %d  TOTAL = %d\n", class->name, MEMORY_size - alloc, total);

  return class;
}


PUBLIC CLASS *CLASS_register(GB_DESC *ptr)
{
  return CLASS_register_class(ptr, CLASS_check_global(((CLASS_DESC_GAMBAS *)ptr)->name));
}

