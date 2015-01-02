/***************************************************************************

  gbx_class.c

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
#include "gbx_jit.h"

#include "gambas.h"

#include "gbx_class.h"

//#define DEBUG_COMP 1
//#define DEBUG_LOAD 1
//#define DEBUG_DESC 1

// We are exiting...
bool CLASS_exiting = FALSE;

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
	TABLE_create_static(&_global_table, sizeof(CLASS_SYMBOL), TF_IGNORE_CASE);

	CLASS_init_native();
}


TABLE *CLASS_get_table(void)
{
	return &_global_table;
}

CLASS *CLASS_get_list(void)
{
	return _classes;
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
	
	if (!CLASS_is_loaded(class))
		return;

	EXEC_public(class, NULL, "_exit", 0);
}

static void unload_class(CLASS *class)
{
	#if DEBUG_LOAD
		fprintf(stderr, "Unloading class %s...\n", class->name);
	#endif

	if (class->free_name)
		FREE(&class->name);

	if (class->is_struct)
	{
		FREE(&class->load->dyn);
		if (class->debug)
			FREE(&class->load->global);
		FREE(&class->load);
		FREE(&class->data);
	}
	else if (!CLASS_is_native(class))
	{
		#ifdef OS_64BITS
			
			if (class->load)
			{
				FREE(&class->load->desc);
				FREE(&class->load->cst);
				FREE(&class->load->class_ref);
				FREE(&class->load->unknown);
				FREE(&class->load->event);
				FREE(&class->load->ext);
				FREE(&class->load->local);
				FREE(&class->load->array);
				
				if (class->debug)
				{
					int i;
					FUNCTION *func;
					
					for (i = 0; i < class->load->n_func; i++)
					{
						func = &class->load->func[i];
						FREE(&func->debug->local);
					}
					
					FREE(&class->load->global);
					FREE(&class->load->debug);
				}
				
				FREE(&class->load->func);
			}

		#endif
		
		if (class->load)
			FREE(&class->load->prof);
		
		FREE(&class->jit_functions);
		
		FREE(&class->load);
		//if (!class->mmapped)
		FREE(&class->data);
	}
	else
	{
		FREE(&class->signature);
		FREE(&class->data);
	}

	if (class->free_event)
		FREE(&class->event);

	FREE(&class->stat);

	FREE(&class->table);
	FREE(&class->sort);

	class->ready = FALSE;
	class->loaded = FALSE;
}

#define SWAP_FIELD(_v, _s, _d, _f) (_v = _s->_f, _s->_f = _d->_f, _d->_f = _v)

static void class_replace_global(CLASS *class)
{
	CLASS_DESC_SYMBOL *cds;
	CLASS *parent;
	const char *name;
	char *old_name;
	CLASS *old_class;
	int len;
	//int index;
	CLASS swap;
	char *swap_name;
	bool swap_free_name;
	int nprefix;
	int i;

	if (CLASS_is_loaded(class))
	{
		//fprintf(stderr, "class_replace_global: %p %s\n", class, name);
		
		name = class->name;
		len = strlen(name);

		nprefix = 0;
		parent = class;
		do
		{
			nprefix++;
			parent = parent->parent;
		}
		while (parent);
		
		ALLOC(&old_name, len + nprefix + 1);
		for (i = 0; i < nprefix; i++)
			old_name[i] = '>';
		strcpy(&old_name[i], name);
		
		old_class = CLASS_find_global(old_name);
		//fprintf(stderr, "-> %p %s\n", old_class, old_name);
		
		FREE(&old_name);
		/*FREE(&old_class->name, "class_replace_global");
		old_class->free_name = FALSE;
		old_class->name = class->name;*/

		//new_name = (char *)new_class->name;

		/*if (TABLE_find_symbol(&_global_table, name, len, &index))
		{
			csym = (CLASS_SYMBOL *)TABLE_get_symbol(&_global_table, index);
			csym->class = new_class;
		}

		if (TABLE_find_symbol(&_global_table, new_name, len + 1, &index))
		{
			csym = (CLASS_SYMBOL *)TABLE_get_symbol(&_global_table, index);
			csym->class = class;
		}

		new_class->name = class->name;
		class->name = new_name;*/
		
		swap = *class;
		*class = *old_class;
		*old_class = swap;
		
		SWAP_FIELD(swap_name, class, old_class, name);
		SWAP_FIELD(swap_free_name, class, old_class, free_name);
		SWAP_FIELD(parent, class, old_class, next);
		SWAP_FIELD(i, class, old_class, count);
		SWAP_FIELD(i, class, old_class, ref);
		
		for (i = 0; i < old_class->n_desc; i++)
		{
			cds = &old_class->table[i];
			if (cds->desc && cds->desc->method.class == class)
				cds->desc->method.class = old_class;
		}
		
		CLASS_inheritance(class, old_class, FALSE);
		
		/*for(;;)
		{
			class->override = new_class;
			parent = class->parent;
			if (!parent || parent->override != class)
				break;
			class = parent;
		}

		class = new_class;*/
	}

	//return class;
}

