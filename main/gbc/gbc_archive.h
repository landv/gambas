/***************************************************************************

  gbc_archive.h

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

#ifndef __GBC_ARCHIVE_H
#define __GBC_ARCHIVE_H

#include "gb_alloc.h"
#include "gb_limit.h"
#include "gb_table.h"
#include "gb_magic.h"
#include "gb_arch.h"

#ifndef __GBC_ARCHIVE_C

EXTERN char *ARCH_project;
EXTERN char *ARCH_project_name;
EXTERN char *ARCH_output;
EXTERN bool ARCH_verbose;
EXTERN bool ARCH_swap;

#endif

void ARCH_init(void);
void ARCH_exit(void);
void ARCH_define_project(const char *project);
void ARCH_define_output(const char *path);
int ARCH_add_file(const char *path);

#endif
