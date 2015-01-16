/***************************************************************************

	gbx_archive.c

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

#define __GBX_ARCHIVE_C

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"

#include "gbx_string.h"
#include "gbx_stream.h"
#include "gbx_component.h"
#include "gbx_regexp.h"
#include "gbx_exec.h"
#include "gbx_class.h"
#include "gbx_project.h"

#include "gbx_archive.h"

#include "gb_arch_temp.h"

/* main archive project (used only if gbx is run with -x flag) */
ARCHIVE *ARCHIVE_main = NULL;

static ARCHIVE *_archive_list = NULL;

static char *arch_pattern = NULL;
static int arch_index = 0;
static ARCH *arch_dir = NULL;
static char *arch_prefix = NULL;

ARCHIVE *ARCHIVE_create(const char *name, const char *path)
{
	ARCHIVE *arch;

	ALLOC_ZERO(&arch, sizeof(ARCHIVE));

	arch->name = name;
	arch->path = path;

	arch->domain = STRING_new_zero(name ? name : "gb");

	arch->translation_loaded = FALSE;

	TABLE_create(&arch->classes, sizeof(CLASS_SYMBOL), TF_IGNORE_CASE);

	LIST_insert(&_archive_list, arch, &arch->list);

	return arch;
}

void ARCHIVE_load_exported_class(ARCHIVE *arch, int pass)
{
	char *buffer;
	int len;
	char *name;
	CLASS *class;
	int i;
	COMPONENT *current;
	bool optional;
	char c;

	if (arch->exported_classes_loaded)
		return;

	current = COMPONENT_current;

	COMPONENT_current = (COMPONENT *)arch->current_component;

	/* COMPONENT_current is set => it will look in the archive */

	#if DEBUG_COMP
		fprintf(stderr, "load_exported_class: %s (component: %s)\n", arch->name, COMPONENT_current ? COMPONENT_current->name : "?");
	#endif

	if (!FILE_exist(".list"))
		return;

	if (pass & AR_FIND_ONLY)
	{
		#if DEBUG_COMP
			fprintf(stderr, "<Find pass>\n");
		#endif

		STREAM_load(".list", &buffer, &len);
		/* The file must end with a newline !*/
		buffer[len - 1] = 0;

		#if DEBUG_COMP
			fprintf(stderr, "-----------\n%s\n----------\n\n", buffer);
		#endif

		ARRAY_create(&arch->exported);

		name = strtok(buffer, "\n");
		while (name)
		{
			#if DEBUG_COMP
				fprintf(stderr, "Check %s global\n", name);
			#endif

			len = strlen(name);
			optional = FALSE;

			for(;;)
			{
				c = name[len - 1];
				if (c == '?')
				{
					optional = TRUE;
					len--;
				}
				else if (c == '!')
					len--;
				else
					break;
			}

			name[len] = 0;

			/*
			class = CLASS_look_global(name, strlen(name));

			if (class)
			{
				#if DEBUG_COMP
					fprintf(stderr, "...override!\n");
				#endif
				CLASS_load(class);
				CLASS_check_global(class);
			}
			else
				class = CLASS_find_global(name);*/

			class = CLASS_find_global(name);
			CLASS_check_global(class);

			if (optional)
			{
				if (CLASS_look_global(name, len))
					class = NULL;
			}

			if (class)
			{
				#if DEBUG_COMP
					fprintf(stderr, "Add to load: %p %s\n", class, name);
				#endif
				class->component = COMPONENT_current;
				*((CLASS **)ARRAY_add(&arch->exported)) = class;
			}

			name = strtok(NULL, "\n");
		}

		FREE(&buffer);
	}

	if (pass & AR_FIND_ONLY)
	{
		#if DEBUG_COMP
			fprintf(stderr, "<Load pass>\n");
		#endif

		for (i = 0; i < ARRAY_count(arch->exported); i++)
		{
			#if DEBUG_COMP
				fprintf(stderr, "load %p %s\n", arch->exported[i], arch->exported[i]->name);
			#endif
			CLASS_load(arch->exported[i]);
		}

		ARRAY_delete(&arch->exported);
		arch->exported_classes_loaded = TRUE;
	}

	COMPONENT_current = current;
}

#if 0
static void load_component(char *name)
{
	COMPONENT *comp;

	comp = COMPONENT_create(name);
	COMPONENT_load(comp);
}
#endif

/*static void load_dependencies(ARCHIVE *arch)
{
	char *buffer;
	int len;

	STREAM_load(".startup", &buffer, &len);
	PROJECT_analyze_startup(buffer, len, load_component);
	FREE(&buffer, "load_dependencies");
}*/

static char *exist_library(const char *dir, const char *name)
{
	char *path;

	path = (char *)FILE_cat(dir, name, NULL);
	if (FILE_exist(path))
		return path;
	else
		return NULL;
}

