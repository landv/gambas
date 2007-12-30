/***************************************************************************

  archive.h

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

#ifndef __GBX_ARCHIVE_H
#define __GBX_ARCHIVE_H

#define PROJECT_EXEC

#include "gb_table.h"
#include "gb_file.h"
#include "gb_arch.h"

#include "gbx_list.h"

typedef
  struct {
    LIST list;
    ARCH *arch;
    const char *name;
    char *domain;
    TABLE *classes;
    int translation_loaded;
    }
  ARCHIVE;

typedef
  struct {
    ARCHIVE *arch;
    ARCH_SYMBOL *sym;
    long pos;
    long len;
    }
  ARCHIVE_FIND;

#ifndef __ARCHIVE_C
EXTERN ARCHIVE *ARCHIVE_main;
#endif

PUBLIC void ARCHIVE_init(void);
PUBLIC void ARCHIVE_exit(void);

PUBLIC void ARCHIVE_create_main(const char *path);
PUBLIC void ARCHIVE_load_main(void);

PUBLIC ARCHIVE *ARCHIVE_create(const char *name);
PUBLIC void ARCHIVE_delete(ARCHIVE *arch);
PUBLIC void ARCHIVE_load(ARCHIVE *arch);

PUBLIC bool ARCHIVE_get(ARCHIVE *arch, const char *path, int len_path, ARCHIVE_FIND *find);

PUBLIC bool ARCHIVE_read(ARCHIVE *arch, long pos, void *buffer, long len);

PUBLIC bool ARCHIVE_exist(ARCHIVE *arch, const char *path);
PUBLIC void ARCHIVE_stat(ARCHIVE *arch, const char *path, FILE_STAT *info);
PUBLIC bool ARCHIVE_is_dir(ARCHIVE *arch, const char *path);

PUBLIC void ARCHIVE_dir_first(ARCHIVE *arch, const char *path, const char *pattern);
PUBLIC bool ARCHIVE_dir_next(char **name, long *len, int attr);

PUBLIC bool ARCHIVE_get_current(ARCHIVE **parch);

PUBLIC bool ARCHIVE_check_addr(char *addr);

#endif
