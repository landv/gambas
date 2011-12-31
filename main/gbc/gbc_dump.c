/***************************************************************************

  gbc_dump.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBC_DUMP_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_error.h"

#include "gb_table.h"
#include "gb_str.h"
#include "gbc_compile.h"
#include "gb_code.h"
#include "gb_file.h"
#include "gbc_chown.h"
#include "gbc_output.h"


static FILE *_finfo;


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


static void dump_type(TYPE type, bool as)
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
    printf("] As ");
    dump_type(array->type, FALSE);
  }
  else if (id == T_OBJECT && value >= 0)
  {
    if (as)
      printf(" As ");
    dump_name(JOB->class->class[value].index);
  }
  else
  {
    if (as)
      printf(" As ");
    printf("%s", TYPE_get_desc(type));
  }
}


static void dump_function(FUNCTION *func)
{
  int i;

	//printf("<%lld> ", func->byref);
	
  printf("(");

  for (i = 0; i < func->nparam; i++)
  {
    if (i > 0) printf(", ");

    if (i >= func->npmin)
      printf("Optional ");
		
		if (func->byref & (1LL << i))
			printf("ByRef ");

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



void CLASS_dump(void)
{
  int i;
  TYPE type;
  CLASS_SYMBOL *sym;
  CLASS *class = JOB->class;

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
    putchar('\n');
  }

  if (class->exported)
    printf("EXPORT\n");

  if (class->autocreate)
    printf("CREATE\n");

  if (class->optional)
    printf("OPTIONAL\n");

  putchar('\n');

  printf("Static size : %d octets\n", class->size_stat);
  printf("Dynamic size : %d octets\n\n", class->size_dyn);

  for (i = 0; i < TABLE_count(class->table); i++)
  {
    sym = CLASS_get_symbol(class, i);
    type = sym->global.type;
    if (TYPE_is_null(type))
      continue;

    if (TYPE_is_static(type)) printf("Static ");
    if (TYPE_is_public(type)) printf("Public "); else printf("Private ");

    switch(TYPE_get_kind(type))
    {
      case TK_VARIABLE:

        dump_name(i);
        dump_type(type, TRUE);
        break;

      case TK_FUNCTION:

        if (TYPE_get_id(type) == T_VOID)
          printf("Procedure ");
        else
          printf("Function ");

        dump_name(i);
        dump_function(&class->function[sym->global.value]);

        break;

      case TK_CONST:

        printf("Const ");
        dump_name(i);
        dump_type(type, TRUE);
        printf(" = ");
        dump_name(class->constant[sym->global.value].index);

        break;

      case TK_PROPERTY:

        printf("Property ");
        if (class->prop[sym->global.value].write == NO_SYMBOL)
          printf("Read ");
        dump_name(i);
        dump_type(type, TRUE);
        break;

      case TK_EVENT:

        printf("Event ");
        dump_name(i);
        break;

      case TK_UNKNOWN: printf("Unknown "); break;
      case TK_EXTERN: printf("Extern "); break;
      case TK_LABEL: printf("Label "); break;
    }

    putchar('\n');

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

  putchar('\n');
}

static void export_newline(void)
{
  fputc('\n', _finfo);
}

static void export_type(TYPE type, bool scomma)
{
  int value;
	int index;
  TYPE_ID id;

  id = TYPE_get_id(type);
  value = TYPE_get_value(type);

  if (id == T_OBJECT && value >= 0)
  {
    fprintf(_finfo, "%s", get_name(JOB->class->class[value].index));
    if (scomma)
      fputc(';', _finfo);
  }
  else if (id == T_ARRAY)
	{
		type = JOB->class->array[value].type;
		index = CLASS_get_array_class(JOB->class, TYPE_get_id(type), TYPE_get_value(type));
		fprintf(_finfo, "%s", get_name(JOB->class->class[index].index));
    if (scomma)
      fputc(';', _finfo);
	}
  // TODO: Manage T_STRUCT
  else
    fprintf(_finfo, "%s", TYPE_get_short_desc(type));
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

static void create_file(FILE **fw, const char *file)
{
	if (!*fw)
	{
		*fw = fopen(file, "w");
		if (!*fw)
			THROW("Cannot create file: &1", FILE_cat(FILE_get_dir(COMP_project), file, NULL));
	}
}

static char *read_line(FILE *f, int *plen)
{
	int len;
	char *line;
	
	if (!f)
		return NULL;
		
	line = fgets(COMMON_buffer, COMMON_BUF_MAX, f);
	
	if (!line)
		return NULL;
	
	len = strlen(line);
	if (line[len - 1] == '\n')
	{
		line[len - 1] = 0;
		len--;
	}
	
	if (plen)
		*plen = len;
		
	return line;
}

static void close_file_and_rename(FILE *f, const char *file, const char *dest)
{
	if (f)
	{
		fclose(f);
    FILE_unlink(dest); 
    FILE_rename(file, dest);
    FILE_set_owner(dest, COMP_project);
	}
	else
	{
		FILE_unlink(file);
		FILE_unlink(dest);
	}
}

static bool exist_bytecode_file(char *name)
{
	char *output = OUTPUT_get_file(name);
	bool exist = FILE_exist(output);
	STR_free(output);
	return exist;
}

static void class_update_exported(CLASS *class)
{
	FILE *fw = NULL;
	FILE *fr;
	char *name;
	int len;
	bool inserted = FALSE;
	bool optional;
	int cmp;
	
	fr = fopen(".list", "r");
	
	if (!fr && !class->exported)
		return;
	
	for(;;)
	{
		name = read_line(fr, &len);
		optional = FALSE;

		if (!name)
			cmp = 1;
		else
		{
			if (name[len - 1] == '?')
			{
				optional = TRUE;
				name[len - 1] = 0;
			}
			cmp = strcmp(name, class->name);
		}
		
		if (cmp == 0)
		{
			if (JOB->verbose)
				printf("Remove '%s' from .list file\n", name);
			continue;
		}
		else if ((cmp > 0) && class->exported && !inserted)
		{
			create_file(&fw, ".list#");
			if (JOB->verbose)
				printf("Insert '%s%s' into .list file\n", class->name, class->optional ? "?" : "");
			fputs(class->name, fw);
			if (class->optional)
				fputc('?', fw);
			fputc('\n', fw);
			inserted = TRUE;
		}
		
		if (!name)
			break;
		
		if (exist_bytecode_file(name))
		{
			if (JOB->verbose)
				printf("Copy '%s' in .list file\n", name);
		
			create_file(&fw, ".list#");		
			fputs(name, fw);
			if (optional)
				fputc('?', fw);
			fputc('\n', fw);
		}
		else
		{
			if (JOB->verbose)
				printf("Remove '%s' from .list file\n", name);
		}
	}
	
	if (fr)
		fclose(fr);
	
	close_file_and_rename(fw, ".list#", ".list");
}

static void insert_class_info(CLASS *class, FILE *fw)
{
  int i;
  TYPE type;
  CLASS_SYMBOL *sym;
  char kind;
  int val;
  FUNCTION *func;
  EVENT *event;
  EXTFUNC *extfunc;
  CONSTANT *cst;
	
	if (JOB->verbose)
		printf("Insert '%s' information into .info file\n", class->name);
		
	_finfo = fw;

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

			case TK_VARIABLE:

				kind = 'v';
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
				cst = &class->constant[sym->global.value];

				switch(TYPE_get_id(type))
				{
					case T_BOOLEAN:
					case T_BYTE:
					case T_SHORT:
					case T_INTEGER:
						fprintf(_finfo, "%d\n", cst->value);
						break;

					case T_LONG:
						fprintf(_finfo, "%" PRId64 "\n", cst->lvalue);
						break;
					
					case T_SINGLE:
					case T_FLOAT:
						fprintf(_finfo, "%s\n", get_name(cst->value));
						break;
					
					case T_STRING:
						fprintf(_finfo, "%s\n", get_string(cst->value));
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

void CLASS_export(void)
{
	FILE *fw = NULL;
	FILE *fr;
	char *line;
	int len;
	bool inserted = FALSE;
  CLASS *class = JOB->class;
  int cmp;
  const char *msg;

  if (chdir(FILE_get_dir(COMP_project)))
  {
    msg = "Cannot change directory";
    goto __ERROR;
  }
	
	class_update_exported(class);
	
	fr = fopen(".info", "r");

	line = read_line(fr, &len);

	for(;;)
	{
		if (!line)
			cmp = 1;
		else
			cmp = strcmp(&line[1], class->name);

		if (cmp == 0)
		{
			if (JOB->verbose)
				printf("Remove '%s' information from .info file\n", class->name);
		
			for(;;)
			{
				line = read_line(fr, &len);
				if (!line || *line == '#')
					break;
			}
			
			continue;
		}
		
		if (cmp > 0 && class->exported && !inserted)
		{
			create_file(&fw, ".info#");
			insert_class_info(class, fw);
			inserted = TRUE;
		}
		
		if (!line)
			break;
		
		// copying class information
		
		if (exist_bytecode_file(&line[1]))
		{
			if (JOB->verbose)
				printf("Copy '%s' information in .info file\n", &line[1]);
			for(;;)
			{
				create_file(&fw, ".info#");
				fputs(line, fw);
				fputc('\n', fw);
				line = read_line(fr, NULL);
				if (!line || *line == '#')
					break;
			}
		}
		else
		{
			if (JOB->verbose)
				printf("Remove '%s' information from .info file\n", &line[1]);
			for(;;)
			{
				line = read_line(fr, NULL);
				if (!line || *line == '#')
					break;
			}
		}
	}
	
	if (fr)
		fclose(fr);
	
	close_file_and_rename(fw, ".info#", ".info");
	return;
	
__ERROR:

  THROW("Cannot create class information: &1: &2", msg, strerror(errno));
}

