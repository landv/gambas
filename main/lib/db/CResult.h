/***************************************************************************

  CResult.h

  The Result class

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

#ifndef __CRESULT_H
#define __CRESULT_H

#include "gambas.h"
#include "gb.db.h"
#include "deletemap.h"
#include "CDatabase.h"

#include "gb_barray.h"

#ifndef __CRESULT_C
extern GB_DESC CResultDesc[];
extern GB_DESC CBlobDesc[];
extern GB_CLASS CLASS_Blob;
#else

#define THIS ((CRESULT *)_object)
#define BLOB ((CBLOB *)_object)

#endif

enum
{
  RESULT_FIND = 0,
  RESULT_EDIT = 1,
  RESULT_CREATE = 2,
  RESULT_DELETE = 3
};

typedef
  struct {
    GB_BASE ob;
    DB_DRIVER *driver;
    CCONNECTION *conn;
    DB_RESULT handle;
    GB_VARIANT_VALUE *buffer;
    BARRAY changed;
    char *edit;
    DB_INFO info;
    int pos;
    int count;
    int field;
    GB_SUBCOLLECTION fields;
    DELETE_MAP *dmap;
    unsigned available : 1;
    unsigned mode : 2;
		unsigned _reserved : 29;
    }
  CRESULT;

#define CBLOB DB_BLOB

CRESULT *DB_MakeResult(CCONNECTION *db, int mode, char *table, char *query);

#endif
