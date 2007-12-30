/***************************************************************************

  archive.c

  The GAMBAS Archiver

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

#include "gbc_archive.h"

/*#define DEBUG*/

PUBLIC char *ARCH_project;
PUBLIC char *ARCH_project_name;
PUBLIC char *ARCH_output = NULL;
PUBLIC bool ARCH_verbose = FALSE;
PUBLIC bool ARCH_swap = FALSE;

static int arch_dir_pos;
static TABLE *arch_table;
static FILE *arch_file = NULL;
static char *arch_buffer;

static long pos_start;

static void write_long(ulong val)
{
 	if (ARCH_swap)
 		SWAP_long((long *)&val);
  if (fwrite(&val, sizeof(ulong), 1, arch_file) < 1)
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
  return ftell(arch_file);
}


static void write_long_at(long pos, ulong val)
{
  long old_pos = get_pos();

  fseek(arch_file, pos, SEEK_SET);
  write_long(val);
  fseek(arch_file, old_pos, SEEK_SET);
}


static void write_string(const char *str, int len)
{
  if (fwrite(str, sizeof(char), len, arch_file) < len)
    THROW("Write error");
}


static void make_executable(void)
{
  struct stat info;

  if (stat(ARCH_output, &info) == 0)
    if (chmod(ARCH_output, info.st_mode | S_IXUSR | S_IXGRP | S_IXOTH) == 0)
      return;

  THROW("Cannot make executable: &1", strerror(errno));
}


PUBLIC void ARCH_define_output(const char *path)
{
	STR_free(ARCH_output);
  ARCH_output = STR_copy(path);
}

PUBLIC void ARCH_define_project(const char *project)
{
  char *name;
  char *dir;

  if (project == NULL)
    project = FILE_get_current_dir();

  chdir(project);
  dir = STR_copy(FILE_get_current_dir());

  arch_dir_pos = strlen(dir) + 1;

  ARCH_project = STR_copy(FILE_cat(dir, ".project", NULL));

  name = STR_copy(FILE_get_name(dir));

  /*ARCH_project_name = STR_copy(FILE_set_ext(name, NULL));*/
  ARCH_project_name = STR_copy(name);

	if (!ARCH_output)
  	ARCH_define_output(strcat((char *)FILE_cat(dir, ARCH_project_name, NULL), ".gambas"));

  STR_free(name);
  STR_free(dir);
}


PUBLIC void ARCH_init(void)
{
  TABLE_create(&arch_table, sizeof(ARCH_SYMBOL), TF_NORMAL);

	ALLOC(&arch_buffer, 4096, "ARCH_init");

  arch_file = fopen(ARCH_output, "w");
  if (arch_file == NULL)
    THROW("Cannot create temporary archive file: &1", ARCH_output);

  fputs("#! /usr/bin/env gbr" GAMBAS_VERSION_STRING "\n", arch_file);

  while (get_pos() < 31)
    fprintf(arch_file, " ");
  fprintf(arch_file, "\n");

  write_long(ARCH_MAGIC);
  write_long(ARCH_VERSION);
  
  if (ARCH_verbose)
  	printf("Format version: %d\n", ARCH_VERSION);

  pos_start = get_pos();
  write_long(0);
  write_long(0);
  write_long(0);
  write_long(0);

  write_long_at(pos_start, get_pos());
}


#if ARCH_VERSION == 2
static void compress_file_name(const char *src, long lsrc, char **dst, long *ldst)
{
  char *p;
  char tpath[MAX_PATH];
  char tpath2[MAX_PATH];
  long len;
  long ind;
  ARCH_SYMBOL *sym;

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
			
	  if (!TABLE_find_symbol(arch_table, tpath, p - tpath, (SYMBOL **)(void *)&sym, &ind))
	  {
	  	*p = 0;
	  	THROW("&1: not in archive!\n", tpath);
		}
	
		len = sprintf(tpath2, "/%ld:%s", ind, p + 1);
		strcpy(tpath, tpath2);
	}

	if (ARCH_verbose)
		printf(" -> %s\n", tpath); 	
		
  *dst = tpath;
  *ldst = len;
}
#endif

PUBLIC void ARCH_exit(void)
{
  int i;
  ARCH_SYMBOL *sym;
  long pos_str;

  /* Write strings */

  write_long_at(pos_start + sizeof(long), get_pos());

  pos_str = 0;

  for (i = 0; i < TABLE_count(arch_table); i++)
  {
    sym = (ARCH_SYMBOL *)TABLE_get_symbol(arch_table, i);
    write_string(sym->symbol.name, sym->symbol.len);
  }

  /* Write file names */

  write_long_at(pos_start + sizeof(long) * 2, get_pos());

  write_long_at(pos_start + sizeof(long) * 3, TABLE_count(arch_table));

  for (i = 0; i < TABLE_count(arch_table); i++)
  {
    sym = (ARCH_SYMBOL *)TABLE_get_symbol(arch_table, i);
    //write_short((ushort)i);
    write_short(sym->symbol.sort);
    write_short(sym->symbol.len);
    write_long(pos_str);
    write_long(sym->pos);
    write_long(sym->len);

    pos_str += sym->symbol.len;
  }

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
  
  FREE(&arch_buffer, "ARCH_exit");
}


PUBLIC long ARCH_add_file(const char *path)
{
  char *rel_path;
  ARCH_SYMBOL *sym;
  FILE *file;
  struct stat info;
  long len, len_read;
  
  long ind;
  
  #if ARCH_VERSION == 2
  compress_file_name(&path[arch_dir_pos], strlen(&path[arch_dir_pos]), &rel_path, &len);
  rel_path = STR_copy(rel_path);
  #else
  rel_path = STR_copy(&path[arch_dir_pos]);
  len = strlen(rel_path);
  #endif
  
  TABLE_add_symbol(arch_table, rel_path, len, (SYMBOL **)(void *)&sym, &ind);
  sym->pos = get_pos();

  file = fopen(path, "r");
  if (file == NULL)
    THROW("Cannot open file '&1'", path);

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
			len_read = fread(arch_buffer, 1, sizeof(arch_buffer), file);
			if (len_read > 0)
				fwrite(arch_buffer, 1, len_read, arch_file);

			if (len_read < sizeof(arch_buffer))
			{
				if (ferror(file))
					THROW("Cannot read file '&1': &2", path, strerror(errno));
				else
					break;
			}
		}

		fclose(file);

		if (ARCH_verbose)
			printf("Adding file %s (%ld bytes)", rel_path, sym->len);
	}
	
	if (ARCH_verbose)
		printf(" -> %ld\n", ind);
	
	return ind;
}
