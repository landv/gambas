/***************************************************************************

  class.c

  Class management

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

#define __GBC_CLASS_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"

#include "gb_table.h"
#include "gbc_compile.h"
#include "gb_code.h"

static long _array_class[17];

PUBLIC void CLASS_create(CLASS **result)
{
  CLASS *class;
  TRANS_FUNC func;

  ALLOC_ZERO(&class, sizeof(CLASS), "CLASS_create");

  ARRAY_create(&class->function);
  ARRAY_create(&class->event);
  ARRAY_create(&class->prop);
  ARRAY_create(&class->ext_func);
  ARRAY_create(&class->constant);
  ARRAY_create(&class->class);
  ARRAY_create(&class->unknown);
  ARRAY_create(&class->stat);
  ARRAY_create(&class->dyn);
  ARRAY_create(&class->array);
  ARRAY_create(&class->structure);

  TABLE_create(&class->table, sizeof(CLASS_SYMBOL), TF_IGNORE_CASE);
  TABLE_create(&class->string, sizeof(SYMBOL), TF_NORMAL);

  /* Cr�tion des fonctions d'initialisation */

  TYPE_clear(&func.type);
  func.nparam = 0;
  func.start = NULL;
  func.line = 0;

  func.index = CLASS_add_symbol(class, "@init");
  CLASS_add_function(class, &func);

  func.index = CLASS_add_symbol(class, "@new");
  CLASS_add_function(class, &func);

  /* Le premier symbole de la table des symboles est vide.
     Ainsi, l'index 0 de la table n'est jamais utilis�! */

  /*CLASS_add_symbol_string(class, "");*/

  class->name = STR_copy(FILE_set_ext(FILE_get_name(JOB->name), NULL));
  class->parent = NO_SYMBOL;

  memset(_array_class, 0, sizeof(_array_class));

  *result = class;
}


static void delete_function(FUNCTION *func)
{
  ARRAY_delete(&func->local);
  ARRAY_delete(&func->code);

  if (JOB->debug)
    ARRAY_delete(&func->pos_line);

  if (func->param)
    FREE(&func->param, "delete_function");
}


static void delete_event(EVENT *event)
{
  if (event->param)
    FREE(&event->param, "delete_event");
}


static void delete_extfunc(EXTFUNC *extfunc)
{
  if (extfunc->param)
    FREE(&extfunc->param, "delete_extfunc");
}


static void delete_structure(CLASS_STRUCT *structure)
{
  if (structure->field)
    ARRAY_delete(&structure->field);
}


PUBLIC void CLASS_delete(CLASS **class)
{
  int i;

  if (*class)
  {
    TABLE_delete(&((*class)->table));
    TABLE_delete(&((*class)->string));

    for (i = 0; i < ARRAY_count((*class)->function); i++)
      delete_function(&((*class)->function[i]));

    for (i = 0; i < ARRAY_count((*class)->event); i++)
      delete_event(&((*class)->event[i]));

    for (i = 0; i < ARRAY_count((*class)->ext_func); i++)
      delete_extfunc(&((*class)->ext_func[i]));

    for (i = 0; i < ARRAY_count((*class)->structure); i++)
      delete_structure(&((*class)->structure[i]));

    ARRAY_delete(&((*class)->function));
    ARRAY_delete(&((*class)->event));
    ARRAY_delete(&((*class)->prop));
    ARRAY_delete(&((*class)->ext_func));
    ARRAY_delete(&((*class)->constant));
    ARRAY_delete(&((*class)->class));
    ARRAY_delete(&((*class)->unknown));
    ARRAY_delete(&((*class)->stat));
    ARRAY_delete(&((*class)->dyn));
    ARRAY_delete(&((*class)->array));
    ARRAY_delete(&((*class)->structure));

    if ((*class)->name != NULL)
      STR_free((*class)->name);

    FREE(class, "CLASS_delete");
  }
}


PUBLIC CLASS_SYMBOL *CLASS_declare(CLASS *class, long index, boolean global)
{
  CLASS_SYMBOL *sym = CLASS_get_symbol(class, index);

  if ((global && !TYPE_is_null(sym->global.type))
      || (!global && !TYPE_is_null(sym->local.type)))
  {
    char name[MAX_SYMBOL_LEN + 1];
    sprintf(name, "%.*s", sym->symbol.len, sym->symbol.name);
    THROW("'&1' already declared", name);
  }

  return sym;
}



