/***************************************************************************

	keyring.c

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

#define __KEYRING_C

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include "gnome-keyring.h"
#include "keyring.h"

BEGIN_METHOD_VOID(KeyRing_init)

	g_set_prgname(GB.Application.Name());
	g_set_application_name(GB.Application.Title());

END_METHOD

static char *get_password(const char *name)
{
	GnomeKeyringAttributeList * attributes;
	GnomeKeyringResult result;
	GList *found_list;
	GList *i;
	GnomeKeyringFound *found;
	char * password = NULL;

	attributes = g_array_new(FALSE, FALSE, sizeof (GnomeKeyringAttribute));
	gnome_keyring_attribute_list_append_string(attributes, "name", name);
	gnome_keyring_attribute_list_append_string(attributes, "magic", GB.Application.Name());

	result = gnome_keyring_find_items_sync(GNOME_KEYRING_ITEM_GENERIC_SECRET, attributes, &found_list);
	gnome_keyring_attribute_list_free(attributes);

	if (result == GNOME_KEYRING_RESULT_NO_MATCH)
		return NULL;

	if (result != GNOME_KEYRING_RESULT_OK)
	{
		GB.Error("Unable to retrieve password: &1", gnome_keyring_result_to_message(result));
		return NULL;
	}

	for (i = found_list; i != NULL; i = i->next)
	{
		found = i->data;
		password = g_strdup(found->secret);
		break;
	}

	gnome_keyring_found_list_free(found_list);

	return password;
}


static bool set_password(const char *name, const char *password)
{
	GnomeKeyringAttributeList *attributes;
	GnomeKeyringResult result;
	guint item_id;
	GList *found_list;
	GList *i;

	attributes = g_array_new(FALSE, FALSE, sizeof (GnomeKeyringAttribute));
	gnome_keyring_attribute_list_append_string(attributes, "name", name);
	gnome_keyring_attribute_list_append_string(attributes, "magic", GB.Application.Name());

	if (password && *password)
	{
		result = gnome_keyring_item_create_sync(NULL,
			GNOME_KEYRING_ITEM_GENERIC_SECRET,
			name,
			attributes,
			password,
			TRUE,
			&item_id);

		gnome_keyring_attribute_list_free(attributes);

		if (result == GNOME_KEYRING_RESULT_OK)
			return FALSE;

		GB.Error("Unable to store password: &1", gnome_keyring_result_to_message(result));
		return TRUE;
	}
	else
	{
		result = gnome_keyring_find_items_sync(GNOME_KEYRING_ITEM_GENERIC_SECRET, attributes, &found_list);
		gnome_keyring_attribute_list_free(attributes);

		if (result == GNOME_KEYRING_RESULT_NO_MATCH)
			return FALSE;

		if (result == GNOME_KEYRING_RESULT_OK)
		{
			for (i = found_list; i != NULL; i = i->next)
				gnome_keyring_item_delete_sync(NULL, ((GnomeKeyringFound *)(i->data))->item_id);

			gnome_keyring_found_list_free(found_list);
			return FALSE;
		}

		GB.Error("Unable to remove password: &1", gnome_keyring_result_to_message(result));
		return TRUE;
	}
}

BEGIN_METHOD(KeyRing_get_password, GB_STRING key)

	char *password;

	password = get_password(GB.ToZeroString(ARG(key)));

	if (password)
	{
		GB.ReturnNewZeroString(password);
		g_free(password);
	}

END_METHOD

BEGIN_METHOD(KeyRing_set_password, GB_STRING key; GB_STRING password)

	if (!set_password(GB.ToZeroString(ARG(key)), GB.ToZeroString(ARG(password))))
		return;

END_METHOD

GB_DESC CKeyringDesc[] =
{
	GB_DECLARE("_Keyring", 0),

	GB_STATIC_METHOD("_init", NULL, KeyRing_init, NULL),

	GB_STATIC_METHOD("GetPassword", "s", KeyRing_get_password, "(Key)s"),
	GB_STATIC_METHOD("SetPassword", NULL, KeyRing_set_password, "(Key)s(Password)s"),

	GB_END_DECLARE
};
