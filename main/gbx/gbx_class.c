/***************************************************************************

  gbx_class.c

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

#define __GBX_CLASS_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
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
#include "gbx_struct.h"
#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_variant.h"
#include "gbx_number.h"
#include "gbx_c_array.h"

#include "gambas.h"

#include "gbx_class.h"

//#define DEBUG_COMP 1
//#define DEBUG_LOAD 1
//#define DEBUG_DESC 1

/* Global class table */
static TABLE _global_table;
/* List of all classes */
static CLASS *_classes = NULL;
/* First created class (must be 'Class' class) */
static CLASS *_first = NULL;
/* This flag forces CLASS_find() to only register in the global table */
static bool _global = FALSE;

#ifdef DEBUG
static CLASS *Class;
#endif


void CLASS_init(void)
{
	//if (sizeof(CLASS) > 128)
	//  fprintf(stderr, "Warning: Use the 64 bits version of Gambas at your own risk!\n");

	TABLE_create_static(&_global_table, sizeof(CLASS_SYMBOL), TF_IGNORE_CASE);

	CLASS_init_native();
}


TABLE *CLASS_get_table(void)
{
	return &_global_table;
}


static void exit_class(CLASS *class, bool native)
{
	//CLASS_DESC *desc;

	/*if (!class->ready && !CLASS_is_virtual(class))
		printf("WARNING: class %s was never loaded.\n", class->name);*/

	#if DEBUG_LOAD
		fprintf(stderr, "Exiting class %s...\n", class->name);
	#endif

	/* Est-ce que _exit marche pour les classes gambas ? */

	if (CLASS_is_native(class) != native)
		return;
	
	if (class->state < CS_LOADED)
		return;

	EXEC_public(class, NULL, "_exit", 0);
}

static void unload_class(CLASS *class)
{
	#if DEBUG_LOAD
		fprintf(stderr, "Unloading class %s...\n", class->name);
	#endif

	if (class->free_name)
		FREE(&class->name, "unload_class");

	if (class->is_struct)
	{
		FREE(&class->load->dyn, "unload_class");
		if (class->debug)
			FREE(&class->load->global, "unload_class");
		FREE(&class->load, "unload_class");
		FREE(&class->data, "unload_class");
	}
	else if (!CLASS_is_native(class))
	{
		#ifdef OS_64BITS
			
			if (class->load)
			{
				FREE(&class->load->desc, "unload_class");
				FREE(&class->load->cst, "unload_class");
				FREE(&class->load->class_ref, "unload_class");
				FREE(&class->load->unknown, "unload_class");
				FREE(&class->load->event, "unload_class");
				FREE(&class->load->ext, "unload_class");
				FREE(&class->load->local, "unload_class");
				FREE(&class->load->array, "unload_class");
				
				if (class->debug)
				{
					int i;
					FUNCTION *func;
					
					for (i = 0; i < class->load->n_func; i++)
					{
						func = &class->load->func[i];
						FREE(&func->debug->local, "unload_class");
					}
					
					FREE(&class->load->global, "unload_class");
					FREE(&class->load->debug, "unload_class");
				}
				
				FREE(&class->load->func, "unload_class");
			}

		#endif
		
		FREE(&class->load, "unload_class");
		if (!class->mmapped)
			FREE(&class->data, "unload_class");
	}
	else
	{
		FREE(&class->signature, "unload_class");
		FREE(&class->data, "unload_class");
	}

	if (class->free_event)
		FREE(&class->event, "unload_class");

	FREE(&class->stat, "unload_class");

	FREE(&class->table, "unload_class");
	FREE(&class->sort, "unload_class");

	class->state = CS_NULL;
}

static CLASS *class_replace_global(const char *name)
{
	CLASS_SYMBOL *csym;
	char *new_name;
	CLASS *class, *parent;
	CLASS *new_class;
	int len;
	int index;

	class = CLASS_find_global(name);
	if (class->state)
	{
		len = strlen(name);

		ALLOC(&new_name, len + 2, "class_replace_global");
		snprintf(new_name, len + 2, ">%s", name);
		new_class = class_replace_global(new_name);
		FREE(&new_name, "class_replace_global");

		new_name = (char *)new_class->name;

		TABLE_find_symbol(&_global_table, name, len, &index);
		csym = (CLASS_SYMBOL *)TABLE_get_symbol(&_global_table, index);
		csym->class = new_class;

		TABLE_find_symbol(&_global_table, new_name, len + 1, &index);
		csym = (CLASS_SYMBOL *)TABLE_get_symbol(&_global_table, index);
		csym->class = class;

		new_class->name = class->name;
		class->name = new_name;
		
		for(;;)
		{
			class->override = new_class;
			parent = class->parent;
			if (!parent || parent->override != class)
				break;
			class = parent;
		}

		class = new_class;
	}

	return class;
}