void ARCHIVE_load(ARCHIVE *arch, bool load_exp)
{
	char *path;

	if (arch->path)
	{
		if (EXEC_debug)
			path = (char *)arch->path;
		else
		{
			path = exist_library(PROJECT_path, arch->name);
			if (!path) path = exist_library("/usr/bin", arch->name);
			if (!path) path = exist_library("/bin", arch->name);
		}

		if (!path || !FILE_exist(path))
			THROW(E_LIBRARY, arch->name, "cannot find library");
	}
	else
	{
		path = FILE_buffer();
		sprintf(path, ARCH_PATTERN, COMPONENT_path, arch->name);
	}

	arch->arch = ARCH_open(path);
	arch->current_component = COMPONENT_current;

	if (load_exp)
		ARCHIVE_load_exported_class(arch, AR_FIND_AND_LOAD); //, dep);
}


void ARCHIVE_create_main(const char *path)
{
	ARCHIVE_main = ARCHIVE_create(NULL, NULL);

	if (path)
		ARCHIVE_main->arch = ARCH_open(path);
	else
		ARCHIVE_main->arch = NULL;
}


void ARCHIVE_load_main()
{
	ARCHIVE_load_exported_class(ARCHIVE_main, AR_FIND_AND_LOAD);
}


void ARCHIVE_delete(ARCHIVE *arch)
{
	LIST_remove(&_archive_list, arch, &arch->list);

	if (arch->arch)
		ARCH_close(arch->arch);

	TABLE_delete(&arch->classes);
	STRING_free(&arch->domain);

	FREE(&arch);
}


void ARCHIVE_init(void)
{
	//ARCH_create(path);
	//ARCHIVE_main = _arch_list;
}


void ARCHIVE_exit(void)
{
	if (ARCHIVE_main)
		ARCHIVE_delete(ARCHIVE_main);

	STRING_free(&arch_pattern);
	STRING_free(&arch_prefix);
}


/* ### *parch must be initialized to NULL or a valid archive */
/* Returns a true archive, never the main archive if we are not running an executable */
static bool get_current(ARCHIVE **parch, const char **ppath)
{
	if (*parch)
		return FALSE;

	if (strncmp(*ppath, "../", 3) == 0)
	{
		*parch = EXEC_arch ? ARCHIVE_main : NULL;
		*ppath += 3;
	}
	else if (COMPONENT_current && COMPONENT_current->archive)
		*parch = COMPONENT_current->archive;
	else if (CP && CP->component && CP->component->archive)
		*parch = CP->component->archive;
	else
		*parch = EXEC_arch ? ARCHIVE_main : NULL;

	return *parch == NULL;
}

/* Can return the main archive even if we are not running an executable */
bool ARCHIVE_get_current(ARCHIVE **parch)
{
	if (COMPONENT_current && COMPONENT_current->archive)
		*parch = COMPONENT_current->archive;
	else if (CP && CP->component && CP->component->archive)
		*parch = CP->component->archive;
	else
		*parch = ARCHIVE_main;

	return *parch == NULL;
}


bool ARCHIVE_get(ARCHIVE *arch, const char **ppath, ARCHIVE_FIND *find)
{
	ARCH_FIND f;
	struct stat buf;

	if (!*ppath || **ppath == 0)
		return TRUE;

	if (get_current(&arch, ppath))
	{
		// no archive found, we try a lstat
		if (!PROJECT_path)
			return TRUE;
		FILE_chdir(PROJECT_path);
		if (stat(*ppath, &buf))
			return TRUE;

		find->pos = S_ISDIR(buf.st_mode) ? -1 : 0;
		find->sym = NULL;
		find->len = (int)buf.st_size;
		find->index = -1;
		find->arch = NULL;
		return FALSE;
	}

	if (ARCH_find(arch->arch, *ppath, 0, &f))
		return TRUE;

	find->sym = f.sym;
	find->pos = f.pos;
	find->len = f.len;
	find->index = f.index;
	find->arch = arch;

	return FALSE;
}


bool ARCHIVE_exist(ARCHIVE *arch, const char *path)
{
	ARCHIVE_FIND find;

	//if (get_current(&arch, &path))
	//  return FALSE;

	return (!ARCHIVE_get(arch, &path, &find));
}


bool ARCHIVE_is_dir(ARCHIVE *arch, const char *path)
{
	ARCHIVE_FIND find;

	if (ARCHIVE_get(arch, &path, &find))
		return FALSE;

	return (find.pos < 0);
}


