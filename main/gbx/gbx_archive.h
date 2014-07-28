/***************************************************************************

  gbx_archive.h

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

#ifndef __GBX_ARCHIVE_H
#define __GBX_ARCHIVE_H

#define PROJECT_EXEC

#include "gb_table.h"
#include "gb_file.h"
#include "gb_arch.h"
#include "gb_list.h"

// ARCHIVE_load_exported_class() pass argument
#define AR_FIND_ONLY      1
#define AR_LOAD_ONLY      2
#define AR_FIND_AND_LOAD  3

typedef
  struct {
    LIST list;
    ARCH *arch;
    const char *name;
    char *domain;
    TABLE *classes;
		const char *path;
		void *current_component;
		struct _CLASS **exported;
    unsigned translation_loaded : 1;
		unsigned exported_classes_loaded : 1;
    }
  ARCHIVE;

typedef
  struct {
    ARCHIVE *arch;
    ARCH_SYMBOL *sym;
    int index;
    int pos;
    int len;
    }
  ARCHIVE_FIND;

#ifndef __ARCHIVE_C
EXTERN ARCHIVE *ARCHIVE_main;
#endif

void ARCHIVE_init(void);
void ARCHIVE_exit(void);

void ARCHIVE_create_main(const char *path);
void ARCHIVE_load_main(void);

ARCHIVE *ARCHIVE_create(const char *name, const char *path);
void ARCHIVE_delete(ARCHIVE *arch);
void ARCHIVE_load(ARCHIVE *arch, bool load_exp);
void ARCHIVE_load_exported_class(ARCHIVE *arch, int pass);

bool ARCHIVE_get(ARCHIVE *arch, const char **ppath, ARCHIVE_FIND *find);

bool ARCHIVE_read(ARCHIVE *arch, int pos, void *buffer, int len);

bool ARCHIVE_exist(ARCHIVE *arch, const char *path);
void ARCHIVE_stat(ARCHIVE *arch, const char *path, FILE_STAT *info);
bool ARCHIVE_is_dir(ARCHIVE *arch, const char *path);

void ARCHIVE_dir_first(ARCHIVE *arch, const char *path, const char *pattern, int attr);
bool ARCHIVE_dir_next(char **name, int *len, int attr);

bool ARCHIVE_get_current(ARCHIVE **parch);

bool ARCHIVE_check_addr(char *addr);

void ARCHIVE_browse(ARCHIVE *arch, void (*found)(const char *path, int64_t size));

#endif