PUBLIC void CLASS_add_function(CLASS *class, TRANS_FUNC *decl)
{
  FUNCTION *func;
  int i;
  CLASS_SYMBOL *sym;
  PARAM *param;

  func = ARRAY_add_void(&class->function);
  TYPE_clear(&func->type);
  func->nparam = 0;
  func->name = NO_SYMBOL;

  ARRAY_create(&func->local);
  ARRAY_create(&func->code);

  if (JOB->debug)
    ARRAY_create(&func->pos_line);

  if (!decl) return;

  sym = CLASS_declare(class, decl->index, TRUE);
  /*CLASS_add_symbol(class, JOB->table, decl->index, &sym, NULL);*/

  sym->global.type = decl->type;
  sym->global.value = ARRAY_count(class->function) - 1;

  func->nparam = decl->nparam;

  if (decl->nparam)
  {
    ALLOC(&func->param, decl->nparam * sizeof(PARAM), "CLASS_add_function");

    for (i = 0; i < decl->nparam; i++)
      func->param[i] = decl->param[i];
  }

  func->type = decl->type;
  func->start = decl->start;
  func->line = decl->line;
  func->name = decl->index;
  func->last_code = (-1);
  func->stack = 16; /* pile n�essaire aux fonctions d'initialisation */
  func->finally = 0;
  func->catch = 0;
  func->npmin = -1;
  func->vararg = decl->vararg;

  /* optional parameters */

  CODE_begin_function(func);
  JOB->func = func;

  for (i = 0; i < func->nparam; i++)
  {
    param = &func->param[i];
    if (param->optional == NULL)
      continue;

    if (func->npmin < 0)
      func->npmin = i;

    TRANS_init_optional(param);
    CODE_pop_optional(i - func->nparam);
  }

  if (func->npmin < 0)
    func->npmin = func->nparam;
}


PUBLIC void CLASS_add_event(CLASS *class, TRANS_EVENT *decl)
{
  EVENT *event;
  int i;
  CLASS_SYMBOL *sym;

  event = ARRAY_add_void(&class->event);
  TYPE_clear(&event->type);
  event->nparam = 0;
  event->name = NO_SYMBOL;

  if (!decl) return;

  sym = CLASS_declare(class, decl->index, TRUE);
  /*CLASS_add_symbol(class, JOB->table, decl->index, &sym, NULL);*/

  sym->global.type = decl->type;
  sym->global.value = ARRAY_count(class->event) - 1;

  event->nparam = decl->nparam;

  if (event->nparam)
  {
    ALLOC(&event->param, decl->nparam * sizeof(PARAM), "CLASS_add_event");

    for (i = 0; i < decl->nparam; i++)
    {
      event->param[i].type = decl->param[i].type;
      event->param[i].index = decl->param[i].index;
    }
  }

  event->type = decl->type;
  event->name = decl->index;
}


PUBLIC void CLASS_add_property(CLASS *class, TRANS_PROPERTY *decl)
{
  PROPERTY *prop;
  CLASS_SYMBOL *sym;

  prop = ARRAY_add_void(&class->prop);
  TYPE_clear(&prop->type);
  prop->name = NO_SYMBOL;
  prop->read = TRUE;
  prop->write = decl->read == 0;

  sym = CLASS_declare(class, decl->index, TRUE);
  /*CLASS_add_symbol(class, JOB->table, decl->index, &sym, NULL);*/

  sym->global.type = decl->type;
  sym->global.value = ARRAY_count(class->prop) - 1;

  prop->type = decl->type;
  prop->name = decl->index;
  prop->line = decl->line;
  prop->comment = decl->comment;
}


PUBLIC void CLASS_add_extern(CLASS *class, TRANS_EXTERN *decl)
{
  EXTFUNC *extfunc;
  int i;
  CLASS_SYMBOL *sym;

  extfunc = ARRAY_add_void(&class->ext_func);
  TYPE_clear(&extfunc->type);
  extfunc->nparam = 0;
  extfunc->name = NO_SYMBOL;

  if (!decl) return;

  sym = CLASS_declare(class, decl->index, TRUE);
  /*CLASS_add_symbol(class, JOB->table, decl->index, &sym, NULL);*/

  sym->global.type = decl->type;
  sym->global.value = ARRAY_count(class->ext_func) - 1;

  extfunc->nparam = decl->nparam;

  if (extfunc->nparam)
  {
    ALLOC(&extfunc->param, decl->nparam * sizeof(PARAM), "CLASS_add_extern");

    for (i = 0; i < decl->nparam; i++)
    {
      extfunc->param[i].type = decl->param[i].type;
      extfunc->param[i].index = decl->param[i].index;
    }
  }

  extfunc->type = decl->type;
  extfunc->name = decl->index;
  extfunc->library = decl->library;
  extfunc->alias = decl->alias;
}


