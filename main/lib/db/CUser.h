/***************************************************************************

  CUser.h

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

#ifndef __CUSER_H
#define __CUSER_H

#include "gambas.h"
#include "gb.db.h"
#include "CConnection.h"

#ifndef __CUSER_C
extern GB_DESC CConnectionUsersDesc[];
extern GB_DESC CUserDesc[];
#else

#define THIS ((CUSER *)_object)

#endif

typedef
  struct {
    GB_BASE ob;
    DB_DRIVER *driver;
    CCONNECTION *conn;
    char *name;
    DB_USER info;
    }
  CUSER;

void *CUSER_get(CCONNECTION *conn, const char *key);
int CUSER_exist(CCONNECTION *conn, const char *key);
void CUSER_list(CCONNECTION *conn, char ***list);
void CUSER_release(CCONNECTION *conn, void *_object);

#endif

