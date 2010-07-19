/***************************************************************************

  c_dbus.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __C_DBUS_C

#include "c_dbusconnection.h"
#include "c_dbus.h"

BEGIN_PROPERTY(DBus_System)

	GB.ReturnObject(CDBUSCONNECTION_get(DBUS_BUS_SYSTEM));

END_PROPERTY

BEGIN_PROPERTY(DBus_Session)

	GB.ReturnObject(CDBUSCONNECTION_get(DBUS_BUS_SESSION));

END_PROPERTY

GB_DESC CDBusDesc[] =
{
  GB_DECLARE("_DBus", 0),

  GB_STATIC_PROPERTY_READ("System", "DBusConnection", DBus_System),
  GB_STATIC_PROPERTY_READ("Session", "DBusConnection", DBus_Session),

  GB_CONSTANT("Method", "i", DBUS_MESSAGE_TYPE_METHOD_CALL),
  GB_CONSTANT("Reply", "i", DBUS_MESSAGE_TYPE_METHOD_RETURN),
  GB_CONSTANT("Signal", "i", DBUS_MESSAGE_TYPE_SIGNAL),
  GB_CONSTANT("Error", "i", DBUS_MESSAGE_TYPE_ERROR),
  
  GB_END_DECLARE
};