PUBLIC long CLASS_add_constant(CLASS *class, TRANS_DECL *decl)
{
  CONSTANT *desc;
  long num;

  num =  ARRAY_count(class->constant);

  desc = ARRAY_add(&class->constant);
  desc->type = decl->type;
  desc->index = decl->index;

  desc->value = decl->value;
  if (TYPE_get_id(decl->type) == T_LONG)
    desc->lvalue = decl->lvalue;

  desc->line = JOB->line;

  return num;
}


PUBLIC long CLASS_add_class(CLASS *class, long index)
{
  long num;
  CLASS_REF *desc;
  CLASS_SYMBOL *sym = CLASS_get_symbol(class, index);

  num = sym->class - 1;
  if (num < 0)
  {
    num =  ARRAY_count(class->class);

    desc = ARRAY_add(&class->class);
    desc->index = index;

    sym->class = num + 1;

    if (JOB->verbose)
      printf("Adding class %.*s\n", sym->symbol.len, sym->symbol.name);
  }

	JOB->class->class[num].used = TRUE;
  return num;
}

PUBLIC long CLASS_add_class_unused(CLASS *class, long index)
{
	long num = CLASS_add_class(class, index);
	JOB->class->class[num].used = FALSE;
	return num;
}


PUBLIC boolean CLASS_exist_class(CLASS *class, long index)
{
  return CLASS_get_symbol(class, index)->class > 0;
}


PUBLIC long CLASS_get_array_class(CLASS *class, long type)
{
  static char *name[] = {
    NULL, "Boolean[]", "Byte[]", "Short[]", "Integer[]", "Long[]", "Single[]", "Float[]",
    "Date[]", "String[]", "String[]", "Variant[]", NULL, NULL, NULL, NULL, "Object[]"
    };

  long index;

  if (type <= T_VOID || type > T_OBJECT)
    return NO_SYMBOL;

  index = _array_class[type];

  if (index == 0)
  {
    if (!TABLE_find_symbol(class->table, name[type], strlen(name[type]), NULL, &index))
      index = CLASS_add_symbol(class, name[type]);
    index = CLASS_add_class(class, index);
    _array_class[type] = index;
    //printf("%s -> %ld\n", name[type], index);
  }

  return index;
}


PUBLIC long CLASS_add_unknown(CLASS *class, long index)
{
  long num;
  long *desc;
  CLASS_SYMBOL *sym = CLASS_get_symbol(class, index);

  num = sym->unknown - 1;
  if (num < 0)
  {
    num =  ARRAY_count(class->unknown);

    desc = ARRAY_add(&class->unknown);
    *desc = index;

    sym->unknown = num + 1;
  }

  return num;
}



PUBLIC long CLASS_add_array(CLASS *class, TRANS_ARRAY *array)
{
  CLASS_ARRAY *desc;
  long num;
  int i;

  num =  ARRAY_count(class->array);

  desc = ARRAY_add(&class->array);
  desc->type = array->type;
  desc->ndim = array->ndim;
  for (i = 0; i < desc->ndim; i++)
    desc->dim[i] = array->dim[i];

  return num;
}



PUBLIC void CLASS_add_declaration(CLASS *class, TRANS_DECL *decl)
{
  CLASS_SYMBOL *sym = CLASS_declare(class, decl->index, TRUE);
  VARIABLE *var;

  sym->global.type = decl->type;

  if (TYPE_get_kind(decl->type) == TK_CONST)
  {
    sym->global.value = CLASS_add_constant(class, decl);
  }
  else if (TYPE_is_static(decl->type))
  {
    sym->global.value = ARRAY_count(class->stat);
    var = ARRAY_add(&class->stat);

    var->type = decl->type;
    var->index = decl->index;
    var->pos = class->size_stat;
    var->size = TYPE_sizeof(var->type);

    class->size_stat += var->size;

    CODE_begin_function(&class->function[FUNC_INIT_STATIC]);
    if (TRANS_init_var(decl))
      CODE_pop_global(sym->global.value, TRUE);
  }
  else
  {
    sym->global.value = ARRAY_count(class->dyn);
    var = ARRAY_add(&class->dyn);

    var->type = decl->type;
    var->index = decl->index;
    var->pos = class->size_dyn;
    var->size = TYPE_sizeof(var->type);

    class->size_dyn += var->size;

    CODE_begin_function(&class->function[FUNC_INIT_DYNAMIC]);
    if (TRANS_init_var(decl))
      CODE_pop_global(sym->global.value, FALSE);
  }
}



