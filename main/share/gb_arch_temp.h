/***************************************************************************

  gb_arch_temp.h

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

#define __GBX_ARCH_C

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_common_swap.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_arch.h"


#ifndef PROJECT_EXEC
#define E_ARCH "Bad archive: &1"
#endif

static bool _swap = FALSE;
static const char *_path;

static void arch_error(const char *msg)
{
	const char *name = FILE_get_name(_path);
  if (msg == NULL)
    THROW(E_ARCH, name, strerror(errno));
  else
    THROW(E_ARCH, name, msg);
}

static void read_at(ARCH *arch, int pos, void *buf, int len)
{
  memcpy(buf, &arch->addr[pos], len);
}


static void load_arch(ARCH *arch, const char *path)
{
  int len, lens;
  int i;
  int pos;
	int pos_sort;
  struct stat info;
  #ifdef OS_64BITS
  ARCH_SYMBOL_32 *sym;
  #endif

	_path = path;

  arch->fd = open(path, O_RDONLY);
  if (arch->fd < 0)
    THROW(E_OPEN, path, strerror(errno));

  if (fstat(arch->fd, &info) < 0)
    THROW(E_OPEN, path, strerror(errno));

  arch->length = info.st_size;
  arch->addr = mmap(NULL, arch->length, PROT_READ, MAP_PRIVATE, arch->fd, 0);
  if (arch->addr == MAP_FAILED)
    THROW(E_OPEN, path, strerror(errno));

  //fprintf(stderr, "mmap: %s\n", path);

  /* Header */

  read_at(arch, 32, &arch->header, sizeof(ARCH_HEADER));
	_swap = arch->header.magic != ARCH_MAGIC;
  
  if (_swap)
  	SWAP_ints((int *)&arch->header, 6);

	if (arch->header.magic != ARCH_MAGIC)
		arch_error("not an archive");
	
  //if (arch->header.version != ARCH_VERSION)
  //	arch_error("bad version");

  /* Strings */

  len = arch->header.pos_table - arch->header.pos_string;
  if (len <= 0)
    arch_error("corrupted header");

  ALLOC(&arch->string, len);
  read_at(arch, arch->header.pos_string, arch->string, len);

  /* File names table */

	len = arch->header.n_symbol * sizeof(ARCH_SYMBOL);
	lens = arch->header.n_symbol * sizeof(ushort);
	if (len <= 0 || lens <= 0)
		arch_error("corrupted header");

	ALLOC(&arch->symbol, len);
	ALLOC(&arch->sort, lens);
	
	#ifdef OS_64BITS
		sym = (ARCH_SYMBOL_32 *)&arch->addr[arch->header.pos_table];
		for (i = 0; i < arch->header.n_symbol; i++, sym++)
		{
			//arch->symbol[i].sym.sort = sym->sym.sort;
			arch->symbol[i].sym.len = sym->sym.len;
			arch->symbol[i].sym.name = (char *)(intptr_t)sym->sym.name;
			arch->symbol[i].pos = sym->pos;
			arch->symbol[i].len = sym->len;
		}	
		pos_sort = arch->header.pos_table + arch->header.n_symbol * sizeof(ARCH_SYMBOL_32);
	#else
		read_at(arch, arch->header.pos_table, arch->symbol, len);
		pos_sort = arch->header.pos_table + len;
	#endif
	
	read_at(arch, pos_sort, arch->sort, lens);

  /* String relocation */

  pos = 0;
	if (_swap)
	{
		for (i = 0; i < arch->header.n_symbol; i++)
		{
			SWAP_short((short *)&arch->sort[i]);
			SWAP_int(&arch->symbol[i].sym.len);
			SWAP_int(&arch->symbol[i].pos);
			SWAP_int(&arch->symbol[i].len);
			arch->symbol[i].sym.name = &arch->string[pos];
			pos += arch->symbol[i].sym.len;
		}
	}
	else
	{
		for (i = 0; i < arch->header.n_symbol; i++)
		{
			arch->symbol[i].sym.name = &arch->string[pos];
			pos += arch->symbol[i].sym.len;
		}
	}

	_path = NULL;

	#if 0
  for (i = 0; i < arch->header.n_symbol; i++)
	{
		fprintf(stderr, "%i (%i) %.*s\n", i, arch->sort[i], arch->symbol[i].sym.len, arch->symbol[i].sym.name);
	}
	#endif
}


