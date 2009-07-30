/***************************************************************************

  c_dbus.c

  gb.dbus component

  (c) 2009 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __C_DBUS_C

#include "c_dbusconnection.h"
#include "c_dbus.h"

BEGIN_PROPERTY(dbus_System)

	GB.ReturnObject(CDBUSCONNECTION_get(DBUS_BUS_SYSTEM));

END_PROPERTY

BEGIN_PROPERTY(dbus_Session)

	GB.ReturnObject(CDBUSCONNECTION_get(DBUS_BUS_SESSION));

END_PROPERTY

GB_DESC CDBusDesc[] =
{
  GB_DECLARE("DBus", 0),

  GB_STATIC_PROPERTY_READ("System", "DBusConnection", dbus_System),
  GB_STATIC_PROPERTY_READ("Session", "DBusConnection", dbus_Session),
  
  GB_END_DECLARE
};