static void reorder_decl(CLASS *class, VARIABLE *tvar, const char *desc)
{
  long count;
  long pos;
  VARIABLE *var;
  int i, j;
  int n;

  /* variables statiques */

  count = ARRAY_count(tvar);
  if (count > 1)
  {
    if (JOB->verbose)
      printf("Reordering %s variables:", desc);

    pos = 0;
    for (i = 0; i < 3; i++)
    {
      for (j = 0; j < count; j++)
      {
        var = &tvar[j];
        n = var->size & 3;

        switch (i)
        {
          case 0: if (n != 0) continue; else break;
          case 1: if (n != 2) continue; else break;
          case 2: if (n != 1 && n != 3) continue; else break;
        }

        var->pos = pos;
        pos += var->size;

        if (JOB->verbose)
          printf(" %s", TABLE_get_symbol_name(class->table, var->index));
      }
    }

    if (JOB->verbose)
      printf("\n");
  }

}


PUBLIC void CLASS_sort_declaration(CLASS *class)
{
  reorder_decl(class, class->stat, "static");
  reorder_decl(class, class->dyn, "dynamic");
}


PUBLIC long CLASS_add_symbol(CLASS *class, const char *name)
{
  long index;

  TABLE_add_symbol(class->table, name, strlen(name), NULL, &index);
  return index;
}


/*
PUBLIC long CLASS_add_symbol(CLASS *class, long index_src)
{
  SYMBOL *sym_src = TABLE_get_symbol(class->table, index_src);
  long index;

  TABLE_add_symbol(class->string, sym_src->name, sym_src->len, NULL, &index);
  return index;
}


PUBLIC long CLASS_add_symbol_string(CLASS *class, char *str)
{
  long index;

  TABLE_add_symbol(class->string, str, strlen(str), NULL, &index);
  return index;
}
*/


PUBLIC void FUNCTION_add_pos_line(void)
{
  short *pos;

  if (JOB->debug)
  {
    pos = ARRAY_add(&JOB->func->pos_line);
    *pos = CODE_get_current_pos();
    //printf("line %ld : %d\n", ARRAY_count(JOB->func->pos_line) - 1, *pos);
  }
}


PUBLIC char *FUNCTION_get_fake_name(int func)
{
  static char buf[6];

  sprintf(buf, "$%d", func);
  return buf;
}


static long check_one_property_func(CLASS *class, PROPERTY *prop, bool write)
{
  CLASS_SYMBOL *sym;
  char *name;
  bool is_static;
  FUNCTION *func;
  long index;

  JOB->line = prop->line;

  is_static = TYPE_is_static(prop->type);

  name = STR_copy(TABLE_get_symbol_name_suffix(class->table, prop->name, write ? "_Write" : "_Read"));

  if (!TABLE_find_symbol(class->table, name, strlen(name), (SYMBOL **)&sym, &index))
    THROW("&1 is not declared", name);

  if (TYPE_get_kind(sym->global.type) != TK_FUNCTION)
    THROW("&1 is declared but is not a function", name);

  func = &class->function[sym->global.value];
  JOB->line = func->line;

  if (TYPE_is_public(sym->global.type))
    THROW("A property implementation cannot be public");

  if (is_static != TYPE_is_static(sym->global.type))
  {
    if (is_static)
      THROW("&1 must be static", name);
    else
      THROW("&1 cannot be static", name);
  }

  if (write)
  {
    if (TYPE_get_id(func->type) != T_VOID)
      goto _BAD_SIGNATURE;

    if (func->nparam != 1 || func->npmin != 1)
      goto _BAD_SIGNATURE;

    if (!TYPE_compare(&func->param[0].type, &prop->type))
      goto _BAD_SIGNATURE;
  }
  else
  {
    if (!TYPE_compare(&func->type, &prop->type))
      goto _BAD_SIGNATURE;

    if (func->nparam != 0 || func->npmin != 0)
      goto _BAD_SIGNATURE;
  }

  STR_free(name);
  return sym->global.value;

_BAD_SIGNATURE:

  THROW("&1 declaration does not match", name);

}


PUBLIC void CLASS_check_properties(CLASS *class)
{
  int i;
  PROPERTY *prop;

  for (i = 0; i < ARRAY_count(class->prop); i++)
  {
    prop = &class->prop[i];

    prop->read = check_one_property_func(class, prop, FALSE);
    if (prop->write)
      prop->write = check_one_property_func(class, prop, TRUE);
    else
      prop->write = NO_SYMBOL;
  }
}


