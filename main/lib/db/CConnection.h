/***************************************************************************

  CConnection.h

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

#ifndef __CCONNECTION_H
#define __CCONNECTION_H

#include "gambas.h"
#include "gb.db.h"

#ifndef __CCONNECTION_C
extern GB_DESC CConnectionDesc[];
extern GB_DESC CDBDesc[];
#else

#define THIS ((CCONNECTION *)_object)

#endif

typedef
  struct {
    GB_BASE ob;
    DB_DRIVER *driver;
    DB_DATABASE db;
    DB_DESC desc;
    GB_SUBCOLLECTION databases;
    GB_SUBCOLLECTION tables;
    GB_SUBCOLLECTION views;
    GB_SUBCOLLECTION users;
    int limit;
    int trans;
    }
  CCONNECTION;

#endif
