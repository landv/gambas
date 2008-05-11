/***************************************************************************

  dump.c

  Class dumping

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

#define __GBC_DUMP_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "gb_common.h"
#include "gb_error.h"

#include "gb_table.h"
#include "gbc_compile.h"
#include "gb_code.h"
#include "gb_file.h"
#include "gbc_chown.h"


static FILE *_flist = NULL;
static FILE *_finfo = NULL;


static const char *get_name(int index)
{
  return TABLE_get_symbol_name(JOB->class->table, index);
}


static const char *get_string(int index)
{
  return TABLE_get_symbol_name(JOB->class->string, index);
}


static void dump_name(int index)
{
  printf("%s", get_name(index));
}


static void dump_type(TYPE type, boolean as)
{
  int value;
  TYPE_ID id;
  CLASS_ARRAY *array;
  int i;

  id = TYPE_get_id(type);
  value = TYPE_get_value(type);

  if (id == T_ARRAY)
  {
    array = &JOB->class->array[value];

    printf("[");
    for (i = 0; i < array->ndim; i++)
    {
      if (i > 0)
        printf(",");
      printf("%d", array->dim[i]);
    }
    printf("] AS ");
    dump_type(array->type, FALSE);
  }
  else if (id == T_OBJECT && value >= 0)
  {
    if (as)
      printf(" AS ");
    dump_name(JOB->class->class[value].index);
  }
  else
  {
    if (as)
      printf(" AS ");
    printf("%s", TYPE_get_desc(type));
  }
}


static void dump_function(FUNCTION *func)
{
  int i;

  printf("(");

  for (i = 0; i < func->nparam; i++)
  {
    if (i > 0) printf(", ");

    if (i == func->npmin)
      printf("OPTIONAL ");

    dump_name(func->param[i].index);
    dump_type(func->param[i].type, TRUE);
  }

  if (func->vararg)
  {
    if (func->nparam > 0)
      printf(", ");
    printf("...");
  }

  printf(")");
}



PUBLIC void CLASS_dump(void)
{
  int i;
  TYPE type;
  CLASS_SYMBOL *sym;
  CLASS *class = JOB->class;

  printf("\n");

  if (JOB->is_module)
    printf("MODULE ");
  else if (JOB->is_form)
    printf("FORM ");
  else
    printf("CLASS ");

  printf("%s\n", class->name);

  if (class->parent != NO_SYMBOL)
  {
    printf("CLASS INHERITS ");
    dump_name(class->class[class->parent].index);
    printf("\n");
  }

  if (class->exported)
    printf("EXPORT\n");

  if (class->autocreate)
    printf("CREATE\n");

  if (class->optional)
    printf("OPTIONAL\n");

  printf("\n");

  printf("Static size : %d octets\n", class->size_stat);
  printf("Dynamic size : %d octets\n\n", class->size_dyn);

  for (i = 0; i < TABLE_count(class->table); i++)
  {
    sym = CLASS_get_symbol(class, i);
    type = sym->global.type;
    if (TYPE_is_null(type))
      continue;

    if (TYPE_is_static(type)) printf("STATIC ");
    if (TYPE_is_public(type)) printf("PUBLIC "); else printf("PRIVATE ");

    switch(TYPE_get_kind(type))
    {
      case TK_VARIABLE:

        dump_name(i);
        dump_type(type, TRUE);
        break;

      case TK_FUNCTION:

        if (TYPE_get_id(type) == T_VOID)
          printf("PROCEDURE ");
        else
          printf("FUNCTION ");

        dump_name(i);
        dump_function(&class->function[sym->global.value]);

        break;

      case TK_CONST:

        printf("CONST ");
        dump_name(i);
        dump_type(type, TRUE);
        printf(" = ");
        dump_name(class->constant[sym->global.value].index);

        break;

      case TK_PROPERTY:

        printf("PROPERTY ");
        if (class->prop[sym->global.value].write == NO_SYMBOL)
          printf("READ ");
        dump_name(i);
        dump_type(type, TRUE);
        break;

      case TK_EVENT:

        printf("EVENT ");
        dump_name(i);
        break;

      case TK_UNKNOWN: printf("UNKNOWN "); break;
      case TK_EXTERN: printf("EXTERN "); break;
      case TK_LABEL: printf("LABEL "); break;
    }

    printf("\n");

    /*
    if (TYPE_get_kind(type) == TK_FUNCTION)
    {
      func = &class->function[value];
      printf(" L:%ld", func->line);
    }
    else if (TYPE_get_kind(type) == TK_EVENT)
      func = (FUNCTION *)&class->event[value];
    else if (TYPE_get_kind(type) == TK_EXTERN)
    {
      func = (FUNCTION *)&class->ext_func[value];
      printf(" in %s", TABLE_get_symbol_name(class->table, class->ext_func[value].library));
    }
    */
  }

  printf("\n");
}

static void export_newline(void)
{
  fputc('\n', _finfo);
}

