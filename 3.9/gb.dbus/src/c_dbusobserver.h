/***************************************************************************

  c_dbusobserver.h

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

#ifndef __C_DBUSOBSERVER_H
#define __C_DBUSOBSERVER_H

#include "main.h"

typedef
	struct CDBUSOBSERVER
	{
		GB_BASE ob;
		struct CDBUSOBSERVER *prev;
		struct CDBUSOBSERVER *next;
		DBusConnection *connection;
		int type;
		char *object;
		char *member;
		char *interface;
		char *destination;
		DBusMessage *message;
		//GB_VARIANT tag;
		unsigned enabled : 1;
		unsigned reply : 1;
	}
	CDBUSOBSERVER;

#ifndef __C_DBUSOBSERVER_C

extern GB_DESC CDBusObserverDesc[];
extern GB_DESC CDBusObserverMessageDesc[];

extern CDBUSOBSERVER *DBUS_observers;

#else

#define THIS ((CDBUSOBSERVER *)_object)

#endif

void DBUS_raise_observer(CDBUSOBSERVER *_object);
	
#endif /* __CDBUS_H */
