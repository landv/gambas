/***************************************************************************

  gbc_compile.c

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

#define __GBC_COMPILE_C

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_component.h"

#include "gbc_compile.h"
#include "gb_reserved.h"
#include "gbc_read.h"
#include "gbc_trans.h"
#include "gbc_header.h"
#include "gb_code.h"
#include "gbc_output.h"
#include "gbc_form.h"
#include "gbc_chown.h"
#include "gb_arch.h"

/*#define DEBUG*/

char *COMP_root = NULL;
char *COMP_project;
char *COMP_project_name;
char *COMP_info_path;
static char *COMP_classes = NULL;
COMPILE COMP_current;
uint COMPILE_version = GAMBAS_PCODE_VERSION;

const FORM_FAMILY COMP_form_families[] =
{
	{ "form", FORM_NORMAL },
	{ "report", FORM_NORMAL },
	{ "webpage", FORM_WEBPAGE },
	{ NULL }
};

static bool read_line(FILE *f, char *dir, int max)
{
	char *p;
	int c;

	p = dir;

	for(;;)
	{
		max--;

		c = fgetc(f);
		if (c == EOF)
			return TRUE;

		if (c == '\n' || max == 0)
		{
			*p = 0;
			return FALSE;
		}

		*p++ = (char)c;
	}
}

static void add_memory_list(char *p, int size)
{
	char *pe = p + size;
	char *p2;
	int len;
	
	for(;;)
	{
		if (p >= pe)
			break;
		
		p2 = p;
		while (p2 < pe && *p2 != '\n')
			p2++;
		
		if (p2 >= pe)
			break;
		
		len = p2 - p;
		/*if (len > 2 && p[len - 1] == '?')
			len--;*/

		COMPILE_add_class(p, len);
		
		p = p2 + 1;
	}
}

static void add_file_list(FILE *fi)
{
	char line[256];
	int len;

	for(;;)
	{
		if (read_line(fi, line, sizeof(line)))
			break;

		len = strlen(line);
		/*if (len > 2 && line[len - 1] == '?')
			len--;*/

		COMPILE_add_class(line, len);
	}
}

static void add_library_list_file(const char *path)
{
	ARCH *arch;
	ARCH_FIND find;
	
	if (FILE_exist(path))
	{
		arch = ARCH_open(path);
		
		if (!ARCH_find(arch, ".list", 0, &find))
			add_memory_list(&arch->addr[find.pos], find.len);
			
		ARCH_close(arch);
	}
}


static void add_component_list_file(char *name)
{
	char *path;
	FILE *fi;

	path = (char *)FILE_cat(COMP_info_path, name, NULL);
	strcat(path, ".list");
	fi = fopen(path, "r");

	if (!fi)
	{
		// Do not raise an error if a component self-reference is not found
		if (strcmp(name, COMP_project_name))
			//fprintf(stderr, "warning: cannot read component list file: %s.list\n", name);
			THROW("Component not found: &1", name);
		return;
	}
	
	add_file_list(fi);

	fclose(fi);
}


static void startup_print(FILE *fs, const char *key, const char *def)
{
	FILE *fp;
	char line[256];
	int len = strlen(key);
	bool print = FALSE;
	
	fp = fopen(COMP_project, "r");
	if (!fp)
		return;
		
	for(;;)
	{
		if (read_line(fp, line, sizeof(line)))
			break;

		if (strncmp(line, key, len) == 0)
		{
			fprintf(fs, "%s\n", &line[len]);
			print = TRUE;
		}
	}
	
	fclose(fp);
	
	if (!print && def)
		fprintf(fs, "%s\n", def);
}

#undef isdigit

static int read_version_digits(const char **pstr)
{
	const char *p = *pstr;
	int n;
	int i;

	if (!isdigit(*p))
		return -1;

	n = 0;

	for (i = 0; i < 4; i++)
	{
		n = (n << 4) + *p++ - '0';
		if (!isdigit(*p))
			break;
	}

	*pstr = p;
	return n;
}

static void init_version(void)
{
	const char *ver;
	int n, v;

	ver = getenv("GB_PCODE_VERSION");
	if (ver && *ver)
	{
		v = 0;
		n = read_version_digits(&ver);
		if (n <= 0 || n > GAMBAS_VERSION)
			return;

		v = n << 24;

		if (*ver++ != '.')
			return;

		n = read_version_digits(&ver);
		if (n < 0 || n > 0x99)
			return;

		v |= n << 16;

		if (*ver++ == '.')
		{
			n = read_version_digits(&ver);
			if (n > 0)
			{
				if (n > 0x9999)
					return;

				v |= n;
			}
		}

		COMPILE_version = v;
	}
}