void CLASS_clean_up(bool silent)
{
	int n, nc, nb;
	//CLASS_SYMBOL *csym;
	CLASS *class;

	#if DEBUG_LOAD
	fprintf(stderr, "\n------------------- CLASS_exit -------------------\n\n");
	#endif

	#if DEBUG_LOAD
	fprintf(stderr, "Freeing auto-creatable objects...\n");
	#endif

	// Count how many classes should be freed
	// And free automatic instances

	nc = 0;

	for (class = _classes; class; class = class->next)
	{
		if (class->instance)
			OBJECT_UNREF(class->instance, "CLASS_exit");
		if (!CLASS_is_native(class) && class->state)
		{
			/*printf("Must free: %s\n", class->name);*/
			nc++;
		}
	}

	#if DEBUG_LOAD
	fprintf(stderr, "Calling _exit on loaded classes...\n");
	#endif

	for (class = _classes; class; class = class->next)
		exit_class(class, FALSE);

	#if DEBUG_LOAD
	fprintf(stderr, "Freeing classes with no instance...\n");
	#endif

	// Free classes having no instance

	n = 0;
	while (n < nc)
	{
		nb = n;

		for (class = _classes; class; class = class->next)
		{
			/*if (!CLASS_is_native(class) && class->ready && !class->exit)
				printf("%s: %d ready = %d\n", class->name, class->count, class->ready);*/

			if (class->count == 0 && !CLASS_is_native(class) && class->state && !class->exit)
			{
				/*printf("Freeing %s\n", class->name);*/
				#if DEBUG_LOAD
				fprintf(stderr, "Freeing %s\n", class->name);
				#endif
				OBJECT_release(class, NULL);
				class->exit = TRUE;
				n++;
			}
		}

		if (n == nb) /* On n'a rien pu faire */
			break;
	}

	#if DEBUG_LOAD
	fprintf(stderr, "Freeing other classes...\n");
	#endif

	// Remaining objects are circular references
	// Everything is forced to be freed

	if (n < nc)
	{
		if (!silent)
			ERROR_warning("circular references detected");
		for (class = _classes; class; class = class->next)
		{
			if (!CLASS_is_native(class) && class->state && !class->exit)
			{
				if (!silent)
					fprintf(stderr, "%s (%d)\n", class->name, class->count);
				OBJECT_release(class, NULL);
				class->exit = TRUE;
				n++;
			}
		}
	}


	#if DEBUG_LOAD
	fprintf(stderr, "Calling _exit on native classes...\n");
	#endif

	for (class = _classes; class; class = class->next)
		exit_class(class, TRUE);
}


void CLASS_exit()
{
	CLASS *class, *next;
	
	#if DEBUG_LOAD
	fprintf(stderr, "Unloading classes...\n");
	#endif

	for (class = _classes; class; class = class->next)
		unload_class(class);

	#if DEBUG_LOAD
	fprintf(stderr, "Destroying classes...\n");
	#endif

	class = _classes;
	while (class)
	{
		next = class->next;
		FREE(&class, "CLASS_exit");
		class = next;
	}

	TABLE_delete_static(&_global_table);
}


CLASS *CLASS_look(const char *name, int len)
{
	CLASS_SYMBOL *csym;
	ARCHIVE *arch = NULL;
	int index;

	#if DEBUG_COMP
	fprintf(stderr, "CLASS_look: %s in %s\n", name, _global ? "global" : "local");
	#endif

	//if (CP && CP->component && CP->component->archive)
	if (!_global && !ARCHIVE_get_current(&arch))
	{
		if (TABLE_find_symbol(arch->classes, name, len, &index))
		{
			#if DEBUG_COMP
			fprintf(stderr, " -> in %s\n", arch->name ? arch->name : "main");
			#endif
			csym = (CLASS_SYMBOL *)TABLE_get_symbol(arch->classes, index);
			return csym->class;
		}
	}

	if (TABLE_find_symbol(&_global_table, name, len, &index))
	{
		#if DEBUG_COMP
		fprintf(stderr, " -> %d in global\n", index);
		#endif
		csym = (CLASS_SYMBOL *)TABLE_get_symbol(&_global_table, index);
		return csym->class;
	}
	else
	{
		#if DEBUG_COMP
		fprintf(stderr, " -> ?\n");
		#endif
		return NULL;
	}
}

