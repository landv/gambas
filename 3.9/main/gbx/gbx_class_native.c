/***************************************************************************

	gbx_class_native.c

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

	GB_ReturnSelf(_object);

END_PROPERTY

CLASS *CLASS_register_class(GB_DESC *ptr, CLASS *class)
{
	int i, n_desc;

	CLASS_DESC *desc;
	CLASS_DESC *start;
	CLASS_DESC_GAMBAS *gambas = (CLASS_DESC_GAMBAS *)ptr;

	CLASS_EVENT *event;
	const char *ptype;
	//const char *type;
	int first_event, nsign;
	TYPE *sign;
	int first;
	int size_dynamic;
	VALUE value;

	#if DEBUG_LOAD
		fprintf(stderr, "Registering native class %s (%p)...\n", class->name, class);
	#endif

	if (gambas->type != GB_VERSION)
		return NULL;
		
	if (class->error)
		THROW_CLASS(class, "Loading has already failed", "");

	class->error = TRUE;
	
	class->is_native = TRUE;
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

	#ifdef OS_64BITS
	size_dynamic = (gambas->val1 + 7) & ~7;
	#else
	size_dynamic = (gambas->val1 + 3) & ~3;
	#endif

	class->n_desc = 0;
	class->n_event = 0;
	nsign = 0;

	/* Read the class global information at the beginning of the description */

	desc = (CLASS_DESC *)&gambas[1];

	for (start = NULL; start == NULL; desc++)
	{
		if (desc->gambas.name == NULL)
		{
			start = desc;
			break;
		}

		switch ((intptr_t)desc->gambas.name)
		{
			case (intptr_t)GB_INHERITS_ID:
				CLASS_inheritance(class, CLASS_find((const char *)desc->gambas.type), FALSE);
				break;

			case (intptr_t)GB_AUTO_CREATABLE_ID:
				class->auto_create = TRUE;
				break;

			case (intptr_t)GB_NOT_CREATABLE_ID:
				class->no_create = TRUE;
				break;

			case (intptr_t)GB_VIRTUAL_CLASS_ID:
				class->no_create = TRUE;
				class->is_virtual = TRUE;
				break;

			case (intptr_t)GB_HOOK_CHECK_ID:
				class->check = (int (*)())(desc->hook.func);
				class->must_check = TRUE;
				break;
				
			default:
				start = desc;
				break;
		}
	}

	/* If there is a parent class, and if the size is zero, then inherits the size */
	
	if (class->parent && size_dynamic == 0)
		size_dynamic = class->parent->size;

	/* Compute the number of symbol description */

	for(desc = start, n_desc = 0; desc->gambas.name != NULL; desc++, n_desc++);

	/* Description analysis */

	for (i = 0; i < n_desc; i++)
	{
		desc = &start[i];

		ptype = (char *)desc->gambas.type;
		desc->gambas.type = TYPE_from_string(&ptype);
		
		switch (CLASS_DESC_get_type(desc))
		{
			case CD_CONSTANT:

				if (desc->constant.type == T_STRING)
					desc->constant.type = T_CSTRING;
				else if (desc->constant.type == T_FLOAT || desc->constant.type == T_SINGLE)
				{
					if (desc->gambas.val1 == 0)
					{
						value._float.value = desc->gambas.val3._double;
					}
					else
					{
						if (NUMBER_from_string(NB_READ_FLOAT, desc->constant.value._string, strlen(desc->constant.value._string), &value))
							THROW_CLASS(class, "Bad constant", "");
					}
					
					if (desc->constant.type == T_SINGLE)
						desc->constant.value._single = (float)value._float.value;
					else
						desc->constant.value._float = value._float.value;
				}
				break;

			case CD_PROPERTY:
			case CD_STATIC_PROPERTY:
			case CD_PROPERTY_READ:
			case CD_STATIC_PROPERTY_READ:

				if ((intptr_t)desc->property.read == CLASS_DESC_SELF)
					desc->property.read = (void (*)())class_self_property;

				desc->property.write = desc->property.read;

				desc->property.native = TRUE;
				//desc->property.help = (char *)type;
				break;

			case CD_METHOD:
			case CD_STATIC_METHOD:

				//desc->method.help = (char *)desc->method.signature;
				desc->method.native = TRUE;
				desc->method.subr = desc->gambas.name[1] == '!';

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

	// Inheritance
	
	CLASS_make_description(class, start, n_desc, &first);

	CLASS_make_event(class, &first_event);

	// Transfer events and signatures

	if (nsign)
	{
		ALLOC(&class->signature, sizeof(TYPE) * nsign);
		sign = class->signature;
		
		for (i = first; i < class->n_desc; i++)
		{
			desc = class->table[i].desc;

			//fprintf(stderr, "[%.*s]\n", class->table[i].len, class->table[i].name);
			
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

					break;
			}
		}
	}
	
	if (class->n_event && (!class->parent || class->n_event > class->parent->n_event))
	{
		for (i = first; i < class->n_desc; i++)
		{
			desc = class->table[i].desc;

			switch (CLASS_DESC_get_type(desc))
			{
				case CD_EVENT:

					event = &class->event[first_event];
					event->name = class->table[i].name;
					if (desc->event.index)
						*((int *)desc->event.index) = first_event;
					desc->event.index = first_event;

					event->type = desc->event.type;
					event->param = (CLASS_PARAM *)desc->event.signature;
					event->n_param = desc->event.npmax;

					first_event++;

					break;
			}
		}
	}

	/* Sort the class description */

	CLASS_sort(class);

	/* Search for special methods */

	CLASS_search_special(class);

	/* Class is loaded */

	class->loaded = TRUE;
	class->error = FALSE;
	
	/* Run the static initializer */

	EXEC_public(class, NULL, "_init", 0);

	/* Class is ready */

	class->ready = TRUE;

	//total += MEMORY_size - alloc;
	//printf("%s: %d  TOTAL = %d\n", class->name, MEMORY_size - alloc, total);

	return class;
}


CLASS *CLASS_register(GB_DESC *ptr)
{
	const char *name = ((CLASS_DESC_GAMBAS *)ptr)->name;
	return CLASS_register_class(ptr, CLASS_check_global(CLASS_find_global(name)));
}

