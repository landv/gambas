/***************************************************************************

  CProxy.c

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CPROXY_C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

#include "CProxy.h"

static bool check_active(CPROXY *proxy)
{
	if (*(proxy->parent_status) > 0)
	{
		GB.Error("Proxy cannot be modified while client is active");
		return TRUE;
	}
	else
		return FALSE;
}

BEGIN_PROPERTY(CurlProxy_Auth)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->proxy.auth);
	else
	{
		if (check_active(THIS))
			return;

		if (CURL_proxy_set_auth(&THIS->proxy, VPROP(GB_INTEGER)))
			GB.Error("Unknown authentication method");
	}

END_PROPERTY


BEGIN_PROPERTY(CurlProxy_Type)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->proxy.type);
	else
	{
		if (check_active(THIS))
			return;

		if (CURL_proxy_set_type(&THIS->proxy, VPROP(GB_INTEGER)))
			GB.Error("Unknown proxy type");
	}

END_PROPERTY


BEGIN_PROPERTY(CurlProxy_User)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->proxy.user);
	else
	{
		if (check_active(THIS))
			return;

		GB.StoreString(PROP(GB_STRING), &THIS->proxy.user);
	}

END_PROPERTY


BEGIN_PROPERTY(CurlProxy_Password)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->proxy.pwd);
	else
	{
		if (check_active(THIS))
			return;

		GB.StoreString(PROP(GB_STRING), &THIS->proxy.pwd);
	}

END_PROPERTY


BEGIN_PROPERTY (CurlProxy_Host)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->proxy.host);
	else
	{
		if (check_active(THIS))
			return;

		GB.StoreString(PROP(GB_STRING), &THIS->proxy.host);
	}

END_PROPERTY


GB_DESC CProxyDesc[] =
{
	GB_DECLARE(".Curl.Proxy", sizeof(CPROXY)), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Host", "s", CurlProxy_Host),
	GB_PROPERTY("User", "s", CurlProxy_User),
	GB_PROPERTY("Password", "s", CurlProxy_Password),
	GB_PROPERTY("Type", "i", CurlProxy_Type),
	GB_PROPERTY("Auth", "i", CurlProxy_Auth),

	GB_END_DECLARE
};