CLASS *CLASS_find(const char *name)
{
	CLASS_SYMBOL *csym;
	CLASS *class;
	int index;
	int len;
	ARCHIVE *arch = NULL;
	bool global;

	if (name == NULL)
		name = COMMON_buffer;

	len = strlen(name);

#if DEBUG_LOAD
		fprintf(stderr, "CLASS_find: %s (%d)\n", name, _global);
#endif

	class = CLASS_look(name, len);
	if (class)
		return class;

	//if (CP && CP->component && CP->component->archive)
	if (!_global && !ARCHIVE_get_current(&arch))
	{
		global = FALSE;
		TABLE_add_symbol(arch->classes, name, len, &index);
		#if DEBUG_LOAD || DEBUG_COMP
			fprintf(stderr, "Not found -> creating new one in %s\n", arch->name ? arch->name : "main");
		#endif
		csym = (CLASS_SYMBOL *)TABLE_get_symbol(arch->classes, index);
	}
	else
	{
		global = TRUE;
		TABLE_add_symbol(&_global_table, name, len, &index);
		#if DEBUG_LOAD || DEBUG_COMP
			fprintf(stderr, "Not found -> creating new one in global\n");
		#endif
		csym = (CLASS_SYMBOL *)TABLE_get_symbol(&_global_table, index);
	}

	ALLOC_ZERO(&class, sizeof(CLASS), "CLASS_find");
	csym->class = class;
	//class->state = CS_NULL;
	//class->count = 0;
	class->ref = 1;
	
	class->next = _classes;
	_classes = class;

	ALLOC(&class->name, len + 1, "CLASS_find");
	strcpy((char *)class->name, name);

	csym->sym.name = class->name;

	class->free_name = TRUE;

	// The first class must be the Class class!
	if (_first == NULL)
		_first = class;
	class->class = _first;
	
	class->global = global;

	return class;
}

int CLASS_count(void)
{
	return TABLE_count(&_global_table);
}


CLASS *CLASS_get(const char *name)
{
	CLASS *class = CLASS_find(name);

	if (class->state == CS_NULL)
		CLASS_load(class);

	return class;
}


CLASS *CLASS_look_global(const char *name, int len)
{
	CLASS *class;

	_global = TRUE;
	class = CLASS_look(name, len);
	_global = FALSE;

	return class;
}


CLASS *CLASS_find_global(const char *name)
{
	CLASS *class;

	_global = TRUE;
	class = CLASS_find(name);
	_global = FALSE;

	return class;
}


bool CLASS_inherits(CLASS *class, CLASS *parent)
{
	for(;;)
	{
		class = class->parent;

		if (class == NULL)
			return FALSE;

		if (class == parent)
			return TRUE;
	}
}


int CLASS_find_symbol_with_prefix(CLASS *class, const char *name, const char *prefix)
{
	return SYMBOL_find(class->table, class->sort, class->n_desc, sizeof(CLASS_DESC_SYMBOL), TF_IGNORE_CASE, name, strlen(name), prefix);
}


CLASS_DESC_SYMBOL *CLASS_get_symbol(CLASS *class, const char *name)
{
	int index;

	index = CLASS_find_symbol(class, name);
	if (index == NO_SYMBOL)
		return NULL;
	else
		return &class->table[index];
}


CLASS_DESC *CLASS_get_symbol_desc(CLASS *class, const char *name)
{
	CLASS_DESC_SYMBOL *cds = CLASS_get_symbol(class, name);

	if (cds == NULL)
		return NULL;
	else
		return cds->desc;
}


short CLASS_get_symbol_index_kind(CLASS *class, const char *name, int kind, int kind2)
{
	CLASS_DESC *desc;
	int index;

	index = CLASS_find_symbol(class, name);
	if (index != NO_SYMBOL)
	{
		desc = CLASS_get_desc(class, index);
		if (desc)
			if ((CLASS_DESC_get_type(desc) == kind) || (CLASS_DESC_get_type(desc) == kind2))
				return (short)index;
	}

	return (short)NO_SYMBOL;
}


CLASS_DESC *CLASS_get_symbol_desc_kind(CLASS *class, const char *name, int kind, int kind2)
{
	short index;

	index = CLASS_get_symbol_index_kind(class, name, kind, kind2);
	if (index == NO_SYMBOL)
		return NULL;
	else
		return CLASS_get_desc(class, index);
}


