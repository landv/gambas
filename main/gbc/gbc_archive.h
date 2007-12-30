/***************************************************************************

  archive.h

  The GAMBAS archiver

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

#ifndef __GBC_ARCHIVE_H
#define __GBC_ARCHIVE_H

#include "gb_alloc.h"
#include "gb_limit.h"
#include "gb_table.h"
#include "gb_magic.h"

typedef
  struct {
    SYMBOL symbol;
    int pos;
    int len;
    }
  ARCH_SYMBOL;

#ifndef __GBC_ARCHIVE_C

EXTERN char *ARCH_project;
EXTERN char *ARCH_project_name;
EXTERN char *ARCH_output;
EXTERN bool ARCH_verbose;
EXTERN bool ARCH_swap;

#endif

PUBLIC void ARCH_init(void);
PUBLIC void ARCH_exit(void);
PUBLIC void ARCH_define_project(const char *project);
PUBLIC void ARCH_define_output(const char *path);
PUBLIC int ARCH_add_file(const char *path);

#endif
