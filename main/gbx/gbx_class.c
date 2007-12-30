/***************************************************************************

  class.c

  Class loader

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

#define __GBX_CLASS_C

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
#include "gbx_c_array.h"

#include "gambas.h"

#include "gbx_class.h"

//#define DEBUG_COMP 1
//#define DEBUG_LOAD 1

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


PUBLIC void CLASS_init(void)
{
  if (sizeof(CLASS) > 128)
    fprintf(stderr, "sizeof(CLASS) = %d! Did you compile gambas on a 64 bits CPU?\n", sizeof(CLASS));

  TABLE_create_static(&_global_table, sizeof(CLASS_SYMBOL), TF_IGNORE_CASE);

  CLASS_init_native();
}


PUBLIC TABLE *CLASS_get_table(void)
{
  return &_global_table;
}


static void exit_class(CLASS *class)
{
  CLASS_DESC *desc;

  /*if (!class->ready && !CLASS_is_virtual(class))
    printf("WARNING: class %s was never loaded.\n", class->name);*/

  #if DEBUG_LOAD
    fprintf(stderr, "Exiting class %s...\n", class->name);
  #endif

  /* Est-ce que _exit marche pour les classes gambas ? */

  if (!CLASS_is_native(class))
    return;

  desc = CLASS_get_symbol_desc_kind(class, "_exit", CD_STATIC_METHOD, 0);
  if (!desc)
    return;

  EXEC.desc = &desc->method;
  EXEC.native = TRUE;
  EXEC.class = class;
  EXEC.object = NULL;
  EXEC.nparam = 0;
  EXEC.drop = TRUE;
  EXEC.use_stack = FALSE;

  EXEC_native();
}

static void unload_class(CLASS *class)
{
  #if DEBUG_LOAD
    fprintf(stderr, "Unloading class %s...\n", class->name);
  #endif

  if (class->free_name)
    FREE(&class->name, "unload_class");

  if (!CLASS_is_native(class))
  {
    /*OBJECT_release(class, NULL);*/
    FREE(&class->load, "unload_class");
    if (!class->mmapped)
      FREE(&class->data, "unload_class");
  }
  else
  {
    if (class->signature)
      FREE(&class->signature, "unload_class");
		if (class->data)
      FREE(&class->data, "unload_class");
  }

  if (class->free_event)
    FREE(&class->event, "unload_class");

  if (class->stat)
    FREE(&class->stat, "unload_class");

  if (class->table)
    FREE(&class->table, "unload_class");

  class->state = CS_NULL;
}


