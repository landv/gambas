/***************************************************************************

  CTable.h

  The Table class

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

#ifndef __CTABLE_H
#define __CTABLE_H

#include "gambas.h"
#include "gb.db.h"
#include "CDatabase.h"

#ifndef __CTABLE_C
extern GB_DESC CTableDesc[];
extern GB_DESC CConnectionTablesDesc[];
//extern GB_DESC CTableFieldDesc[];
//extern GB_DESC CTableIndexDesc[];
#else

#define THIS ((CTABLE *)_object)

#endif

typedef
  struct {
    GB_BASE ob;
    DB_DRIVER *driver;
    CCONNECTION *conn;
    char *name;
    char *type;

    GB_SUBCOLLECTION fields;
    GB_SUBCOLLECTION indexes;

    bool create;
    DB_FIELD *new_fields; /* linked list of DB_FIELD structures located in the objects stored in fields */
    char **primary;
    }
  CTABLE;

void *CTABLE_get(CCONNECTION *conn, const char *key);
int CTABLE_exist(CCONNECTION *conn, const char *key);
void CTABLE_list(CCONNECTION *conn, char ***list);
  
#endif