static void release_class(CLASS *class)
{
	#if DEBUG_LOAD
	fprintf(stderr, "Freeing %s\n", class->name);
	#endif
	OBJECT_release(class, NULL);
	class->exit = TRUE;
}

void CLASS_clean_up(bool silent)
{
	int n, nc, nb;
	//CLASS_SYMBOL *csym;
	CLASS *class;

	#if DEBUG_LOAD
	fprintf(stderr, "\n------------------- CLASS_clean_up -------------------\n\n");
	#endif

	#if DEBUG_LOAD
	fprintf(stderr, "Freeing auto-creatable objects...\n");
	#endif

	CLASS_exiting = TRUE;

	// Free automatic instances

	for (class = _classes; class; class = class->next)
	{
		if (class->instance)
		{
			#if DEBUG_LOAD
			fprintf(stderr, "Freeing instance of %p %s\n", class, class->name);
			#endif
			OBJECT_UNREF(class->instance);
		}
	}

	// Count how many classes should be freed

	nc = 0;

	for (class = _classes; class; class = class->next)
	{
		if (!CLASS_is_native(class) && CLASS_is_loaded(class))
		{
			#if DEBUG_LOAD
			fprintf(stderr, "Must free: %p (%s) %s [%d]\n", class, class->component ? class->component->name : "-", class->name, class->count);
			#endif
			nc++;
		}
	}

	#if DEBUG_LOAD
	fprintf(stderr, "Must free %d classes...\n", nc);
	#endif

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
	for(;;)
	{
		nb = n;

		for (class = _classes; class; class = class->next)
		{
			/*if (!CLASS_is_native(class) && class->ready && !class->exit)
				printf("%s: %d ready = %d\n", class->name, class->count, class->ready);*/

			if (class->count == 0 && !CLASS_is_native(class) && CLASS_is_loaded(class) && !class->exit)
			{
				release_class(class);
				n++;
			}
		}

		if (n == nb) // nothing could be done
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
			ERROR_warning("circular references detected:");

		for(;;)
		{
			nb = n;

			for (class = _classes; class; class = class->next)
			{
				if (!CLASS_is_native(class) && CLASS_is_loaded(class) && !class->exit && (!class->array_class || class->array_class->exit)) // && !class->astruct_class)
				{
					if (!silent)
						fprintf(stderr, "gbx" GAMBAS_VERSION_STRING ": % 5d %s\n", class->count, class->name);
					release_class(class);
					n++;
				}
			}

			if (n == nb) // nothing could be done
				break;
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
		FREE(&class);
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
	fprintf(stderr, "CLASS_look: %s in %s", name, _global ? "global" : "local");
	#endif

	/*if (strncasecmp(name, "listbox", 7) == 0)
	{
		name = "MyListBox";
		len = 9;
	}*/
	
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

	ALLOC_ZERO(&class, sizeof(CLASS));
	csym->class = class;
	class->ref = 1;
	
	class->next = _classes;
	_classes = class;

	ALLOC(&class->name, len + 1);
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

	if (!CLASS_is_loaded(class))
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
		res = STRING_add(res, TYPE_to_string(type), 0);
		if (TYPE_is_object(type))
			res = STRING_add_char(res, ';');
	}
	
	return res;
}

// NOTE: The _free method can be called during a conversion, so we must save the EXEC structure

static void error_CLASS_free()
{
	void *object = (void *)ERROR_handler->arg1;
	EXEC = *((EXEC_GLOBAL *)ERROR_handler->arg2);
	((OBJECT *)object)->ref = 0;
	OBJECT_release(OBJECT_class(object), object);
}

void CLASS_free(void *object)
{
	CLASS *class = OBJECT_class(object);
	EXEC_GLOBAL save = EXEC;

	ON_ERROR_2(error_CLASS_free, object, &save)
	{
		((OBJECT *)object)->ref = 1; // Prevents anybody from freeing the object!
		
		EXEC_special_inheritance(SPEC_FREE, class, object, 0, TRUE);
		
		((OBJECT *)object)->ref = 0;
		EXEC = save;
		OBJECT_release(class, object);
	}
	END_ERROR
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

static CLASS *_sorted_class;

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
			
			if (result == 0)
			{
				if (*cds[pivot].name != '.')
					ERROR_panic("Symbol '%s' declared twice in class '%s'\n", cds[sym[i]].name, _sorted_class->name);
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
	
	_sorted_class = class;
	
	ALLOC(&sym, sizeof(ushort) * class->n_desc);

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

			fprintf(stderr, "[%d] %.*s (%d)\n", i, (int)s->len, s->name, sym[i]);
		}
		fputc('\n', stderr);
	}
	#endif
}