PUBLIC void CLASS_exit(void)
{
  int n, nc, nb;
  //CLASS_SYMBOL *csym;
  CLASS *class, *next;
  bool in_error;

  #if DEBUG_LOAD
  fprintf(stderr, "\n------------------- CLASS_exit (variables statiques) -------------------\n\n");
  #endif

	in_error = ERROR_info.code > 0;

  #if DEBUG_LOAD
  fprintf(stderr, "Freeing auto-creatable objects...\n");
  #endif

  /* On compte le nombre de classes �lib�er */
  /* On lib�e les instances automatiques */

  nc = 0;

  for (class = _classes; class; class = class->next)
  {
    if (class->instance)
      OBJECT_UNREF(&class->instance, "CLASS_exit");
    if (!CLASS_is_native(class) && class->state)
    {
      /*printf("Must free: %s\n", class->name);*/
      nc++;
    }
  }

  #if DEBUG_LOAD
  fprintf(stderr, "Freeing classes with no instance...\n");
  #endif

  /* On ne lib�e que les classes n'ayant aucun objet instanci�*/

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

  /* On force la lib�ation de ce qui reste, tant pis */
  /* ERROR_info.code != 0 if we are exiting just after an error */

  if (n < nc)
  {
    if (!in_error)
      fprintf(stderr, "WARNING: circular references detected\n");
    for (class = _classes; class; class = class->next)
    {
      if (!CLASS_is_native(class) && class->state && !class->exit)
      {
        if (!in_error)
          fprintf(stderr, "%s (%ld)\n", class->name, class->count);
        OBJECT_release(class, NULL);
        class->exit = TRUE;
        n++;
      }
    }
  }


  #if DEBUG_LOAD
  fprintf(stderr, "\n------------------- CLASS_exit (d�hargement) -------------------\n\n");
  #endif

  for (class = _classes; class; class = class->next)
    exit_class(class);

  for (class = _classes; class; class = class->next)
    unload_class(class);

  #if DEBUG_LOAD
  fprintf(stderr, "\n------------------- CLASS_exit (lib�ation) -------------------\n\n");
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




PUBLIC CLASS *CLASS_look(const char *name)
{
  CLASS_SYMBOL *csym;
  ARCHIVE *arch = NULL;
	long index;

  #if DEBUG_COMP
  fprintf(stderr, "CLASS_look: %s (%d)\n", name, _global);
  #endif

  //if (CP && CP->component && CP->component->archive)
  if (!_global && !ARCHIVE_get_current(&arch))
  {
    if (TABLE_find_symbol(arch->classes, name, strlen(name), (SYMBOL **)&csym, NULL))
    {
      #if DEBUG_COMP
      fprintf(stderr, " -> in %s\n", arch->name ? arch->name : "main");
      #endif
      return csym->class;
    }
  }

  if (TABLE_find_symbol(&_global_table, name, strlen(name), (SYMBOL **)&csym, &index))
  {
    #if DEBUG_COMP
    fprintf(stderr, " -> %ld in global\n", index);
    #endif
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


PUBLIC CLASS *CLASS_find(const char *name)
{
  CLASS_SYMBOL *csym;
  CLASS *class;
  long index;
  long len;
  ARCHIVE *arch = NULL;

  if (name == NULL)
    name = COMMON_buffer;

#if DEBUG_LOAD
    fprintf(stderr, "CLASS_find: %s (%d)\n", name, _global);
#endif

  class = CLASS_look(name);
  if (class)
    return class;

  //if (CP && CP->component && CP->component->archive)
  if (!_global && !ARCHIVE_get_current(&arch))
  {
    TABLE_add_symbol(arch->classes, name, strlen(name), (SYMBOL **)&csym, NULL);
    #if DEBUG_LOAD || DEBUG_COMP
      fprintf(stderr, "Not found -> creating new one in %s\n", arch->name ? arch->name : "main");
    #endif
  }
  else
  {
    TABLE_add_symbol(&_global_table, name, strlen(name), (SYMBOL **)&csym, &index);
    #if DEBUG_LOAD || DEBUG_COMP
      fprintf(stderr, "Not found -> creating new one in global\n");
    #endif
  }

  ALLOC_ZERO(&csym->class, Max(128, sizeof(CLASS)), "CLASS_find");
  /*csym->class->id = index;*/
  csym->class->state = CS_NULL;
  csym->class->count = 0;
  csym->class->ref = 1;
  csym->class->next = _classes;
  _classes = csym->class;

	len = strlen(name);
  ALLOC(&csym->class->name, len + 1, "CLASS_find");
  strcpy((char *)csym->class->name, name);

  csym->sym.name = csym->class->name;

  csym->class->free_name = TRUE;

  if (_first == NULL)
    _first = csym->class;

  csym->class->class = _first;

  #if DEBUG_LOAD
  fprintf(stderr, "New class %s !\n", name);
  #endif

  return csym->class;
}


PUBLIC long CLASS_count(void)
{
  return TABLE_count(&_global_table);
}


PUBLIC CLASS *CLASS_get(const char *name)
{
  CLASS *class = CLASS_find(name);

  if (class->state == CS_NULL)
    CLASS_load(class);

  return class;
}


PUBLIC CLASS *CLASS_look_global(const char *name)
{
  CLASS *class;

  _global = TRUE;
  class = CLASS_look(name);
  _global = FALSE;

  return class;
}


PUBLIC CLASS *CLASS_find_global(const char *name)
{
  CLASS *class;

  _global = TRUE;
  class = CLASS_find(name);
  _global = FALSE;

  return class;
}


PUBLIC boolean CLASS_inherits(CLASS *class, CLASS *parent)
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


PUBLIC long CLASS_find_symbol(CLASS *class, const char *name)
{
  long index;

  SYMBOL_find(class->table, class->n_desc, sizeof(CLASS_DESC_SYMBOL), TF_IGNORE_CASE, name, strlen(name), NULL, &index);

  //if (strcmp(name, "m1") == 0)
  //  printf("find: %s in %s -> %d\n", name, class->name, index);

  return index;
}


PUBLIC long CLASS_find_symbol_with_prefix(CLASS *class, const char *name, const char *prefix)
{
  long index;

  SYMBOL_find(class->table, class->n_desc, sizeof(CLASS_DESC_SYMBOL), TF_IGNORE_CASE, name, strlen(name), prefix, &index);
  return index;
}


PUBLIC CLASS_DESC_SYMBOL *CLASS_get_symbol(CLASS *class, const char *name)
{
  long index;

  index = CLASS_find_symbol(class, name);
  if (index == NO_SYMBOL)
    return NULL;
  else
    return &class->table[index];
}


PUBLIC CLASS_DESC *CLASS_get_symbol_desc(CLASS *class, const char *name)
{
  CLASS_DESC_SYMBOL *cds = CLASS_get_symbol(class, name);

  if (cds == NULL)
    return NULL;
  else
    return cds->desc;
}


PUBLIC short CLASS_get_symbol_index_kind(CLASS *class, const char *name, int kind, int kind2)
{
  CLASS_DESC *desc;
  long index;

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


PUBLIC CLASS_DESC *CLASS_get_symbol_desc_kind(CLASS *class, const char *name, int kind, int kind2)
{
  short index;

  index = CLASS_get_symbol_index_kind(class, name, kind, kind2);
  if (index == NO_SYMBOL)
    return NULL;
  else
    return CLASS_get_desc(class, index);
}


PUBLIC CLASS_DESC_EVENT *CLASS_get_event_desc(CLASS *class, const char *name)
{
  long index;
  CLASS_DESC *cd;

  index = CLASS_find_symbol_with_prefix(class, name, ":");
  if (index == NO_SYMBOL)
    return NULL;

  cd = class->table[index].desc;

  if (CLASS_DESC_get_type(cd) != CD_EVENT)
    return NULL;

  return &cd->event;
}


PUBLIC const char *CLASS_DESC_get_signature(CLASS_DESC *cd)
{
  #if 0
  switch (CLASS_DESC_get_type(cd))
  {
    case CD_METHOD:
    case CD_STATIC_METHOD:

      return cd->method.help;

    case CD_EVENT:

      return cd->event.help;

    case CD_PROPERTY:
    case CD_STATIC_PROPERTY:

      return cd->property.help;

    default:

      return NULL;
  }
  #endif

  return NULL;
}


PUBLIC void CLASS_free(void **pobject)
{
	void *object = *pobject;
  CLASS *class = OBJECT_class(object);

	*pobject = NULL;
  ((OBJECT *)object)->ref = 1; /* Prevents anybody from freeing the object ! */
  //fprintf(stderr, "CLASS_free: %s %p\n", class->name, object);
  EXEC_special_inheritance(SPEC_FREE, class, object, 0, TRUE);
  ((OBJECT *)object)->ref = 0;

  (*class->free)(class, object);
}

#if DEBUG_REF
PUBLIC void CLASS_ref(void *object)
{
  ((OBJECT *)object)->ref++;

  fprintf(stderr, "%s: ref(%s %p) -> %ld\n",
    DEBUG_get_current_position(),
    OBJECT_class(object)->name, object, ((OBJECT *)object)->ref);
  fflush(stdout);
}

PUBLIC void CLASS_unref(void **pobject, boolean can_free)
{
  OBJECT *object = *((OBJECT **)pobject);

  if (object->ref <= 0)
    fprintf(stderr, "*** %p REF = %ld !\n", object, object->ref);

  fprintf(stderr, "%s: unref(%s %p) -> %ld\n",
    DEBUG_get_current_position(),
    OBJECT_class(object)->name, object, object->ref - 1);
  fflush(stdout);

  /*if (strcmp(OBJECT_class(object)->name, "Class") == 0)
    fprintf(stderr, "Class ?\n");*/

  if ((--(object->ref) <= 0) && can_free)
  {
    fprintf(stderr, "FREE %p !\n", object);
    CLASS_free(pobject);
  }
}
#endif

PUBLIC void CLASS_do_nothing()
{
}

PUBLIC int CLASS_return_zero()
{
  return 0;
}


static CLASS_DESC_SYMBOL *SortClassDesc;

static int sort_desc(ushort *i1, ushort *i2)
{
  /*return strcmp(SortClassDesc[*i1].load.symbol.name, SortClassDesc[*i2].load.symbol.name);*/
  return compare_ignore_case(
    SortClassDesc[*i1].name,
    SortClassDesc[*i1].len,
    SortClassDesc[*i2].name,
    SortClassDesc[*i2].len
    );
}

PUBLIC void CLASS_sort(CLASS *class)
{
  ushort *sym;
  ushort i;

  if (class->n_desc == 0)
    return;

  ALLOC(&sym, sizeof(ushort) * class->n_desc, "CLASS_sort");

  for (i = 0; i < class->n_desc; i++)
    sym[i] = i;

  SortClassDesc = class->table;
  qsort(sym, class->n_desc, sizeof(ushort), (int (*)(const void *, const void *))sort_desc);

  for (i = 0; i < class->n_desc; i++)
    class->table[i].sort = sym[i];

  #if 0
  {
    SYMBOL *sym;

    printf("\n%s\n", class->name);

    for (i = 0; i < class->n_desc; i++)
    {
      sym = (SYMBOL *)&class->table[i];

      printf("[%d] (%d) %.*s\n", i, (int)sym->sort, (int)sym->len, sym->name);
    }
  }
  #endif

  FREE(&sym, "CLASS_sort");
}

#define GET_IF_NULL(_callback) if (class->_callback == NULL) class->_callback = class->parent->_callback

PUBLIC void CLASS_inheritance(CLASS *class, CLASS *parent)
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

  GET_IF_NULL(check);

  if (parent->auto_create)
    class->auto_create = TRUE;

	//fprintf(stderr, "CLASS_inheritance: %s %s\n", class->name, class->auto_create ? "AUTO CREATE" : "");
}


PUBLIC CLASS *CLASS_replace_global(const char *name)
{
  CLASS_SYMBOL *csym;
  char *new_name;
  CLASS *class;
  CLASS *new_class;
  int len;

  class = CLASS_find_global(name);
  if (class->state)
  {
    len = strlen(name);

    ALLOC(&new_name, len + 2, "CLASS_replace_global");
    sprintf(new_name, ">%s", name);

    new_class = CLASS_replace_global(new_name);
    FREE(&new_name, "CLASS_replace_global");

    new_name = (char *)new_class->name;

    TABLE_find_symbol(&_global_table, name, len, (SYMBOL **)&csym, NULL);
    csym->class = new_class;

    TABLE_find_symbol(&_global_table, new_name, len + 1, (SYMBOL **)&csym, NULL);
    csym->class = class;

    new_class->name = class->name;
    class->name = new_name;

    class = new_class;
  }

  return class;
}


PUBLIC void CLASS_make_description(CLASS *class, CLASS_DESC *desc, long n_desc, long *first)
{
  static const char *nonher[] = { "_new", "_free", "_init", "_exit", NULL };

  long ind;
  int i, j;
  const char *name;
  const char **pnonher;

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
      class->table[i].sort = 0;
      i++;
    }

    for (j = 0; j < n_desc; j++)
    {
      if (CLASS_is_native(class))
        name = &(desc[j].gambas.name[1]);
      else
        name = desc[j].gambas.name;

      ind = CLASS_find_symbol(class->parent, name);
      if (ind != NO_SYMBOL)
      {
        class->table[ind].desc = &desc[j];
        class->table[ind].sort = 0;
        //class->table[ind].name = ".";
        //class->table[ind].len = 1;
        if (!desc[j].gambas.val1 && index(CD_CALL_SOMETHING_LIST, CLASS_DESC_get_type(&desc[j])) != NULL)
        {
        	desc[j].gambas.val1 = class->parent->table[ind].desc->gambas.val1;
        }
      }
    }

    for (pnonher = nonher; *pnonher; pnonher++)
    {
      ind = CLASS_find_symbol(class->parent, *pnonher);
      if (ind != NO_SYMBOL)
      {
        class->table[ind].desc = NULL;
        class->table[ind].sort = 0;
        class->table[ind].name = ".";
        class->table[ind].len = 1;
      }
    }
  }

  *first = i;

  for (j = 0; j < n_desc; j++, i++)
  {
    class->table[i].desc = &desc[j];

    /* On saute le caract�e de type de symbole */
    if (CLASS_is_native(class))
      class->table[i].name = &(desc[j].gambas.name[1]);
    else
      class->table[i].name = desc[j].gambas.name;

    class->table[i].sort = 0;
    class->table[i].len = strlen(class->table[i].name);
  }
}

/* 'all' means that size_dynamic is the entire size of the object, not just the size of the data */

PUBLIC void CLASS_calc_info(CLASS *class, int n_event, int size_dynamic, boolean all, int size_static)
{
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

  class->size = class->off_event + sizeof(OBJECT_EVENT) + class->n_event * sizeof(ushort);

  class->size_stat = size_static;
  if (size_static)
    ALLOC_ZERO(&class->stat, class->size_stat, "CLASS_calc_info");
  else
    class->stat = NULL;
}


PUBLIC void CLASS_make_event(CLASS *class, int *first)
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


PUBLIC int CLASS_get_inheritance(CLASS *class, CLASS **her)
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


PUBLIC void *CLASS_auto_create(CLASS *class, int nparam)
{
  void *ob = class->instance;
  int i;

  if (ob)
  {
    if (!class->check(ob))
    {
      for (i = 0; i < nparam; i++)
        POP();

      return ob;
    }

    OBJECT_UNREF(&class->instance, "CLASS_auto_create");
    class->instance = NULL;
  }

  /*fprintf(stderr, "CLASS_auto_create: create %s\n", class->name);*/

  OBJECT_create(&class->instance, class, NULL, NULL, nparam);
  ob = class->instance;
  OBJECT_REF(ob, "CLASS_auto_create");

  return ob;
}


/*
PUBLIC CLASS *CLASS_copy(CLASS *class_src, char *prefix)
{
  CLASS *class;
  CLASS save;
  int len = strlen(prefix) + 1 + strlen(class_src->name) + 1;
  char *name;

  ALLOC(&name, len, "CLASS_copy");
  sprintf(name, "%s.%s", prefix, class_src->name);

  class = CLASS_find(name);

  if (!class->ready)
  {
    save = *class;
    *class = *class_src;

    class->name = name;
    class->ready = TRUE;
    class->copy = TRUE;
    class->template = class_src;
  }
  else
    FREE(&name, "CLASS_copy");

  return class;
}
*/


PUBLIC void CLASS_search_special(CLASS *class)
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
  class->special[SPEC_PRINT] = CLASS_get_symbol_index_kind(class, "_print", CD_METHOD, 0);

	if (class->special[SPEC_NEXT] != NO_SYMBOL)
  	class->enum_static = CLASS_DESC_get_type(CLASS_get_desc(class, class->special[SPEC_NEXT])) == CD_STATIC_METHOD;
}


