/***************************************************************************

  CField.h

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#ifndef __CFIELD_H
#define __CFIELD_H

#include "gambas.h"
#include "gb.db.h"
#include "CTable.h"

#ifndef __CFIELD_C
extern GB_DESC CTableFieldsDesc[];
extern GB_DESC CFieldDesc[];
#else

#define THIS ((CFIELD *)_object)

#endif

typedef
  struct {
    GB_BASE ob;
    DB_DRIVER *driver;
    CTABLE *table;
    char *name;
    DB_FIELD info;
    }
  CFIELD;

void *CFIELD_get(CTABLE *table, const char *name);
int CFIELD_exist(CTABLE *table, const char *name);
void CFIELD_list(CTABLE *table, char ***list);
void CFIELD_release(CTABLE *table, void *_object);

void CFIELD_free_info(DB_FIELD *info);

#endif

