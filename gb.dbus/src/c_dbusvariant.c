/***************************************************************************

  c_dbusvariant.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

BEGIN_METHOD_VOID(DBusVariant_free)

	GB.StoreVariant(NULL, (void *)&THIS->value);
	//GB.StoreString(NULL, &THIS->signature);
	//fprintf(stderr, "DBusVariant: free: %p\n", THIS);

END_METHOD

BEGIN_PROPERTY(DBusVariant_Value)

	if (READ_PROPERTY)
		GB.ReturnVariant(&THIS->value);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->value);

END_PROPERTY

/*BEGIN_PROPERTY(DBusVariant_Signature)

	GB.ReturnString(THIS->signature);

END_PROPERTY*/

GB_DESC CDBusVariantDesc[] =
{
  GB_DECLARE("DBusVariant", sizeof(CDBUSVARIANT)),

	GB_METHOD("_free", NULL, DBusVariant_free, NULL),
	GB_PROPERTY("Value", "v", DBusVariant_Value),
	GB_CONSTANT("Signature", "s", "v"),

  GB_END_DECLARE
};


char *CDBUSVARIANT_get_signature(CDBUSVARIANT *_object)
{
	GB_VALUE *val = GB.GetProperty((void *)GB.GetClass(THIS), "Signature");
	
	if (!val || (val->type != GB_T_STRING && val->type != GB_T_CSTRING))
		return "v";
	
	return val->_string.value.addr;
}