CLASS_DESC_EVENT *CLASS_get_event_desc(CLASS *class, const char *name)
{
	int index;
	CLASS_DESC *cd;

	index = CLASS_find_symbol_with_prefix(class, name, ":");
	if (index == NO_SYMBOL)
		return NULL;

	cd = class->table[index].desc;

	if (CLASS_DESC_get_type(cd) != CD_EVENT)
		return NULL;

	return &cd->event;
}


char *CLASS_DESC_get_signature(CLASS_DESC *cd)
{
	char *res = NULL;
	TYPE *sign;
	int i, n;
	TYPE type;
	
	switch (CLASS_DESC_get_type(cd))
	{
		case CD_METHOD:
		case CD_STATIC_METHOD:
			
			sign = cd->method.signature;
			n = cd->method.npmax;
			break;

		case CD_EVENT:

			sign = cd->event.signature;
			n = cd->event.npmax;
			break;
			
		case CD_EXTERN:
			
			sign = cd->ext.signature;
			n = cd->ext.npmax;
			break;

		default:

			return NULL;
	}
	
	for (i = 0; i < n; i++)
	{
		type = sign[i];
		STRING_add(&res, TYPE_to_string(type), 0);
		if (TYPE_is_object(type))
			STRING_add(&res, ";", 1);
	}
	
	return res;
}

// The _free method can be called during a conversion, so we must save the EXEC structure

void CLASS_free(void *object)
{
	CLASS *class = OBJECT_class(object);
	EXEC_GLOBAL save = EXEC;

	//fprintf(stderr, "> CLASS_free: %s %p\n", class->name, object);
	
	((OBJECT *)object)->ref = 1; /* Prevents anybody from freeing the object ! */
	//fprintf(stderr, "CLASS_free: %s %p\n", class->name, object);
	EXEC_special_inheritance(SPEC_FREE, class, object, 0, TRUE);
	((OBJECT *)object)->ref = 0;
	
	EXEC = save;

	OBJECT_release(class, object);

	//fprintf(stderr, "< CLASS_free: %s %p\n", class->name, object);
}

#if DEBUG_REF
void CLASS_ref(void *object)
{
	char *name;
	
	((OBJECT *)object)->ref++;
	
	if (OBJECT_class(object) == FREE_MARK)
		name = "*ALREADY FREED*";
	else
		name = OBJECT_class(object)->name;
	
	#if DEBUG_MEMORY
	fprintf(stderr, "%s: %s: ref(%s <%d>) -> %ld\n", OBJECT_ref_where,
		DEBUG_get_current_position(),
		name, GET_ALLOC_ID(object), ((OBJECT *)object)->ref);
	#else
	fprintf(stderr, "%s: %s: ref(%s %p) -> %ld\n", OBJECT_ref_where,
		DEBUG_get_current_position(),
		name, object, ((OBJECT *)object)->ref);
	#endif
	fflush(stdout);
}

bool CLASS_unref(void *ob, bool can_free)
{
	char *name;

	OBJECT *object = (OBJECT *)ob;
	
	if (OBJECT_class(object) == FREE_MARK)
		name = "*ALREADY FREED*";
	else
		name = OBJECT_class(object)->name;

	#if DEBUG_MEMORY
	if (object->ref <= 0)
		fprintf(stderr, "*** <%d> REF = %ld !\n", GET_ALLOC_ID(object), object->ref);

	fprintf(stderr, "%s: %s: unref(%s <%d>) -> %ld\n", OBJECT_ref_where,
		DEBUG_get_current_position(),
		name, GET_ALLOC_ID(object), object->ref - 1);
	#else
	if (object->ref <= 0)
		fprintf(stderr, "*** %p REF = %d !\n", object, object->ref);

	fprintf(stderr, "%s, %s: unref(%s %p) -> %ld\n", OBJECT_ref_where,
		DEBUG_get_current_position(),
		name, object, object->ref - 1);
	#endif
	fflush(stdout);

	/*if (strcmp(OBJECT_class(object)->name, "Class") == 0)
		fprintf(stderr, "Class ?\n");*/

	if ((--(object->ref) <= 0) && can_free)
	{
		#if DEBUG_MEMORY
		fprintf(stderr, "FREE <%d> !\n", GET_ALLOC_ID(object));
		#else
		fprintf(stderr, "FREE %p !\n", object);
		#endif
		CLASS_free(object);
		return TRUE;
	}
	
	return FALSE;
}
#endif

void CLASS_do_nothing()
{
}

int CLASS_return_zero()
{
	return 0;
}