ARCH *ARCH_open(const char *path)
{
  ARCH *arch;

  ALLOC_ZERO(&arch, sizeof(ARCH));

  load_arch(arch, path);

  return arch;
}

void ARCH_close(ARCH *arch)
{
  if (arch->fd)
  {
    FREE(&arch->string);
    FREE(&arch->symbol);
    FREE(&arch->sort);
    munmap(arch->addr, arch->length);
    close(arch->fd);
  }

  FREE(&arch);
}


static bool get_absolute_path(const char *path, int len_path, char *abs_path, int *len_abs_path)
{
  const char *p, *lp;
  char *ap, *apm;
  char c;
  int rest;
	bool err = FALSE;

	p = path;
	lp = p;
	ap = abs_path;
	apm = &abs_path[PATH_MAX];
	*len_abs_path = 0;

	for(;;)
	{
		rest = &path[len_path] - p;
		if (rest <= 0)
			break;
		
		if (ap >= apm)
		{
			err = TRUE;
			break;
		}

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
	*ap = 0;
	return err;
}

static int strint(char *str, int value)
{
	char buffer[16];
	char *p;
	int len;
	
	p = &buffer[15];
	while (value >= 10)
	{
		*p-- = '0' + value % 10;
		value /= 10;
	}
	*p = '0' + value;
	
	len = buffer + sizeof(buffer) - p;
	memcpy(str, p, len);
	return len;
}

bool ARCH_find(ARCH *arch, const char *path, int len_path, ARCH_FIND *find)
{
  int ind;
  ARCH_SYMBOL *sym;
  char tpath[PATH_MAX];
  int len_tpath;
	int n;

  if (len_path <= 0)
    len_path = strlen(path);

	if (get_absolute_path(path, len_path, tpath, &len_tpath))
		return TRUE;
	
	if (len_tpath == 0)
	{
		find->sym = NULL;
		find->index = NO_SYMBOL;
		find->pos = -1;
		find->len = 0;
		return FALSE;
	}

  //if (arch->header.version == 2)
  //{
		char *p;
		char tpath2[len_tpath + 8];
	
		for(;;)
		{
			p = index(tpath + 1, '/');
			if (!p)
				break;
	
			ind = SYMBOL_find(arch->symbol, arch->sort, arch->header.n_symbol, sizeof(ARCH_SYMBOL), TF_NORMAL, tpath, p - tpath, 0);
			if (ind == NO_SYMBOL)
				break;

			sym = &arch->symbol[ind];
			tpath2[0] = '/';
			n = strint(&tpath2[1], ind);
			tpath2[n + 1] = ':';
			strcpy(&tpath2[n + 2], p + 1);
			len_tpath = n + 2 + strlen(p + 1);
			memcpy(tpath, tpath2, len_tpath);
			tpath[len_tpath] = 0;
		}
		
  	ind = SYMBOL_find(arch->symbol, arch->sort, arch->header.n_symbol, sizeof(ARCH_SYMBOL), TF_NORMAL, tpath, len_tpath, 0);
	//}
	//else
  //	SYMBOL_find_old(arch->symbol, arch->header.n_symbol, sizeof(ARCH_SYMBOL), TF_NORMAL, tpath, len_tpath, 0, &ind);
	
  if (ind == NO_SYMBOL)
    return TRUE;

  sym = &arch->symbol[ind];

  find->sym = sym;
  find->pos = sym->pos;
  find->len = sym->len;
  find->index = ind;

  return FALSE;
}


bool ARCH_read(ARCH *arch, int pos, void *buffer, int len)
{
  /*if (lseek(arch->fd, pos, SEEK_SET) < 0)
    return TRUE;

  if (read(arch->fd, buffer, len) != len)
    return TRUE;*/

  memcpy(buffer, &arch->addr[pos], len);

  return FALSE;
}
