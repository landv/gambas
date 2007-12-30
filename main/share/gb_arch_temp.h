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

#define __GBX_ARCH_C

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_common_swap.h"
#include "gb_magic.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_arch.h"


#ifndef PROJECT_EXEC
#define E_ARCH "Bad archive: &1"
#endif

static bool _swap = FALSE;

static void arch_error(const char *msg)
{
  if (msg == NULL)
    THROW(E_ARCH, strerror(errno));
  else
    THROW(E_ARCH, msg);
}

static void read_at(ARCH *arch, long pos, void *buf, long len)
{
  memcpy(buf, &arch->addr[pos], len);
}


static void load_arch(ARCH *arch, const char *path)
{
  long len;
  int i;
  long pos;
  struct stat info;

  arch->fd = open(path, O_RDONLY);
  if (arch->fd < 0)
    THROW(E_OPEN, path, strerror(errno));

  if (fstat(arch->fd, &info) < 0)
    THROW(E_OPEN, path, strerror(errno));

  arch->length = info.st_size;
  arch->addr = mmap(NULL, arch->length, PROT_READ | PROT_WRITE, MAP_PRIVATE, arch->fd, 0);
  if (arch->addr == MAP_FAILED)
    THROW(E_OPEN, path, strerror(errno));

  //fprintf(stderr, "mmap: %s\n", path);

  /* Header */

  read_at(arch, 32, &arch->header, sizeof(ARCH_HEADER));
	_swap = arch->header.magic != ARCH_MAGIC;

  if (_swap)
  {
  	fprintf(stderr, "gbx: Swapping archive\n");
  	SWAP_longs((long *)&arch->header, 6);
	}

  /* Strings */

  len = arch->header.pos_table - arch->header.pos_string;
  if (len <= 0)
    arch_error("corrupted header");

  ALLOC(&arch->string, len, "ARCH_init");
  read_at(arch, arch->header.pos_string, arch->string, len);

  /* Lecture de la table */

  len = arch->header.n_symbol * sizeof(ARCH_SYMBOL);
  if (len <= 0)
    arch_error("corrupted header");

  ALLOC(&arch->symbol, len, "ARCHIVE_load");
  read_at(arch, arch->header.pos_table, arch->symbol, len);

  /* Relocation des cha�es */

  pos = 0;
  for (i = 0; i < arch->header.n_symbol; i++)
  {
    if (_swap)
    {
    	SWAP_short((short *)&arch->symbol[i].sym.sort);
    	SWAP_short((short *)&arch->symbol[i].sym.len);
			SWAP_long(&arch->symbol[i].pos);
			SWAP_long(&arch->symbol[i].len);
    }
    arch->symbol[i].sym.name = &arch->string[pos];
    pos += arch->symbol[i].sym.len;
  }

  /*if (arch != ARCHIVE_main)
    load_exported_class(arch);*/
}


PUBLIC ARCH *ARCH_open(const char *path)
{
  ARCH *arch;

  ALLOC_ZERO(&arch, sizeof(ARCH), "ARCH_open");

  //arch->name = name;

  load_arch(arch, path);

  return arch;
}

PUBLIC void ARCH_close(ARCH *arch)
{
  if (arch->fd)
  {
    FREE(&arch->string, "ARCH_close");
    FREE(&arch->symbol, "ARCH_close");
    munmap(arch->addr, arch->length);
    close(arch->fd);
  }

  FREE(&arch, "ARCH_close");
}


PUBLIC void ARCH_get_absolute_path(const char *path, int len_path, char *abs_path, int *len_abs_path)
{
  const char *p, *lp;
  char *ap;
  char c;
  int rest;

	p = path;
	lp = p;
	ap = abs_path;
	*len_abs_path = 0;

	for(;;)
	{
		rest = &path[len_path] - p;
		if (rest <= 0)
			break;

		c = *p;

		if (p == lp && c == '.')
		{
			if (rest == 1 || p[1] == '/')
			{
				if (rest == 1)
					p++;
				else
					p += 2;

				lp = p;
				continue;
			}
			else if (rest >= 2 && p[1] == '.' && (rest == 2 || p[2] == '/'))
			{
				if (ap > abs_path)
				{
					ap--; // Jumps the last '/'
					while (ap > abs_path)
					{
						ap--;
						if (*ap == '/')
						{
							ap++;
							break;
						}
					}
				}

				p += 3;
				lp = p;
				continue;
			}
		}

		*ap++ = c;
		p++;

		if (c == '/')
			lp = p;
	}

	*len_abs_path = ap - abs_path;
}

PUBLIC bool ARCH_find(ARCH *arch, const char *path, int len_path, ARCH_FIND *find)
{
  long index;
  ARCH_SYMBOL *sym;
  char abs_path[MAX_PATH];
  int len_abs_path;

  if (len_path <= 0)
    len_path = strlen(path);

	ARCH_get_absolute_path(path, len_path, abs_path, &len_abs_path);

	if (len_abs_path == 0)
	{
		find->sym = NULL;
		find->pos = -1;
		find->len = 0;
		return FALSE;
	}

	//fprintf(stderr, "ARCH_find: %.*s => %.*s\n", len_path, path, len_abs_path, abs_path);

  SYMBOL_find(arch->symbol, arch->header.n_symbol, sizeof(ARCH_SYMBOL), TF_NORMAL, abs_path, len_abs_path, 0, &index);

  //FREE(&abs_path, "ARCH_find");

  if (index == NO_SYMBOL)
    return TRUE;

  sym = &arch->symbol[index];

  find->sym = sym;
  find->pos = find->sym->pos;
  find->len = sym->len;

  return FALSE;
}


PUBLIC bool ARCH_read(ARCH *arch, long pos, void *buffer, long len)
{
  /*if (lseek(arch->fd, pos, SEEK_SET) < 0)
    return TRUE;

  if (read(arch->fd, buffer, len) != len)
    return TRUE;*/

  memcpy(buffer, &arch->addr[pos], len);

  return FALSE;
}