static void export_type(TYPE type, bool scomma)
{
  int value;
  TYPE_ID id;

  id = TYPE_get_id(type);
  value = TYPE_get_value(type);

  if (id == T_OBJECT && value >= 0)
  {
    fprintf(_finfo, get_name(JOB->class->class[value].index));
    if (scomma)
      fputc(';', _finfo);
  }
  else
    fprintf(_finfo, TYPE_get_short_desc(type));
}


static void export_signature(int nparam, int npmin, PARAM *param, bool vararg)
{
  int i;

  for (i = 0; i < nparam; i++)
  {
    if (i == npmin)
      fprintf(_finfo, "[");

    fprintf(_finfo, "(%s)", get_name(param[i].index));
    export_type(param[i].type, TRUE);
  }

  if (npmin < nparam)
    fprintf(_finfo, "]");

  if (vararg)
    fprintf(_finfo, ".");

  export_newline();
}


PUBLIC void CLASS_export(void)
{
  const char *path;
  CLASS *class = JOB->class;
  int i;
  TYPE type;
  CLASS_SYMBOL *sym;
  char kind;
  int val;
  FUNCTION *func;
  EVENT *event;
  EXTFUNC *extfunc;

  if (!_flist)
  {
    path = FILE_cat(FILE_get_dir(JOB->name), ".list#", NULL);
    _flist = fopen(path, "w");
    if (!_flist)
      THROW("Cannot create file: &1", path);

    path = FILE_cat(FILE_get_dir(JOB->name), ".info#", NULL);
    _finfo = fopen(path, "w");
    if (!_finfo)
      THROW("Cannot create file: &1", path);
  }

  fprintf(_flist, "%s%s\n", class->name, class->optional ? "?" : "");

  fprintf(_finfo, "#%s\n", class->name);

  if (class->parent != NO_SYMBOL)
    fprintf(_finfo, "%s", get_name(JOB->class->class[class->parent].index));
  export_newline();

	if (!class->nocreate)
		fprintf(_finfo, "C");
  if (class->autocreate)
    fprintf(_finfo, "A");
  if (class->optional)
    fprintf(_finfo, "O");
  export_newline();

  for (i = 0; i < TABLE_count(class->table); i++)
  {
    sym = CLASS_get_symbol(class, i);
    type = sym->global.type;

    if (TYPE_is_null(type))
      continue;
    if (!TYPE_is_public(type))
      continue;

    switch(TYPE_get_kind(type))
    {
      case TK_CONST:

        kind = 'C';
        break;

      case TK_FUNCTION:

        kind = 'm';
        break;

      case TK_PROPERTY:

        if (class->prop[sym->global.value].write == NO_SYMBOL)
          kind = 'r';
        else
          kind = 'p';
        break;

      case TK_EVENT:

        kind = ':';
        break;

      case TK_EXTERN:

        kind = 'X';
        break;

      default:
        continue;
    }

    fprintf(_finfo, "%s\n", get_name(i));
    fprintf(_finfo, "%c\n", TYPE_is_static(type) ? toupper(kind) : kind);

    export_type(type, FALSE);

    if (kind == 'r' || kind == 'p')
    {
      val = class->prop[sym->global.value].comment;
      if (val != NO_SYMBOL)
        fprintf(_finfo, "%s", get_string(val));
    }

    export_newline();

    switch(kind)
    {
      case 'C':
        val = class->constant[sym->global.value].value;

        switch(TYPE_get_id(type))
        {
          case T_BOOLEAN:
          case T_BYTE:
          case T_SHORT:
          case T_INTEGER:
            fprintf(_finfo, "%d\n", val);
            break;

          case T_LONG:
          case T_SINGLE:
          case T_FLOAT:
          case T_STRING:
            fprintf(_finfo, "%s\n", get_string(val));
            break;

          default:
            export_newline();
            break;
        }
        break;

      case 'm':
        func = &class->function[sym->global.value];
        export_signature(func->nparam, func->npmin, func->param, func->vararg);
        break;

      case ':':
        event = &class->event[sym->global.value];
        export_signature(event->nparam, event->nparam, event->param, FALSE);
        break;

      case 'X':
        extfunc = &class->ext_func[sym->global.value];
        export_signature(extfunc->nparam, extfunc->nparam, extfunc->param, FALSE);
        break;

      default:
        export_newline();
    }
  }
}


PUBLIC void CLASS_exit_export(void)
{
  if (_flist)
  {
    fclose(_flist);
    fclose(_finfo);
    chdir(FILE_get_dir(COMP_project));
    unlink(".list"); 
    rename(".list#", ".list");
    FILE_set_owner(".list", COMP_project);
    unlink(".info"); 
    rename(".info#", ".info");
    FILE_set_owner(".info", COMP_project);
  }
  else
  {
    chdir(FILE_get_dir(COMP_project));
  	unlink(".list");
  	unlink(".info");
  }
}

