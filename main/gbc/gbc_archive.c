/***************************************************************************

	gbc_archive.c

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

#define __GBC_ARCHIVE_C

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_magic.h"
#include "gb_common_swap.h"
#include "gbc_chown.h"
#include "gbc_archive.h"

/*#define DEBUG*/

#define TEMP_EXEC ".temp.gambas"

char *ARCH_project;
char *ARCH_project_name;
char *ARCH_output = NULL;
bool ARCH_verbose = FALSE;
bool ARCH_swap = FALSE;

static int arch_dir_pos;
static TABLE *arch_table;
static FILE *arch_file = NULL;

#define ARCH_BUFFER_SIZE 4096
static char *arch_buffer;

static int pos_start;

static void write_int(uint val)
{
	if (ARCH_swap)
		SWAP_int((int *)&val);
	if (fwrite(&val, sizeof(uint), 1, arch_file) < 1)
		THROW("Write error");
}


static void write_short(ushort val)
{
	if (ARCH_swap)
		SWAP_short((short *)&val);
	if (fwrite(&val, sizeof(ushort), 1, arch_file) < 1)
		THROW("Write error");
}


static long get_pos(void)
{
	long pos = ftell(arch_file);
	if (pos < 0)
		THROW("Unable to get file position");
	return (int)pos; // No archive file greater then 2 Go!
}


static void write_int_at(int pos, uint val)
{
	int old_pos = get_pos();

	fseek(arch_file, pos, SEEK_SET);
	write_int(val);
	fseek(arch_file, old_pos, SEEK_SET);
}


static void write_string(const char *str, int len)
{
	if (fwrite(str, sizeof(char), len, arch_file) < len)
		THROW("Write error");
}


static void make_executable(void)
{
	const char *err;
	struct stat info;

	FILE_chdir(FILE_get_dir(ARCH_output));
	
	// If we cannot make the archive executable, just print a warning. gbs creates an executable cache 
	// inside /tmp, and /tmp may be mounted with the "noexec" flag.
	
	if (stat(TEMP_EXEC, &info) || chmod(TEMP_EXEC, info.st_mode | S_IXUSR | S_IXGRP | S_IXOTH))
		fprintf(stderr, "gba: warning: cannot change executable permissions\n");

	FILE_set_owner(TEMP_EXEC, FILE_cat(FILE_get_dir(ARCH_project), ".project", NULL));

	if (FILE_exist(ARCH_output) && unlink(ARCH_output))
	{
		err = "Cannot remove previous executable";
		goto __ERROR;
	}
	
	if (rename(TEMP_EXEC, ARCH_output))
	{
		err = "Cannot create executable";
		goto __ERROR;
	}
	
	return;

__ERROR:

	THROW("Cannot make executable: &1: &2", err, strerror(errno));
}


void ARCH_define_output(const char *path)
{
	STR_free(ARCH_output);
	
	if (path && *path != '/')
		path = FILE_cat(FILE_get_current_dir(), path, NULL);
	else if (!path)
		path = "";
	
	ARCH_output = STR_copy(path);
}

void ARCH_define_project(const char *project)
{
	char *name;
	char *dir;
	const char *path;

	if (project == NULL)
		project = FILE_get_current_dir();

	FILE_chdir(project);
	dir = STR_copy(FILE_get_current_dir());

	arch_dir_pos = strlen(dir) + 1;

	path = FILE_cat(dir, ".startup", NULL);
	if (FILE_exist(path))
		ARCH_project = STR_copy(path);
	else
		ARCH_project = STR_copy(FILE_cat(dir, ".project", NULL));

	name = STR_copy(FILE_get_name(dir));

	/*ARCH_project_name = STR_copy(FILE_set_ext(name, NULL));*/
	ARCH_project_name = STR_copy(name);

	if (!ARCH_output)
		ARCH_define_output(strcat((char *)FILE_cat(dir, ARCH_project_name, NULL), ".gambas"));

	STR_free(name);
	STR_free(dir);
}


void ARCH_init(void)
{
	TABLE_create(&arch_table, sizeof(ARCH_SYMBOL), TF_NORMAL);

	ALLOC(&arch_buffer, ARCH_BUFFER_SIZE);

	arch_file = fopen(FILE_cat(FILE_get_dir(ARCH_output), TEMP_EXEC, NULL), "w");
	if (arch_file == NULL)
		THROW("Cannot create temporary archive file: &1", ARCH_output);

	fputs("#! /usr/bin/env gbr" GAMBAS_VERSION_STRING "\n", arch_file);

	while (get_pos() < 31)
		fprintf(arch_file, " ");
	fprintf(arch_file, "\n");

	write_int(ARCH_MAGIC);
	write_int(ARCH_VERSION);
	
	if (ARCH_verbose)
		printf("Format version: %d\n", ARCH_VERSION);

	pos_start = get_pos();
	write_int(0);
	write_int(0);
	write_int(0);
	write_int(0);

	write_int_at(pos_start, get_pos());
}


