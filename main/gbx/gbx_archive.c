/***************************************************************************

  archive.c

  The archive management routines

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

#define __GBX_ARCHIVE_C

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"

#include "gbx_list.h"
#include "gbx_string.h"
#include "gbx_stream.h"
#include "gbx_component.h"
#include "gbx_regexp.h"
#include "gbx_exec.h"
#include "gbx_class.h"

#include "gbx_archive.h"

#include "gb_arch_temp.h"

/* main archive project (used only if gbx is run with -x flag) */
PUBLIC ARCHIVE *ARCHIVE_main = NULL;

static ARCHIVE *_archive_list = NULL;

static char *arch_pattern = NULL;
static char *arch_path = NULL;
static int arch_len_path;
static long arch_index = 0;
static ARCH *arch_dir = NULL;


PUBLIC ARCHIVE *ARCHIVE_create(const char *name)
{
  ARCHIVE *arch;

  ALLOC_ZERO(&arch, sizeof(ARCHIVE), "ARCHIVE_create");

  arch->name = name;

  if (name)
    STRING_new(&arch->domain, name, 0);
  else
    STRING_new(&arch->domain, "gb", 0);

  arch->translation_loaded = FALSE;

  TABLE_create(&arch->classes, sizeof(CLASS_SYMBOL), TF_IGNORE_CASE);

  LIST_insert((void **)&_archive_list, arch, &arch->list);

  return arch;
}

static void load_exported_class(ARCHIVE *arch)
{
  /*STREAM stream;*/
  char *buffer;
  long len;
  char *name;

  /* COMPONENT_current is set => it will look in the archive */

	if (!FILE_exist(".list"))
		return;

  STREAM_load(".list", &buffer, &len);
  /* The file must end with a newline !*/
  buffer[len - 1] = 0;

  #if DEBUG_COMP
    fprintf(stderr, ".list file:\n-----------\n\n%s\n----------\n\n", buffer);
  #endif

  name = strtok(buffer, "\n");
  while (name)
  {
    #if DEBUG_COMP
      fprintf(stderr, "Check %s global\n", name);
    #endif

    CLASS_check_global(name);
    name = strtok(NULL, "\n");
  }

  FREE(&buffer, "load_exported_class");


  STREAM_load(".list", &buffer, &len);
  /* The file must end with a newline !*/
  buffer[len - 1] = 0;

  name = strtok(buffer, "\n");
  while (name)
  {
    #if DEBUG_COMP
      fprintf(stderr, "Load %s global\n", name);
    #endif

		len = strlen(name);
		if (len > 2 && name[len - 1] == '?')
		{
			name[len - 1] = 0;
			if (!CLASS_look_global(name, strlen(name)))
				CLASS_load(CLASS_find_global(name));
		}
		else
    	CLASS_load(CLASS_find_global(name));
    name = strtok(NULL, "\n");
  }

  FREE(&buffer, "load_exported_class");
}

static void load_archive(ARCHIVE *arch, const char *path)
{
	if (path)
  	arch->arch = ARCH_open(path);
	else
		arch->arch = NULL;

  //if (arch != ARCHIVE_main)
	load_exported_class(arch);
}


PUBLIC void ARCHIVE_load(ARCHIVE *arch)
{
  char *path = FILE_buffer();

  sprintf(path, ARCH_PATTERN, COMPONENT_path, arch->name);
  if (!FILE_exist(path))
	  sprintf(path, ARCH_PATTERN, COMPONENT_user_path, arch->name);

	load_archive(arch, path);
}


PUBLIC void ARCHIVE_create_main(const char *path)
{
  ARCHIVE_main = ARCHIVE_create(NULL);

	if (path)
  	ARCHIVE_main->arch = ARCH_open(path);
	else
		ARCHIVE_main->arch = NULL;
}


PUBLIC void ARCHIVE_load_main()
{
	load_exported_class(ARCHIVE_main);
}


PUBLIC void ARCHIVE_delete(ARCHIVE *arch)
{
  LIST_remove((void **)&_archive_list, arch, &arch->list);

  if (arch->arch)
    ARCH_close(arch->arch);

  TABLE_delete(&arch->classes);
  STRING_free(&arch->domain);

  FREE(&arch, "ARCHIVE_delete");
}


PUBLIC void ARCHIVE_init(void)
{
  //ARCH_create(path);
  //ARCHIVE_main = _arch_list;
}


PUBLIC void ARCHIVE_exit(void)
{
  if (ARCHIVE_main)
    ARCHIVE_delete(ARCHIVE_main);

  STRING_free(&arch_pattern);
  STRING_free(&arch_path);
}


/* ### *parch must be initialized to NULL or a valid archive */
/* Returns a true archive, never the main archive if we are not running an executable */
static bool get_current(ARCHIVE **parch)
{
  if (*parch)
    return FALSE;

  if (COMPONENT_current && COMPONENT_current->archive)
    *parch = COMPONENT_current->archive;
  else if (CP && CP->component && CP->component->archive)
    *parch = CP->component->archive;
  else
    *parch = EXEC_arch ? ARCHIVE_main : NULL;

  return *parch == NULL;
}