void ARCHIVE_stat(ARCHIVE *arch, const char *path, FILE_STAT *info)
{
	ARCHIVE_FIND find;
	struct stat buf;

	//if (get_current(&arch))
	//  THROW_SYSTEM(ENOENT, path);

	if (ARCHIVE_get(arch, &path, &find))
		THROW_SYSTEM(ENOENT, path);

	if (find.arch)
		fstat(find.arch->arch->fd, &buf);
	else
		stat(PROJECT_path, &buf);

	info->type = find.pos < 0 ? GB_STAT_DIRECTORY : GB_STAT_FILE;
	info->mode = 0400;

	info->size = find.len;
	info->atime = (int)buf.st_mtime;
	info->mtime = (int)buf.st_mtime;
	info->ctime = (int)buf.st_mtime;
	info->hidden = (*FILE_get_name(path) == '.');
	info->uid = (int)buf.st_uid;
	info->gid = (int)buf.st_gid;
}


bool ARCHIVE_read(ARCHIVE *arch, int pos, void *buffer, int len)
{
	return ARCH_read(arch->arch, pos, buffer, len);
}


void ARCHIVE_dir_first(ARCHIVE *arch, const char *path, const char *pattern, int attr)
{
	ARCHIVE_FIND find;
	char abs_path[16];
	int abs_len;

	/*if (get_current(&arch, &path))
	{
		arch_dir = NULL;
		return;
	}*/

	if (ARCHIVE_get(arch, &path, &find))
	{
		arch_dir = NULL;
		return;
	}

	// ?? "." means that we want to browse the archive.
	if (!find.sym && !find.arch) // && !(path[0] == '.' && path[1] == 0))
	{
		// By calling FILE_dir_first() again with an absolute path, we are sure that next calls to
		// FILE_dir_next() will never call ARCHIVE_dir_next().
		FILE_dir_first(FILE_cat(PROJECT_path, path, NULL), pattern, attr);
		return;
	}

	if (pattern == NULL)
		pattern = "*";

	arch = find.arch;
	arch_dir = arch->arch;
	arch_index = 0;

	STRING_free(&arch_pattern);
	arch_pattern = STRING_new_zero(pattern);

	//if (arch_dir->header.version == 2)
	//{
		if (find.index >= 0)
			abs_len = sprintf(abs_path, "/%d:", find.index);
		else
			abs_len = 0;
	/*}
	else
	{
		ARCH_get_absolute_path(path, strlen(path), abs_path, &abs_len);

		if (abs_len && abs_path[abs_len - 1] != '/')
		{
			abs_path[abs_len] = '/';
			abs_len++;
		}
	}*/

	STRING_free(&arch_prefix);
	if (abs_len)
		arch_prefix = STRING_new(abs_path, abs_len);
}


bool ARCHIVE_dir_next(char **name, int *len, int attr)
{
	ARCH_SYMBOL *asym;
	SYMBOL *sym;
	char *s = NULL;
	int l = 0;
	int lenp;

	/*if (arch_fd < 0)
		return FILE_dir_next(name, len);*/

	if (!arch_dir)
		return TRUE;

	lenp = STRING_length(arch_prefix);

	for(;;)
	{
		if (arch_index >= arch_dir->header.n_symbol)
			return TRUE;

		//sym = &arch_dir->symbol[arch_index].sym;
		asym = &arch_dir->symbol[arch_dir->sort[arch_index]];
		sym = &asym->sym;

		if (arch_pattern == NULL)
			break;

		arch_index++;

		if (attr == GB_STAT_DIRECTORY && (asym->pos >= 0))
			continue;

		if (sym->len < lenp)
			continue;

		if (lenp)
		{
			if (strncmp(sym->name, arch_prefix, lenp))
				continue;
		}
		else // special case: root directory when header.version == 2
		{
			if (!strncmp(sym->name, "/", 1))
				continue;
		}

		s = sym->name + lenp;
		l = sym->len - lenp;

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


bool ARCHIVE_check_addr(char *addr)
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

void ARCHIVE_browse(ARCHIVE *arch, void (*found)(const char *path, int64_t size))
{
	int i;
	ARCH_SYMBOL *asym;
	SYMBOL *sym;
	ARCH *a = arch->arch;
	char *path;
	char *temp;
	int size;
	int ip;

	for (i = 0; i < a->header.n_symbol; i++)
	{
		asym = &a->symbol[i];
		sym = &asym->sym;

		size = asym->len;

		path = STRING_new(sym->name, sym->len);
		for(;;)
		{
			if (*path != '/')
				break;

			ip = atoi(&path[1]);
			sym = &a->symbol[ip].sym;

			temp = path;
			path = STRING_new(sym->name, sym->len);
			if (path[sym->len - 1] != '/')
				path = STRING_add(path, "/", 1);
			path = STRING_add(path, strchr(temp, ':') + 1, 0);
			STRING_free(&temp);
		}

		(*found)(path, size);
		STRING_free(&path);
	}
}
