/***************************************************************************

  gbc_form.c

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

#define __GBC_FORM_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_file.h"
#include "gb_str.h"
#include "gbc_compile.h"
#include "gbc_chown.h"
#include "gbc_form.h"

/*#define DEBUG*/

static char *_source;
static const char *_current;

static FORM_PARENT form_parent[MAX_FORM_PARENT];
static int form_parent_level;

static bool _no_trim = FALSE;


void FORM_print_len(const char *buffer, int len)
{
	if (JOB->verbose)
		printf("%.*s", len, buffer);

	BUFFER_add(&JOB->source, buffer, len);
}


void FORM_print(const char *buffer)
{
	if (JOB->verbose)
		printf("%s", buffer);
	
	BUFFER_add(&JOB->source, buffer, strlen(buffer));
}


void FORM_print_char(char c)
{
	if (JOB->verbose)
		putchar(c);
	
	BUFFER_add_char(&JOB->source, c);
}


static void print_fmt(const char *before, const char *word, int len, const char *after)
{
	FORM_print(before);
	FORM_print_len(word, len);
	FORM_print(after);
}


static void print_var(const char *before, const char *word, int len, const char *after)
{
	FORM_print(before);
	if (!word)
		FORM_print("Me");
	else
	{
		FORM_print("{");
		FORM_print_len(word, len);
		FORM_print("}");
	}
	FORM_print(after);
}


static bool read_line(const char **str, int *len)
{
	const char *start, *end;
	unsigned char car;
	int l;

	if (_no_trim)
	{
		car = *_current;
		if (!car)
			return TRUE;
	}
	else
	{
		for(;;)
		{
			car = *_current;
			if (!car)
				return TRUE;
			if (car > ' ')
				break;
			
			_current++;
		}
	}

	start = _current;
	for(;;)
	{
		car = *_current++;
		if (car == '\n') // || car =='\r' || !car)
			break;
		//if (car > ' ')
		//	nospace = _current;
	}

	end = _current;
	l = (int)(end - start);
	
	while (l > 0 && (uchar)end[-1] <= ' ')
	{
		end--;
		l--;
	}

	*str = start;
	*len = l;
	return FALSE;
}


static char *get_word(const char **str, int *len)
{
	char car;
	const char *pos;

	*len = 0;

	for(;;)
	{
		car = **str;
		if (car == '\n')
			return NULL;
		if (!isspace(car))
			break;
		(*str)++;
	}

	pos = *str;

	for(;;)
	{
		car = **str;
		if (isspace(car))
			break;
		(*str)++;
		(*len)++;
	}

	return (char *)pos;
}


static void parent_enter(const char *str, int len)
{
	if (form_parent_level >= MAX_FORM_PARENT)
		THROW("&1: too many nested containers", JOB->form);

	form_parent[form_parent_level].name = (char *)str;
	form_parent[form_parent_level].len = len;
	form_parent_level++;
}


static void parent_leave(void)
{
	if (form_parent_level == 0)
		THROW("&1: syntax error", JOB->form);

	form_parent_level--;
}


static void parent_get(char **str, int *len, int add)
{
	if (form_parent_level < add)
	{
		*str = NULL;
		*len = 0;
	}
	else
	{
		FORM_PARENT *fp = &form_parent[form_parent_level - add];
		*str = fp->name;
		*len = fp->len;
	}
}


static void get_container(char **str, int *len)
{
	parent_get(str, len, 2);
}

static void get_current(char **str, int *len)
{
	parent_get(str, len, 1);
}

static void save_action(bool delete)
{
	FILE *file;
	const char *path;
	char *name;
	const char *line;
	int len;
	
	path = FILE_cat(FILE_get_dir(COMP_project), ".action", NULL);
	mkdir(path, 0777);
	FILE_set_owner(path, COMP_project);
	
	name = STR_copy(FILE_set_ext(FILE_get_name(JOB->form), "action"));
	path = FILE_cat(FILE_get_dir(COMP_project), ".action", name, NULL);
	
	if (delete)
	{
		if (FILE_exist(path))
		{
			if (JOB->verbose)
				printf("Deleting action file %s\n", path);
			
			FILE_unlink(path);
		}
	}
	else
	{
		if (JOB->verbose)
			printf("Writing action file %s\n", path);
		
		file = fopen(path, "w");
		if (!file)
			THROW("Cannot create action file: &1", path);

		fputs("# Gambas Action File 3.0\n", file);

		_no_trim = TRUE;
		while (!read_line(&line, &len))
		{
			fwrite(line, sizeof(char), len, file);
			putc('\n', file);
		}
		_no_trim = FALSE;

		if (fclose(file))
			THROW("Cannot create action file: &1", path);
			
		FILE_set_owner(path, COMP_project);
	}
	
	STR_free(name);
}