void CLASS_inheritance(CLASS *class, CLASS *parent, bool in_jit_compilation)
{
	if (class->parent != NULL)
		THROW_CLASS(class, "Multiple inheritance", "");

	class->parent = parent;
	parent->has_child = TRUE;

	TRY
	{
		if (!in_jit_compilation)
			CLASS_load(class->parent);
		else
			JIT.LoadClass(class->parent);
	}
	CATCH
	{
		THROW_CLASS(class, "Cannot load parent class: ", STRING_new_temp_zero(ERROR_current->info.msg));
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



const char *CLASS_DESC_get_type_name(const CLASS_DESC *desc)
{
	switch (desc->gambas.val3._int[1])
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
		default: 
			//fprintf(stderr, "CLASS_DESC_get_type_name: %s: %ld\n", desc->gambas.name, desc->gambas.val4);
			return "?";
	}
}

static bool check_signature(char type, const CLASS_DESC *desc, const CLASS_DESC *pdesc)
{
	TYPE *sd, *sp;
	int nsd, nsp;
	
	if (!TYPE_are_compatible(desc->property.type, pdesc->property.type))
		return TRUE;
	
	switch (type)
	{
		case CD_METHOD:
		case CD_STATIC_METHOD:
			
			sd = desc->method.signature;
			nsd = desc->method.npmax;
			sp = pdesc->method.signature;
			nsp = pdesc->method.npmax;
			break;

		case CD_EVENT:

			sd = desc->event.signature;
			nsd = desc->event.npmax;
			sp = pdesc->event.signature;
			nsp = pdesc->event.npmax;
			break;
			
		case CD_EXTERN:
			
			sd = desc->ext.signature;
			nsd = desc->ext.npmax;
			sp = pdesc->ext.signature;
			nsp = pdesc->ext.npmax;
			break;

		default:

			return FALSE;
	}

	return TYPE_compare_signature(sd, nsd, sp, nsp, TRUE);
}

void CLASS_make_description(CLASS *class, const CLASS_DESC *desc, int n_desc, int *first)
{
	static const char *nonher[] = { "_new", "_free", "_init", "_exit", NULL };

	int ind;
	int i, j;
	const char *name;
	const char **pnonher;
	CLASS *parent;
	char type, parent_type;
	CLASS_DESC_SYMBOL *cds;
	bool check;
	
	#if DEBUG_DESC
	fprintf(stderr, "\n---- %s\n", class->name);
	#endif
	
	// Compute number of public descriptions

	class->n_desc = n_desc;
	if (class->parent)
		class->n_desc += class->parent->n_desc;

	// Make the description symbol table

	if (class->n_desc)
		ALLOC(&class->table, sizeof(CLASS_DESC_SYMBOL) * class->n_desc);

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
				//ptype = (const char*)desc[j].gambas.type;
				//fprintf(stderr, "%s -> ", ptype);
				//desc[j].gambas.type = TYPE_from_string(&ptype);
				//fprintf(stderr, "%p\n", (void *)desc[j].gambas.type);
			}
			else
			{
				name = desc[j].gambas.name;
				type = *CLASS_DESC_get_type_name(&desc[j]);
			}

			//fprintf(stderr, "%s.%s\n", class->name, name);

			// An inherited symbol has two or more entries in the table. Only the first one
			// will be used, and so it must point at the new description, not the inherited one.
			check = FALSE;
			parent = class;
			while ((parent = parent->parent))
			{
				ind = CLASS_find_symbol(parent, name);
				if (ind == NO_SYMBOL)
					continue;

				cds = &parent->table[ind];
					
				// The parent class public symbols of non-native classes were replaced by the symbol kind returned by CLASS_DESC_get_type_name()
				// Only the first inheritance level is tested against signature compatibility
				
				if (cds->desc && !check)
				{
					parent_type = CLASS_DESC_get_type(cds->desc);
					
					if (parent_type != type)
					{
						#if DEBUG_DESC
						fprintf(stderr, "type = '%c' parent_type = '%c'\n", type, parent_type);
						#endif
						THROW(E_OVERRIDE, CLASS_get_name(parent), cds->name, CLASS_get_name(class));
					}
					
					if (!CLASS_is_native(class) && strcasecmp(name, "_new"))
					{
						//fprintf(stderr, "check_signature: %s\n", name);
						if (check_signature(type, &desc[j], cds->desc))
							THROW(E_OVERRIDE, CLASS_get_name(parent), cds->name, CLASS_get_name(class));
					}
					
					check = TRUE;
				}
				
				cds = &class->table[ind];
				
				#if DEBUG_DESC
				fprintf(stderr, "%s: [%d] (%p %ld) := (%p %ld)\n", name, ind, cds->desc, cds->desc ? cds->desc->gambas.val1 : 0, &desc[j], desc[j].gambas.val1);
				#endif
				
				cds->desc = (CLASS_DESC *)&desc[j];
				cds->name = ".";
				cds->len = 1;
				
				/*if (!desc[j].gambas.val1 && index(CD_CALL_SOMETHING_LIST, type) != NULL)
				{
					//#if DEBUG_DESC
					fprintf(stderr, "CLASS_make_description: '%s.%s' gambas.val1: %ld -> %ld\n", class->name, desc[j].gambas.name, desc[j].gambas.val1, class->parent->table[ind].desc->gambas.val1);
					//#endif
					((CLASS_DESC *)desc)[j].gambas.val1 = class->parent->table[ind].desc->gambas.val1;
				}*/
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
		class->table[i].desc = (CLASS_DESC *)&desc[j];
		name = desc[j].gambas.name;

		/* On saute le caractère de type de symbole */
		if (CLASS_is_native(class))
		{
			name++;
			if (*name == '!')
				name++;
		}

		class->table[i].name = (char *)name;
		class->table[i].len = strlen(name);
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
		ALLOC_ZERO(&class->stat, class->size_stat);
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

	ALLOC_ZERO(&class->event, sizeof(CLASS_EVENT) * class->n_event);

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

		OBJECT_UNREF(class->instance);
		class->instance = NULL;
	}

	/*fprintf(stderr, "CLASS_auto_create: create %s\n", class->name);*/

	OBJECT_create_and_set(&class->instance, class, NULL, NULL, nparam);
	//ob = class->instance;
	//OBJECT_REF(ob, "CLASS_auto_create");

	//fprintf(stderr, "<<< CLASS_auto_create: %s (%p) valid=%d\n", class->name, ob, OBJECT_is_valid(ob));
	
	return class->instance;
}

