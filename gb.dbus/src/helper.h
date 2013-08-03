/***************************************************************************

  helper.h

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

#ifndef __HELPER_H
#define __HELPER_H

#include "main.h"

#ifndef __HELPER_C
extern bool DBUS_Debug;
#endif

bool DBUS_call_method(DBusConnection *connection, const char *application, const char *path, const char *interface, const char *method, 
                      const char *signature_in, const char *signature_out, GB_ARRAY arguments);

char *DBUS_introspect(DBusConnection *connection, const char *dest, const char *path);

bool DBUS_register(DBusConnection *connection, const char *name, bool unique);
bool DBUS_unregister(DBusConnection *connection, const char *name);
bool DBUS_watch(DBusConnection *connection, bool on);

bool DBUS_validate_path(const char *path, int len);
bool DBUS_validate_interface (const char *interface, int len);
bool DBUS_validate_method(const char *method, int len);

bool DBUS_retrieve_message_arguments(DBusMessage *message);
bool DBUS_reply(DBusConnection *connection, DBusMessage *message, const char *signature, GB_ARRAY arguments);

const char *DBUS_to_dbus_type(GB_TYPE type);

bool DBUS_error(DBusConnection *connection, DBusMessage *message, const char *type, const char *error);

#endif
