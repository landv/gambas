/***************************************************************************

  c_dbusvariant.c

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

#define __C_DBUSVARIANT_C

#include "helper.h"
#include "c_dbusvariant.h"

BEGIN_METHOD(DBusVariant_call, GB_VARIANT value; GB_STRING signature)

	_object = GB.New(GB.FindClass("DBusVariant"), NULL, NULL);
	GB.StoreVariant(ARG(value), (void *)&THIS->value);
	GB.StoreString(ARG(signature), &THIS->signature);
	GB.ReturnObject(THIS);
	//fprintf(stderr, "DBusVariant: new: %p %d\n", _object, (int)((GB_BASE *)_object)->ref);

END_METHOD

BEGIN_METHOD_VOID(DBusVariant_free)

	GB.StoreVariant(NULL, (void *)&THIS->value);
	GB.StoreString(NULL, &THIS->signature);
	//fprintf(stderr, "DBusVariant: free: %p\n", THIS);

END_METHOD

BEGIN_PROPERTY(DBusVariant_Value)

	GB.ReturnVariant(&THIS->value);

END_PROPERTY

BEGIN_PROPERTY(DBusVariant_Signature)

	GB.ReturnString(THIS->signature);

END_PROPERTY

GB_DESC CDBusVariantDesc[] =
{
  GB_DECLARE("DBusVariant", sizeof(CDBUSVARIANT)), GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("_call", "DBusVariant", DBusVariant_call, "(Value)v(Signature)s"),
	GB_METHOD("_free", NULL, DBusVariant_free, NULL),
	GB_PROPERTY_READ("Value", "v", DBusVariant_Value),
	GB_PROPERTY_READ("Signature", "s", DBusVariant_Signature),

  GB_END_DECLARE
};

