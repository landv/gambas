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

static bool check_active(CCURL *_object)
{
	if (THIS && THIS->status > 0)
	{
		GB.Error("Proxy cannot be modified while client is active");
		return TRUE;
	}
	else
		return FALSE;
}

#define GET_PROXY() CURL_PROXY *proxy = _object ? &THIS->proxy : &CURL_default_proxy

BEGIN_PROPERTY(CurlProxy_Auth)

	GET_PROXY();

	if (READ_PROPERTY)
		GB.ReturnInteger(proxy->auth);
	else
	{
		if (check_active(THIS))
			return;

		if (CURL_proxy_set_auth(proxy, VPROP(GB_INTEGER)))
			GB.Error("Unknown authentication method");
	}

END_PROPERTY


BEGIN_PROPERTY(CurlProxy_Type)

	GET_PROXY();

	if (READ_PROPERTY)
		GB.ReturnInteger(proxy->type);
	else
	{
		if (check_active(THIS))
			return;

		if (CURL_proxy_set_type(proxy, VPROP(GB_INTEGER)))
			GB.Error("Unknown proxy type");
	}

END_PROPERTY


BEGIN_PROPERTY(CurlProxy_User)

	GET_PROXY();

	if (READ_PROPERTY)
		GB.ReturnString(proxy->user);
	else
	{
		if (check_active(THIS))
			return;

		GB.StoreString(PROP(GB_STRING), &proxy->user);
	}

END_PROPERTY


BEGIN_PROPERTY(CurlProxy_Password)

	GET_PROXY();

	if (READ_PROPERTY)
		GB.ReturnString(proxy->pwd);
	else
	{
		if (check_active(THIS))
			return;

		GB.StoreString(PROP(GB_STRING), &proxy->pwd);
	}

END_PROPERTY


BEGIN_PROPERTY (CurlProxy_Host)

	GET_PROXY();

	if (READ_PROPERTY)
		GB.ReturnString(proxy->host);
	else
	{
		if (check_active(THIS))
			return;

		GB.StoreString(PROP(GB_STRING), &proxy->host);
	}

END_PROPERTY


GB_DESC CProxyDesc[] =
{
	GB_DECLARE_VIRTUAL(".Curl.Proxy"),
	
	GB_PROPERTY("Host", "s", CurlProxy_Host),
	GB_PROPERTY("User", "s", CurlProxy_User),
	GB_PROPERTY("Password", "s", CurlProxy_Password),
	GB_PROPERTY("Type", "i", CurlProxy_Type),
	GB_PROPERTY("Auth", "i", CurlProxy_Auth),

	GB_END_DECLARE
};