PUBLIC CLASS *CLASS_check_global(char *name)
{
  CLASS *class, *parent;

  class = CLASS_find_global(name);
  if (class->state)
  {
    if (class->component == COMPONENT_current)
      ERROR_panic("Class '%s' declared twice in this component.", class->name);

    parent = class;
    class = CLASS_replace_global(name);

    CLASS_inheritance(class, parent);
  }

  return class;
}


PUBLIC CLASS_DESC_METHOD *CLASS_get_special_desc(CLASS *class, int spec)
{
  short index = class->special[spec];

  if (index == NO_SYMBOL)
    return NULL;
	else
		return &CLASS_get_desc(class, index)->method;
}


PUBLIC CLASS_DESC_SYMBOL *CLASS_get_next_sorted_symbol(CLASS *class, int *index)
{
  CLASS_DESC *old = NULL;
	int sort;
	
	for(;;)
	{
		if (*index >= class->n_desc)
			return NULL;

	  sort = class->table[*index].sort;
	  if (*index > 0)
	  	old = class->table[class->table[*index - 1].sort].desc;
  	(*index)++;
  	if (class->table[sort].desc && class->table[sort].desc != old)
  		return &class->table[sort];
	}
}

#if 0
PUBLIC void CLASS_create_array_class(CLASS *class)
{
	GB_DESC *desc;
	char *save = TYPE_joker;
	STRING_new(&TYPE_joker, class->name, strlen(class->name) - 2);

	ALLOC(&desc, sizeof(GB_DESC) * ARRAY_TEMPLATE_NDESC, "CLASS_create_array_class");
	memcpy(desc, NATIVE_TemplateArray, sizeof(GB_DESC) * ARRAY_TEMPLATE_NDESC);
	((CLASS_DESC_GAMBAS *)desc)->name = class->name;

	fprintf(stderr, "CLASS_create_array_class: %s\n", class->name);

	CLASS_register_class(desc, class);

	class->data = (char *)desc;

	STRING_free(&TYPE_joker);
	TYPE_joker = save;
}
#endif
