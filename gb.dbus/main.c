/***************************************************************************

  main.c

  (c) 2000-2017 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include "c_dbusvariant.h"
#include "c_dbusconnection.h"
#include "c_dbusobserver.h"
#include "c_dbus.h"

#include "main.h"

GB_INTERFACE GB EXPORT;

GB_CLASS CLASS_DBusVariant;
GB_CLASS CLASS_DBusNull;
GB_CLASS CLASS_DBusObject;

GB_DESC *GB_CLASSES[] EXPORT =
{
	CDBusVariantDesc,
	CDBusObserverMessageDesc,
	CDBusObserverDesc,
	CDBusConnectionDesc,
	CDBusDesc,
	NULL
};

int EXPORT GB_INIT(void)
{
	CLASS_DBusVariant = GB.FindClass("DBusVariant");
	CLASS_DBusNull = GB.FindClass("_DBusNull");
	CLASS_DBusObject = GB.FindClass("DBusObject");
	return 0;
}

void EXPORT GB_EXIT()
{
}