static int partition(CLASS_DESC_SYMBOL *cds, ushort *sym, const int start, const int end)
{
	int pos = start;
	short pivot = sym[start];
	int i;
	ushort val;
	int len;
	const char *s1, *s2;
	int result;

	for (i = start + 1; i <= end; i++)
	{
		len = cds[sym[i]].len;
		result = len - cds[pivot].len;

		if (result > 0)
			continue;
		
		if (result == 0)
		{
			s1 = cds[sym[i]].name;
			s2 = cds[pivot].name;

			while (len)
			{
				result = tolower(*s1++) - tolower(*s2++);
				if (result)
					break;
				len--;
			}
			
			if (result >= 0)
				continue;
		}

		pos++; // incrémente compteur cad la place finale du pivot
		//echanger(tableau, compteur, i); // élément positionné
		val = sym[pos];
		sym[pos] = sym[i];
		sym[i] = val;
	}

	//echanger(tableau, compteur, debut); // le pivot est placé
	val = sym[pos];
	sym[pos] = sym[start];
	sym[start] = val;
	
	return pos; // et sa position est retournée
}

static void my_qsort(CLASS_DESC_SYMBOL *cds, ushort *sym, const int start, const int end)
{
   if (start < end) // cas d'arrêt pour la récursivité
   {
      int pivot = partition(cds, sym, start, end); // division du tableau
      my_qsort(cds, sym, start, pivot - 1); // trie partie1
      my_qsort(cds, sym, pivot + 1, end); // trie partie2
   }
}

void CLASS_sort(CLASS *class)
{
	ushort *sym;
	ushort i;

	if (!class->n_desc)
		return;
	
	ALLOC(&sym, sizeof(ushort) * class->n_desc, "CLASS_sort");

	for (i = 0; i < class->n_desc; i++)
		sym[i] = i;

	//qsort(sym, class->n_desc, sizeof(ushort), (int (*)(const void *, const void *))sort_desc);
	my_qsort(class->table, sym, 0, class->n_desc - 1);

	class->sort = sym;

	#if DEBUG_DESC
	{
		SYMBOL *s;

		fprintf(stderr, "\nSORT %s\n", class->name);

		for (i = 0; i < class->n_desc; i++)
		{
			s = (SYMBOL *)&class->table[sym[i]];

			fprintf(stderr, "[%d] %.*s\n", i, (int)s->len, s->name);
		}
	}
	#endif
}

void CLASS_inheritance(CLASS *class, CLASS *parent)
{
	if (class->parent != NULL)
		THROW(E_CLASS, class->name, "Multiple inheritance", "");

	class->parent = parent;

	TRY
	{
		CLASS_load(class->parent);
	}
	CATCH
	{
		THROW(E_CLASS, class->name, "Cannot load parent class ", parent->name);
	}
	END_TRY

	if (!class->check)
	{
		class->check = class->parent->check;
		class->must_check = class->parent->must_check;
	}
	// CREATE STATIC is inherited, but not CREATE PRIVATE

	if (parent->auto_create)
		class->auto_create = TRUE;

	if (!class->array_type)
		class->array_type = parent->array_type;
	
	//fprintf(stderr, "CLASS_inheritance: %s %s\n", class->name, class->auto_create ? "AUTO CREATE" : "");
}



const char *CLASS_DESC_get_type_name(CLASS_DESC *desc)
{
	switch (desc->gambas.val4)
	{
		case CD_PROPERTY_ID: return desc->gambas.val2 < 0 ? "r" : "p";
		case CD_VARIABLE_ID: return "v";
		case CD_METHOD_ID: return "m";
		case CD_STATIC_PROPERTY_ID: return desc->gambas.val2 < 0 ? "R" : "P";
		case CD_STATIC_VARIABLE_ID: return "V";
		case CD_STATIC_METHOD_ID: return "M";
		case CD_CONSTANT_ID: return "C";
		case CD_EVENT_ID: return ":";
		case CD_EXTERN_ID: return "X";
		default: return "?";
	}
}

// A symbol can be overridden by a more powerful one:
// - A read-only property can be overridden by a read-write property.
// - A constant can be overridden by any property.

static bool check_override(char parent_type, char type)
{
	if (parent_type == type)
		return FALSE;
		
	if (parent_type == 'r' && type == 'p')
		return FALSE;
	if (parent_type == 'R' && type == 'P')
		return FALSE;
	if (parent_type == 'C' && (type == 'R' || type == 'P'))
		return FALSE;
	
	return TRUE;
}