char *FORM_get_file_family(const char *file, const FORM_FAMILY **family)
{
	char *form;
	const FORM_FAMILY *p;

	if (strcmp(FILE_get_ext(file), "class"))
		return NULL;

	p = COMP_form_families;
	
	while (p->ext)
	{
		form = STR_copy(FILE_set_ext(file, p->ext));
		if (FILE_exist(form))
			break;
		STR_free(form);
		form = NULL;
		p++;
	}

	if (form) *family= p;
	return form;
}


void FORM_do(char *source, bool ctrl_public)
{
	const char *line;
	char *word;
	char *win = NULL;
	char *twin = NULL;

	int len, len_twin;
	const char *pos_rewind;
	bool virtual;
	bool public;
	bool action;

	if (JOB->form == NULL)
		return;

	_source = source;
	_current = _source;

	/* version */

	if (read_line(&line, &len))
		goto _ERROR;

	if (strncasecmp(line, "# Gambas Form File 3.0", len))
		THROW("Bad form file version");

	pos_rewind = _current;
	form_parent_level = 0;

	//FORM_print("@SECTION\n");

	while (!read_line(&line, &len))
	{
		if (len == 0 || *line == '#' || *line == '\'')
			continue;

		if (*line == '{')
		{
			form_parent_level++;
			line++;

			word = get_word(&line, &len);
			if (word == NULL)
				goto _ERROR;

			if (win != NULL)
			{
				if (word[0] == '!')
				{
					word++;
					len--;
					public = TRUE;
				}
				else
					public = FALSE;
				
				if (ctrl_public || public)
					FORM_print("Public");
				else
					FORM_print("Private");

				//print_fmt(" {%.*s} ", len, word);
				print_fmt(" {", word, len, "} ");
			}

			if (win == NULL)
			{
				win = word;
			}

			word = get_word(&line, &len);
			if (word == NULL)
				goto _ERROR;

			if (twin != NULL)
			{
				if (*word == '#')
				{
					word++;
					len--;
				}
				//print_fmt("AS %.*s\n", len, word);
				print_fmt("As ", word, len, "\n");
			}

			if (twin == NULL)
			{
				twin = word;
				len_twin = len;

				//print_fmt("INHERITS %.*s\n\n", len_twin, twin);
				print_fmt("Inherits ", twin, len_twin, "\n\n");
			}
		}
		else if (*line == '}')
		{
			form_parent_level--;
			if (form_parent_level <= 0)
				break;
		}
	}

	FORM_print("\nPrivate Sub {$load}()\n\n");

	_current = pos_rewind;
	form_parent_level = 0;

	while (!read_line(&line, &len))
	{
		if (len == 0 || *line == '#' || *line == '\'')
			continue;

		if (*line == '{')
		{
			line++;

			word = get_word(&line, &len);
			if (word == NULL)
				goto _ERROR;

			if (form_parent_level == 0)
			{
				parent_enter(NULL, 0);
				FORM_print("  With Me\n");
			}
			else
			{
				//print_fmt("  {%.*s} = NEW ", len, word);
				if (word[0] == '!')
				{
					word++;
					len--;
				}
				
				print_fmt("  {", word, len, "} = New ");
				parent_enter(word, len);

				word = get_word(&line, &len);
				if (word == NULL)
					goto _ERROR;

				if (*word == '#')
				{
					virtual = TRUE;
					word++;
					len--;
				}
				else
					virtual = FALSE;

				//print_fmt("%.*s", len, word);
				FORM_print_len(word, len);

				if (!virtual)
				{
					get_container(&word, &len);
					print_var("(", word, len, ")");
				}

				word = get_word(&line, &len);
				if (word == NULL)
					get_current(&word, &len);

				//print_fmt(" AS \"%.*s\"\n", len, word);
				print_fmt(" As \"", word, len, "\"\n");

				get_current(&word, &len);
				print_var("  With ", word, len, "\n");
			}
		}
		else if (*line == '}')
		{
			FORM_print("  End With\n");
			parent_leave();
			if (form_parent_level == 0)
				break;
		}
		else
		{
			/*get_current(&word, &len_word);*/
			/*FORM_print("  %.*s", len_word, word);*/
			FORM_print("    .");
			FORM_print_len(line, len);
			FORM_print("\n");
		}
	}

	if (form_parent_level > 0)
		goto _ERROR;

	FORM_print("\nEnd\n\n");

	// Create or delete the action file if needed
	
	action = FALSE;
	
	while (!read_line(&line, &len))
	{
		if (!strncasecmp(line, "# Gambas Action File 3.0", len))
		{
			save_action(FALSE);
			action = TRUE;
			break;
		}
	}

	if (!action)
		save_action(TRUE);

	return;

_ERROR:

	THROW("&1: syntax error in form file", JOB->form);
}