/* Can return the main archive even if we are not running an executable */
PUBLIC bool ARCHIVE_get_current(ARCHIVE **parch)
{
  if (COMPONENT_current && COMPONENT_current->archive)
    *parch = COMPONENT_current->archive;
  else if (CP && CP->component && CP->component->archive)
    *parch = CP->component->archive;
  else
    *parch = ARCHIVE_main;

  return *parch == NULL;
}


PUBLIC bool ARCHIVE_get(ARCHIVE *arch, const char *path, int len_path, ARCHIVE_FIND *find)
{
  ARCH_FIND f;

  if (get_current(&arch))
    return TRUE;

  if (ARCH_find(arch->arch, path, len_path, &f))
    return TRUE;

  find->sym = f.sym;
  find->pos = f.pos;
  find->len = f.len;
  find->arch = arch;

  return FALSE;
}


PUBLIC bool ARCHIVE_exist(ARCHIVE *arch, const char *path)
{
  ARCHIVE_FIND find;

  if (get_current(&arch))
    return FALSE;

  return (!ARCHIVE_get(arch, path, 0, &find));
}


PUBLIC bool ARCHIVE_is_dir(ARCHIVE *arch, const char *path)
{
  ARCHIVE_FIND find;

	if (ARCHIVE_get(arch, path, 0, &find))
		return FALSE;

	return (find.pos < 0);
}


PUBLIC void ARCHIVE_stat(ARCHIVE *arch, const char *path, FILE_STAT *info)
{
  ARCHIVE_FIND find;
  struct stat buf;

  //if (get_current(&arch))
  //  THROW_SYSTEM(ENOENT, path);

  if (ARCHIVE_get(arch, path, 0, &find))
    THROW_SYSTEM(ENOENT, path);

  fstat(find.arch->arch->fd, &buf);

  info->type = find.pos < 0 ? GB_STAT_DIRECTORY : GB_STAT_FILE;
  info->mode = 0400;

  info->size = find.len;
  info->atime = (long)buf.st_mtime;
  info->mtime = (long)buf.st_mtime;
  info->ctime = (long)buf.st_mtime;
  info->hidden = (*FILE_get_name(path) == '.');
  info->uid = buf.st_uid;
  info->gid = buf.st_gid;
}


PUBLIC bool ARCHIVE_read(ARCHIVE *arch, long pos, void *buffer, long len)
{
  return ARCH_read(arch->arch, pos, buffer, len);
}


PUBLIC void ARCHIVE_dir_first(ARCHIVE *arch, const char *path, const char *pattern)
{
	char abs_path[MAX_PATH];

  if (pattern == NULL)
    pattern = "*";

  if (get_current(&arch))
  {
    arch_dir = NULL;
    return;
  }

	ARCH_get_absolute_path(path, strlen(path), abs_path, &arch_len_path);

  if (arch_len_path && abs_path[arch_len_path - 1] != '/')
  {
  	abs_path[arch_len_path] = '/';
  	arch_len_path++;
	}

  STRING_free(&arch_pattern);
  STRING_new(&arch_pattern, pattern, 0);

  STRING_free(&arch_path);
  if (arch_len_path)
  	STRING_new(&arch_path, abs_path, arch_len_path);

  arch_index = 0;
  arch_dir = arch->arch;
}


PUBLIC bool ARCHIVE_dir_next(char **name, long *len, int attr)
{
  SYMBOL *sym;
  char *s = NULL;
  int l = 0;

  /*if (arch_fd < 0)
    return FILE_dir_next(name, len);*/

  if (!arch_dir)
    return TRUE;

  for(;;)
  {
    if (arch_index >= arch_dir->header.n_symbol)
      return TRUE;

    sym = (SYMBOL *)&arch_dir->symbol[arch_index];
    sym = (SYMBOL *)&arch_dir->symbol[sym->sort];

    if (arch_pattern == NULL)
      break;

    arch_index++;

		if (attr == GB_STAT_DIRECTORY && (((ARCH_SYMBOL *)sym)->pos >= 0))
			continue;

		if (sym->len < arch_len_path)
			continue;

		if (strncmp(sym->name, arch_path, arch_len_path))
			continue;

		s = sym->name + arch_len_path;
		l = sym->len - arch_len_path;

		if (l < 0)
			continue;

		if (memchr(s, '/', l))
			continue;

    if (!REGEXP_match(arch_pattern, STRING_length(arch_pattern), s, l))
    	continue;

		break;
	}

  *name = s;
  *len = l;
  return FALSE;
}


PUBLIC bool ARCHIVE_check_addr(char *addr)
{
  ARCHIVE *arch;
  ARCH *a;

  LIST_for_each(arch, _archive_list)
  {
    a = arch->arch;
    if (a && addr >= a->addr && addr < &a->addr[a->length])
      return FALSE;
  }

  return TRUE;
}