void CLASS_make_description(CLASS *class, CLASS_DESC *desc, int n_desc, int *first)
{
	static const char *nonher[] = { "_new", "_free", "_init", "_exit", NULL };

	int ind;
	int i, j;
	const char *name;
	const char **pnonher;
	CLASS *parent;
	char type, parent_type;
	CLASS_DESC_SYMBOL *cds;
	
	#if DEBUG_DESC
	fprintf(stderr, "\n---- %s\n", class->name);
	#endif
	
	/* Number of descriptions */

	class->n_desc = n_desc;
	if (class->parent)
		class->n_desc += class->parent->n_desc;

	/* Fabrication de la table */

	if (class->n_desc)
		ALLOC(&class->table, sizeof(CLASS_DESC_SYMBOL) * class->n_desc, "CLASS_make_description");

	i = 0;

	if (class->parent && class->parent->n_desc)
	{
		for (j = 0; j < class->parent->n_desc; j++)
		{
			class->table[i] = class->parent->table[j];
			i++;
		}

		for (j = 0; j < n_desc; j++)
		{
			if (CLASS_is_native(class))
			{
				name = &(desc[j].gambas.name[1]);
				type = CLASS_DESC_get_type(&desc[j]);
			}
			else
			{
				name = desc[j].gambas.name;
				type = *CLASS_DESC_get_type_name(&desc[j]);
			}

			//fprintf(stderr, "%s.%s\n", class->name, name);

			// An inherited symbol has two or more entries in the table. Only the first one
			// will be used, and so it must point at the new description, not the inherited one.
			parent = class;
			while ((parent = parent->parent))
			{
				ind = CLASS_find_symbol(parent, name);
				if (ind != NO_SYMBOL)
				{
					cds = &parent->table[ind];
					
					// The parent class public symbols of non-native classes were replaced by the symbol kind returned by CLASS_DESC_get_type_name()
					
					if (cds->desc)
					{
						parent_type = CLASS_DESC_get_type(cds->desc);
						
						if (check_override(parent_type, type))
						{
							#if DEBUG_DESC
							fprintf(stderr, "type = '%c' parent_type = '%c'\n", type, parent_type);
							#endif
							THROW(E_OVERRIDE, parent->name, cds->name, class->name);
						}
					}
					
					cds = &class->table[ind];
					
					#if DEBUG_DESC
					fprintf(stderr, "%s: [%d] (%p %d) := (%p %d)\n", name, ind, cds->desc, cds->desc ? cds->desc->gambas.val1 : 0, &desc[j], desc[j].gambas.val1);
					#endif
					
					cds->desc = &desc[j];
					cds->name = ".";
					cds->len = 1;
					
					if (!desc[j].gambas.val1 && index(CD_CALL_SOMETHING_LIST, type) != NULL)
					{
						#if DEBUG_DESC
						fprintf(stderr, "'%s' gambas.val1: %d -> %d\n", desc[j].gambas.name, desc[j].gambas.val1, class->parent->table[ind].desc->gambas.val1);
						#endif
						desc[j].gambas.val1 = class->parent->table[ind].desc->gambas.val1;
					}
				}
			}
		}
		
		for (pnonher = nonher; *pnonher; pnonher++)
		{
			ind = CLASS_find_symbol(class->parent, *pnonher);
			if (ind != NO_SYMBOL)
			{
				cds = &class->table[ind];
				cds->desc = NULL;
				cds->name = ".";
				cds->len = 1;
			}
		}
	}

	*first = i;

	for (j = 0; j < n_desc; j++, i++)
	{
		class->table[i].desc = &desc[j];

		/* On saute le caractère de type de symbole */
		if (CLASS_is_native(class))
			class->table[i].name = &(desc[j].gambas.name[1]);
		else
			class->table[i].name = desc[j].gambas.name;

		class->table[i].len = strlen(class->table[i].name);
	}
	
	#if DEBUG_DESC
	{
		CLASS_DESC_SYMBOL *cds;
		
		for (i = 0; i < class->n_desc; i++)
		{
			cds = &class->table[i];
			fprintf(stderr, "%d: %.*s %p\n", i, cds->len, cds->name, cds->desc);
		}
	}
	#endif
}

/* 'all' means that size_dynamic is the entire size of the object, not just the size of the data */
/* And check if the class is a stream */