//#define SET_OPTIONAL_OPERATOR(_class, _op, _func) if (!CLASS_has_operator(_class, _op)) (_class)->operators[_op] = (EXEC_no_operator##_func)

void CLASS_search_special(CLASS *class)
{
	static int _operator_strength = 0;
	int sym;
	
	class->special[SPEC_NEW] = CLASS_get_symbol_index_kind(class, "_new", CD_METHOD, 0);
	class->special[SPEC_FREE] = CLASS_get_symbol_index_kind(class, "_free", CD_METHOD, 0);
	class->special[SPEC_GET] = CLASS_get_symbol_index_kind(class, "_get", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_PUT] = CLASS_get_symbol_index_kind(class, "_put", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_FIRST] = CLASS_get_symbol_index_kind(class, "_first", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_NEXT] = CLASS_get_symbol_index_kind(class, "_next", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_CALL] = CLASS_get_symbol_index_kind(class, "_call", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_UNKNOWN] = CLASS_get_symbol_index_kind(class, "_unknown", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_PROPERTY] = CLASS_get_symbol_index_kind(class, "_property", CD_METHOD, CD_STATIC_METHOD);
	class->special[SPEC_COMPARE] = CLASS_get_symbol_index_kind(class, "_compare", CD_METHOD, 0);
	class->special[SPEC_ATTACH] = CLASS_get_symbol_index_kind(class, "_attach", CD_METHOD, 0);
	
	sym = CLASS_get_symbol_index_kind(class, "_@_convert", CD_CONSTANT, 0);
	if (sym != NO_SYMBOL)
	{
		class->has_convert = TRUE;
		class->convert = CLASS_get_desc(class, sym)->constant.value._pointer;
	}
	
	sym = CLASS_get_symbol_index_kind(class, "_@_operator", CD_CONSTANT, 0);
	if (sym != NO_SYMBOL)
	{
		class->has_operators = TRUE;
		class->operators = CLASS_get_desc(class, sym)->constant.value._pointer;
		_operator_strength++;
		CLASS_set_operator_strength(class, _operator_strength);
	}
	
	if (class->special[SPEC_NEXT] != NO_SYMBOL)
		class->enum_static = CLASS_DESC_get_type(CLASS_get_desc(class, class->special[SPEC_NEXT])) == CD_STATIC_METHOD;
	if (class->special[SPEC_UNKNOWN] != NO_SYMBOL)
		class->unknown_static = CLASS_DESC_get_type(CLASS_get_desc(class, class->special[SPEC_UNKNOWN])) == CD_STATIC_METHOD;
	if (class->special[SPEC_PROPERTY] != NO_SYMBOL)
		class->property_static = CLASS_DESC_get_type(CLASS_get_desc(class, class->special[SPEC_PROPERTY])) == CD_STATIC_METHOD;
}


CLASS *CLASS_check_global(CLASS *class)
{
	if (CLASS_is_loaded(class))
	{
		if (COMPONENT_current && class->component == COMPONENT_current)
			ERROR_panic("Class '%s' declared twice in the component '%s'.", CLASS_get_name(class), class->component->name);

		/*if (class->has_child)
			THROW(E_CLASS, class->name, "Overriding an already inherited class is forbidden", "");*/
			
		class_replace_global(class);
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

	ALLOC(&desc, sizeof(GB_DESC) * ARRAY_TEMPLATE_NDESC);
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

	ALLOC(&desc, sizeof(GB_DESC) * ARRAY_OF_STRUCT_TEMPLATE_NDESC);
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

char *CLASS_get_name(CLASS *class)
{
	char *name = class->name;
	
	while (*name == '>')
		name++;
	
	return name;
}
