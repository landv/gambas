/***************************************************************************

  CResultField.h

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

#ifndef __CRESULTFIELD_H
#define __CRESULTFIELD_H

#include "gambas.h"
#include "gb.db.h"
#include "CResult.h"

#ifndef __CRESULTFIELD_C
extern GB_DESC CResultFieldsDesc[];
extern GB_DESC CResultFieldDesc[];
#else

#define THIS ((CRESULTFIELD *)_object)

#endif

typedef
  struct {
    GB_BASE ob;
    DB_DRIVER *driver;
    CRESULT *result;
    int index;
    }
  CRESULTFIELD;

void *CRESULTFIELD_get(CRESULT *result, const char *name);
int CRESULTFIELD_exist(CRESULT *result, const char *name);
int CRESULTFIELD_find(CRESULT *result, const char *name, bool error);
void CRESULTFIELD_release(CRESULT *result, void *_object);

char *CRESULTFIELD_key(CRESULT *result, int index);

#endif