void CLASS_calc_info(CLASS *class, int n_event, int size_dynamic, bool all, int size_static)
{
	// If the class is native and static, then size_dynamic == 0. But if we want to inherit 
	// the static class, and make the inherited class dynamic, then class->off_event must 
	// start after the object header. So we fix size_dynamic accordingly.
	
	if (all && size_dynamic == 0)
		size_dynamic = sizeof(OBJECT);
	
	if (class->parent)
	{
		if (all)
			class->off_event = size_dynamic;
		else
			class->off_event = class->parent->off_event + size_dynamic;

		class->n_event = class->parent->n_event + n_event;
	}
	else
	{
		if (all)
			class->off_event = size_dynamic;
		else
			class->off_event = sizeof(OBJECT) + size_dynamic;

		class->n_event = n_event;
	}

	if (class->n_event)
		class->size = class->off_event + sizeof(OBJECT_EVENT) + class->n_event * sizeof(ushort);
	else
		class->size = class->off_event;

	class->size_stat = size_static;
	if (size_static)
		ALLOC_ZERO(&class->stat, class->size_stat, "CLASS_calc_info");
	else
		class->stat = NULL;
		
	class->is_stream = (class == CLASS_Stream) || (class->parent && class->parent->is_stream);
}


void CLASS_make_event(CLASS *class, int *first)
{
	int i, ev;

	*first = 0;

	if (class->n_event == 0)
		return;

	if (!CLASS_is_native(class))
	{
		if (class->parent == NULL)
		{
			class->event = class->load->event;
			class->free_event = FALSE;
			return;
		}
	}

	ALLOC_ZERO(&class->event, sizeof(CLASS_EVENT) * class->n_event, "CLASS_make_event");

	if (class->parent != NULL)
	{
		ev = class->parent->n_event;

		for (i = 0; i < ev; i++)
			class->event[i] = class->parent->event[i];
	}
	else
		ev = 0;

	class->free_event = TRUE;

	*first = ev;
}


int CLASS_get_inheritance(CLASS *class, CLASS **her)
{
	int nher = 0;

	while ((nher < MAX_INHERITANCE) && (class != NULL))
	{
		*her++ = class;
		nher++;
		class = class->parent;
	}

	return nher;
}

void *CLASS_auto_create(CLASS *class, int nparam)
{
	void *ob = class->instance;

	//fprintf(stderr, ">>> CLASS_auto_create: %s (%p)\n", class->name, ob);
	
	// We automatically release invalid automatic instances
	
	if (ob)
	{
		if (OBJECT_is_valid(ob))
		{
			RELEASE_MANY(SP, nparam);
			//fprintf(stderr, "<<< CLASS_auto_create: %s (%p): valid=1\n", class->name, ob);
			return ob;
		}

		OBJECT_UNREF(class->instance, "CLASS_auto_create");
		class->instance = NULL;
	}

	/*fprintf(stderr, "CLASS_auto_create: create %s\n", class->name);*/

	class->instance = OBJECT_create(class, NULL, NULL, nparam);
	ob = class->instance;
	OBJECT_REF(ob, "CLASS_auto_create");

	//fprintf(stderr, "<<< CLASS_auto_create: %s (%p) valid=%d\n", class->name, ob, OBJECT_is_valid(ob));
	
	return ob;
}

void CLASS_search_special(CLASS *class)
{
	class->special[SPEC_NEW] = CLASS_get_symbol_index_kind(class, "_new", CD_METHOD, 0);
	class->special[SPEC_FREE] = CLASS_get_symbol_index_kind(class, "_free", CD_METHOD, 0);
	class->special[SPEC_GET] = CLASS_get_symbol_index_kind(class, "_get", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_PUT] = CLASS_get_symbol_index_kind(class, "_put", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_FIRST] = CLASS_get_symbol_index_kind(class, "_first", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_NEXT] = CLASS_get_symbol_index_kind(class, "_next", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_CALL] = CLASS_get_symbol_index_kind(class, "_call", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_UNKNOWN] = CLASS_get_symbol_index_kind(class, "_unknown", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_COMPARE] = CLASS_get_symbol_index_kind(class, "_compare", CD_METHOD, 0);
	class->special[SPEC_ATTACH] = CLASS_get_symbol_index_kind(class, "_attach", CD_METHOD, 0);
	
	class->special[SPEC_CONVERT] = CLASS_get_symbol_index_kind(class, "_@_convert", CD_CONSTANT, 0);

	if (class->special[SPEC_NEXT] != NO_SYMBOL)
		class->enum_static = CLASS_DESC_get_type(CLASS_get_desc(class, class->special[SPEC_NEXT])) == CD_STATIC_METHOD;
}


CLASS *CLASS_check_global(char *name)
{
	CLASS *class, *parent;

	class = CLASS_find_global(name);
	if (class->state)
	{
		if (COMPONENT_current && class->component == COMPONENT_current)
			ERROR_panic("Class '%s' declared twice in the component '%s'.", class->name, class->component->name);

		parent = class;
		class = class_replace_global(name);

		CLASS_inheritance(class, parent);
	}

	return class;
}