void COMPILE_init(void)
{
	const char *root;
	FILE *fp;
	char line[256];
	const char *name;
	FILE *fs;

	RESERVED_init();

	if (COMP_root)
		root = COMP_root;
	else
		root = FILE_get_dir(FILE_get_dir(FILE_find_gambas()));

	// Component directory

	COMP_info_path = STR_copy(FILE_cat(root, "share/gambas" GAMBAS_VERSION_STRING "/info", NULL));
	
	// Project name
	
	COMP_project_name = STR_copy(FILE_get_name(FILE_get_dir(COMP_project)));
	
	// Bytecode version

	init_version();

	// Project classes

	BUFFER_create(&COMP_classes);

	add_component_list_file("gb");

	fp = fopen(COMP_project, "r");
	if (!fp)
		THROW(E_OPEN, COMP_project);
		
	for(;;)
	{
		if (read_line(fp, line, sizeof(line)))
			break;

		/*printf("%s\n", line);*/

		if (strncmp(line, "Component=", 10) == 0)
			add_component_list_file(&line[10]);
		else if (strncmp(line, "Library=", 8) == 0)
			add_library_list_file(&line[8]);
	}

	fclose(fp);

	name = FILE_cat(FILE_get_dir(COMP_project), ".startup", NULL);
	fs = fopen(name, "w");
	if (!fs)
		THROW("Cannot create .startup file");

	startup_print(fs, "Startup=", "");
	startup_print(fs, "Title=", "");
	startup_print(fs, "Stack=", "0");
	startup_print(fs, "StackTrace=", "0");
	startup_print(fs, "Version=", "");
	fputc('\n', fs);
	startup_print(fs, "Component=", NULL);
	startup_print(fs, "Library=", NULL);
	fputc('\n', fs);

	if (fclose(fs))
		THROW("Cannot create .startup file");
		
	FILE_set_owner(name, COMP_project);

	// Adds a separator to make the difference between classes from components 
	// (they must be searched in the global symbol table) and classes from the
	// project (they must be searched in the project symbol table)
	
	COMPILE_add_class("-", 1);

	/*
	dir = opendir(FILE_get_dir(COMP_project));
	if (dir)
	{
		while ((dirent = readdir(dir)) != NULL)
		{
			name = dirent->d_name;
			if (*name == '.')
				continue;

			if ((strcasecmp(FILE_get_ext(name), "module") == 0)
					|| (strcasecmp(FILE_get_ext(name), "class") == 0))
			{
				name = FILE_get_basename(name);
				BUFFER_add(&COMP_classes, name, strlen(name));
				BUFFER_add(&COMP_classes, "\n", 1);
			}
		}

		closedir(dir);
	}

	BUFFER_add(&COMP_classes, "\n", 1);
	*/
}


void COMPILE_begin(const char *file, bool trans, bool debug)
{
	struct stat info;
	off_t size;

	CLEAR(JOB);

	JOB->name = STR_copy(file);
	JOB->debug = debug;
	JOB->form = FORM_get_file_family(JOB->name, &JOB->family);
	JOB->output = OUTPUT_get_file(JOB->name);

	if (trans)
	{
		JOB->trans = TRUE;
		JOB->tname = OUTPUT_get_trans_file(JOB->name);
	}

	BUFFER_create(&JOB->source);
	CLASS_create(&JOB->class);

	JOB->default_library = NO_SYMBOL;

	size = 0;

	if (stat(JOB->name, &info))
		ERROR_warning("cannot stat file: %s", JOB->name);
	else
		size += info.st_size;
	
	if (JOB->form)
	{
		if (stat(JOB->form, &info))
			ERROR_warning("cannot stat file: %s", JOB->form);
		else
			size += info.st_size * 2;
	}

	ALLOC(&JOB->pattern, sizeof(PATTERN) * (16 + size));
	JOB->pattern_count = 0;
}


void COMPILE_load(void)
{
	if (BUFFER_load_file(&JOB->source, JOB->name))
		THROW("Cannot load source file: &1", strerror(errno));

	//BUFFER_add(&JOB->source, "\n", 1);
}


void COMPILE_end(void)
{
	CLASS_delete(&JOB->class);
	BUFFER_delete(&JOB->source);
	FREE(&JOB->pattern);

	if (JOB->help)
		ARRAY_delete(&JOB->help);
	
	STR_free(JOB->name);
	STR_free(JOB->form);
	STR_free(JOB->output);
	if (JOB->trans)
		STR_free(JOB->tname);
	if (JOB->help)
		STR_free(JOB->hname);
}


void COMPILE_exit(void)
{
	RESERVED_exit();
	BUFFER_delete(&COMP_classes);
	STR_free(COMP_project_name);
	STR_free(COMP_project);
	STR_free(COMP_info_path);
	STR_free(COMP_root);
}

void COMPILE_add_class(const char *name, int len)
{
	unsigned char clen = len;
	
	if (clen != len)
		ERROR_panic("Class name is too long");
	
	BUFFER_add(&COMP_classes, &clen, 1);
	BUFFER_add(&COMP_classes, name, len);
}

void COMPILE_end_class(void)
{
	unsigned char clen = 0;
	BUFFER_add(&COMP_classes, &clen, 1);
}

void COMPILE_enum_class(char **name, int *len)
{
	char *p = *name;
	
	if (!p)
		p = COMP_classes;
	else
		p += p[-1];
	
	*len = *p;
	*name = p + 1;
}

void COMPILE_print(int type, int line, const char *msg, ...)
{
	int i;
  va_list args;
	const char *arg[4];
	bool col;

	if (!JOB->warnings && type == MSG_WARNING)
		return;
	
  va_start(args, msg);

	if (line < 0)
	{
		line = JOB->line;
		col = JOB->column;
	}
	else
		col = FALSE;
	
	if (JOB->name)
	{
		const char *name = FILE_get_name(JOB->name);
		if (line)
		{
			if (line > JOB->max_line && JOB->form)
			{
				name = FILE_get_name(JOB->form);
				fprintf(stderr, "%s:%d: ", name, line - FORM_FIRST_LINE + 1);
			}
			else
			{
				if (col)
					fprintf(stderr, "%s:%d:%d: ", name, line, READ_get_column());
				else
					fprintf(stderr, "%s:%d: ", name, line);
			}
		}
		else
			fprintf(stderr, "%s: ", name);
	}
	else
		fprintf(stderr, "gbc: ");
	
	fprintf(stderr, "%s: ", type ? "warning" : "error");
  
	if (msg)
	{
		for (i = 0; i < 4; i++)
			arg[i] = va_arg(args, const char *);

		ERROR_define(msg, arg);
		fputs(ERROR_info.msg, stderr);
		putc('\n', stderr);
	}
	
	va_end(args);
}

