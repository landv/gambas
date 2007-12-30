/***************************************************************************

  gb_arch.h

  The archive accessing routines

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

#ifndef __GBX_ARCH_H
#define __GBX_ARCH_H

#include "gb_table.h"
#include "gb_magic.h"

typedef
  struct {
    long magic;
    long version;
    long pos_data;
    long pos_string;
    long pos_table;
    long n_symbol;
    }
  ARCH_HEADER;

typedef
  struct {
    SYMBOL sym;
    long pos;
    long len;
    }
  ARCH_SYMBOL;

typedef
  struct {
    int fd;
    ARCH_HEADER header;
    ARCH_SYMBOL *symbol;
    char *string;
    char *addr;
    size_t length;
    }
  ARCH;

typedef
  struct {
    ARCH_SYMBOL *sym;
    long pos;
    long len;
    }
  ARCH_FIND;


PUBLIC ARCH *ARCH_open(const char *path);
PUBLIC void ARCH_close(ARCH *arch);
PUBLIC bool ARCH_find(ARCH *arch, const char *path, int len_path, ARCH_FIND *find);
PUBLIC bool ARCH_read(ARCH *arch, long pos, void *buffer, long len);
PUBLIC void ARCH_get_absolute_path(const char *path, int len_path, char *abs_path, int *len_abs_path);

#endif