CLASS_DESC_METHOD *CLASS_get_special_desc(CLASS *class, int spec)
{
	short index = class->special[spec];

	if (index == NO_SYMBOL)
		return NULL;
	else
		return &CLASS_get_desc(class, index)->method;
}


CLASS_DESC_SYMBOL *CLASS_get_next_sorted_symbol(CLASS *class, int *index)
{
	CLASS_DESC_SYMBOL *old = NULL;
	CLASS_DESC_SYMBOL *cur;
	
	for(;;)
	{
		if (*index >= class->n_desc)
			return NULL;

		cur = &class->table[class->sort[*index]];
		if (*index > 0)
			old = &class->table[class->sort[*index - 1]];
		
		(*index)++;
		
		if (!cur->desc)
			continue;
		
		if (old && !TABLE_compare_ignore_case(cur->name, cur->len, old->name, old->len))
			continue;
		
		return cur;
	}
}


void CLASS_create_array_class(CLASS *class)
{
	void *save = TYPE_joker;
	char *name_joker;
	GB_DESC *desc;
	CLASS *array_type = (CLASS *)class->array_type;
	
	//fprintf(stderr, "CLASS_create_array_class: create %s\n", class->name);

	name_joker = STRING_new(class->name, strlen(class->name) - 2);

	if (!array_type)
	{
		array_type = class->global ? CLASS_find_global(name_joker) : CLASS_find(name_joker);
		class->array_type = (TYPE)array_type;
	}
	
	TYPE_joker = array_type;
	array_type->array_class = class;

	ALLOC(&desc, sizeof(GB_DESC) * ARRAY_TEMPLATE_NDESC, "CLASS_create_array_class");
	memcpy(desc, NATIVE_TemplateArray, sizeof(GB_DESC) * ARRAY_TEMPLATE_NDESC);
	((CLASS_DESC_GAMBAS *)desc)->name = class->name;

	CLASS_register_class(desc, class);

	class->is_array = TRUE;
	class->quick_array = CQA_ARRAY;
	class->data = (char *)desc;
	
	STRING_free(&name_joker);
	TYPE_joker = save;
}


CLASS *CLASS_get_array_class(CLASS *class)
{
	if (!class->array_class)
	{
		char name[strlen(class->name) + 3];
		strcpy(name, class->name);
		strcat(name, "[]");
		class->array_class = class->global ? CLASS_find_global(name) : CLASS_find(name);
		CLASS_create_array_class(class->array_class);
	}
	
	return class->array_class;
}


void CLASS_create_array_of_struct_class(CLASS *class)
{
	void *save = TYPE_joker;
	char *name_joker;
	GB_DESC *desc;
	CLASS *array_type = (CLASS *)class->array_type;
	
	//fprintf(stderr, "CLASS_create_array_class: create %s\n", class->name);

	name_joker = STRING_new(&class->name[1], strlen(class->name) - 3);

	if (!array_type)
	{
		array_type = class->global ? CLASS_find_global(name_joker) : CLASS_find(name_joker);
		class->array_type = (TYPE)array_type;
	}
	
	TYPE_joker = array_type;
	array_type->astruct_class = class;

	ALLOC(&desc, sizeof(GB_DESC) * ARRAY_OF_STRUCT_TEMPLATE_NDESC, "CLASS_create_array_of_struct_class");
	memcpy(desc, NATIVE_TemplateArrayOfStruct, sizeof(GB_DESC) * ARRAY_OF_STRUCT_TEMPLATE_NDESC);
	((CLASS_DESC_GAMBAS *)desc)->name = class->name;

	CLASS_register_class(desc, class);

	class->is_array = TRUE;
	class->is_array_of_struct = TRUE;
	class->data = (char *)desc;
	
	STRING_free(&name_joker);
	TYPE_joker = save;
}


CLASS *CLASS_get_array_of_struct_class(CLASS *class)
{
	if (!class->astruct_class)
	{
		char name[strlen(class->name) + 4];
		sprintf(name, "$%s[]", class->name);
		class->astruct_class = class->global ? CLASS_find_global(name) : CLASS_find(name);
		CLASS_create_array_of_struct_class(class->astruct_class);
	}
	
	return class->astruct_class;
}

int CLASS_sizeof(CLASS *class)
{
	if (CLASS_is_struct(class))
		return class->size - sizeof(CSTRUCT);
	else
		return class->size - sizeof(OBJECT);
}