#if ARCH_VERSION == 2
static void compress_file_name(const char *src, int lsrc, char **dst, int *ldst)
{
	char *p;
	static char tpath[PATH_MAX];
	char tpath2[PATH_MAX];
	int len;
	int ind;

	strncpy(tpath, src, lsrc);
	tpath[lsrc] = 0;
	len = lsrc;
	
	if (ARCH_verbose)
		printf("%s", tpath); 	
	
	for(;;)
	{
		p = index(tpath + 1, '/');
		if (!p)
			break;
			
		if (!TABLE_find_symbol(arch_table, tpath, p - tpath, &ind))
		{
			*p = 0;
			THROW("&1: not in archive", tpath);
		}
	
		len = snprintf(tpath2, sizeof(tpath2), "/%d:%s", ind, p + 1);
		strcpy(tpath, tpath2);
	}

	if (ARCH_verbose)
		printf(" -> %s\n", tpath); 	
		
	*dst = tpath;
	*ldst = len;
}
#endif

void ARCH_exit(void)
{
	int i;
	ARCH_SYMBOL *sym;
	int pos_str;

	/* Write strings */

	write_int_at(pos_start + sizeof(int), get_pos());

	pos_str = 0;

	for (i = 0; i < TABLE_count(arch_table); i++)
	{
		sym = (ARCH_SYMBOL *)TABLE_get_symbol(arch_table, i);
		write_string(sym->sym.name, sym->sym.len);
	}

	/* Write file names */

	write_int_at(pos_start + sizeof(int) * 2, get_pos());

	write_int_at(pos_start + sizeof(int) * 3, TABLE_count(arch_table));

	for (i = 0; i < TABLE_count(arch_table); i++)
	{
		sym = (ARCH_SYMBOL *)TABLE_get_symbol(arch_table, i);
		//write_short((ushort)i);
		write_int(pos_str);
		write_int(sym->sym.len);
		write_int(sym->pos);
		write_int(sym->len);

		pos_str += sym->sym.len;
	}

	for (i = 0; i < TABLE_count(arch_table); i++)
		write_short(arch_table->sort[i]);
	
	/* Close file */

	fclose(arch_file);

	make_executable();

	/* Free everything */

	for (i = 0; i < TABLE_count(arch_table); i++)
		STR_free(TABLE_get_symbol(arch_table, i)->name);

	TABLE_delete(&arch_table);

	STR_free(ARCH_output);
	STR_free(ARCH_project);
	STR_free(ARCH_project_name);
	
	FREE(&arch_buffer);
}


int ARCH_add_file(const char *path)
{
	char *rel_path;
	ARCH_SYMBOL *sym;
	FILE *file;
	struct stat info;
	int len, len_read;
	
	int ind;
	
	#if ARCH_VERSION == 2
	compress_file_name(&path[arch_dir_pos], strlen(&path[arch_dir_pos]), &rel_path, &len);
	rel_path = STR_copy(rel_path);
	#else
	rel_path = STR_copy(&path[arch_dir_pos]);
	len = strlen(rel_path);
	#endif
	
	TABLE_add_symbol(arch_table, rel_path, len, &ind);
	sym = (ARCH_SYMBOL *)TABLE_get_symbol(arch_table, ind);
	sym->pos = get_pos();

	file = fopen(path, "r");
	if (file == NULL)
		THROW("Cannot open file: &1", path);

	fstat(fileno(file), &info);

	if (S_ISDIR(info.st_mode))
	{
		sym->pos = -1;
		sym->len = 0;
		fclose(file);
		if (ARCH_verbose)
			printf("Adding directory %s", rel_path);
	}
	else
	{
		sym->len = info.st_size;

		len = sym->len;
		while (len > 0)
		{
			len_read = fread(arch_buffer, 1, ARCH_BUFFER_SIZE, file);
			if (len_read > 0)
				fwrite(arch_buffer, 1, len_read, arch_file);

			if (len_read < ARCH_BUFFER_SIZE)
			{
				if (ferror(file))
					THROW("Cannot read file: &1: &2", path, strerror(errno));
				else
					break;
			}
		}

		fclose(file);

		if (ARCH_verbose)
			printf("Adding file %s (%d bytes)", rel_path, sym->len);
	}
	
	if (ARCH_verbose)
		printf(" -> %d\n", ind);
	
	return ind;
}

#if 0
void ARCH_browse(ARCH *a, void (*found)(const char *path, int64_t size))
{
	int i;
	ARCH_SYMBOL *asym;
	SYMBOL *sym;
	char *path;
	char *temp;
	int size;
	int ip;
	
	for (i = 0; i < a->header.n_symbol; i++)
	{
		asym = &a->symbol[i];
		sym = &asym->sym;
		
		size = asym->len;
		
		path = STR_copy_len(sym->name, sym->len);
		for(;;)
		{
			if (*path != '/')
				break;
			
			ip = atoi(&path[1]);
			sym = &a->symbol[ip].sym;
			
			temp = path;
			path = STR_copy_len(sym->name, sym->len);
			if (path[sym->len - 1] != '/')
				STRING_add(&path, "/", 1);
			STRING_add(&path, strchr(temp, ':') + 1, 0);
			STRING_free(&temp);
		}
		
		(*found)(path, size);
		STR_free(path);
	}
}
#endif