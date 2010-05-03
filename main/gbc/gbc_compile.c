/***************************************************************************

	gbc_compile.c

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
char *COMP_classes = NULL;

COMPILE COMP_current;

const char *COMP_form_families[] = { "form", "report", NULL };

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
		if (len > 2 && p[len - 1] == '?')
			len--;

		BUFFER_add(&COMP_classes, p, len);
		BUFFER_add(&COMP_classes, "\n", 1);
		
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
		if (len > 2 && line[len - 1] == '?')
			len--;

		BUFFER_add(&COMP_classes, line, len);
		BUFFER_add(&COMP_classes, "\n", 1);
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


void COMPILE_init(void)
{
	const char *root;
	FILE *fp;
	char line[256];
	//DIR *dir;
	//struct dirent *dirent;
	const char *name;
	//struct passwd *info;
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
	
	BUFFER_add(&COMP_classes, "-\n", 2);

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


void COMPILE_begin(const char *file, bool trans)
{
	struct stat info;
	off_t size;

	CLEAR(JOB);

	JOB->name = STR_copy(file);
	JOB->form = FORM_get_file(JOB->name);
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
		fprintf(stderr, "gbc: Warning: Cannot stat file: %s\n", JOB->name);
	else
		size += info.st_size;
	
	if (JOB->form)
	{
		if (stat(JOB->form, &info))
			fprintf(stderr, "gbc: Warning: Cannot stat file: %s\n", JOB->form);
		else
			size += info.st_size * 2;
	}

	ALLOC(&JOB->pattern, sizeof(PATTERN) * (16 + size), "COMPILE_begin");
	JOB->pattern_count = 0;
}


void COMPILE_load(void)
{
	BUFFER_load_file(&JOB->source, JOB->name);
	/*if (JOB->source[BUFFER_length(JOB->source) - 1] != '\n')
		BUFFER_add(&JOB->source, "\n", 1);*/
	BUFFER_add(&JOB->source, "\n", 1);
}


void COMPILE_end(void)
{
	CLASS_delete(&JOB->class);
	BUFFER_delete(&JOB->source);
	FREE(&JOB->pattern, "COMPILE_end");

	STR_free(JOB->name);
	STR_free(JOB->form);
	STR_free(JOB->output);
	if (JOB->trans)
		STR_free(JOB->tname);
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

