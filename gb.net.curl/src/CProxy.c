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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CPROXY_C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

#include "CProxy.h"


/***************************************************************
	PROPERTIES
****************************************************************/
BEGIN_PROPERTY (CProxy_Auth)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->proxy.auth);
		return;
	}

	if (*(THIS->parent_status) > 0)
	{
		GB.Error ("Proxy Auth property can not be changed while working");
		return;
  	}

	if (Adv_proxy_SETAUTH (&THIS->proxy,VPROP(GB_INTEGER)) )
		GB.Error ("Unknown authentication method");

END_PROPERTY

BEGIN_PROPERTY (CProxy_TYPE)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->proxy.type);
		return;
	}

	if (*(THIS->parent_status) > 0)
	{
		GB.Error ("Proxy Type property can not be changed while working");
		return;
  	}
	if (Adv_proxy_SETTYPE(&THIS->proxy,VPROP(GB_INTEGER)) )
		GB.Error ("Unknown proxy type");

END_PROPERTY

BEGIN_PROPERTY (CProxy_USER)

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->proxy.user);
		return;
	}

	if (*(THIS->parent_status) > 0)
	{
		GB.Error ("Proxy User property can not be changed while working");
		return;
  	}

	if (THIS->proxy.user) GB.FreeString (&THIS->proxy.user);
	GB.StoreString(PROP(GB_STRING), &THIS->proxy.user);

END_PROPERTY

BEGIN_PROPERTY (CProxy_PASSWORD)

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->proxy.pwd);
		return;
	}

	if (*(THIS->parent_status) > 0)
	{
		GB.Error ("Proxy Passwod property can not be changed while working");
		return;
  	}

	if (THIS->proxy.pwd) GB.FreeString (&THIS->proxy.pwd);
	GB.StoreString(PROP(GB_STRING), &THIS->proxy.pwd);

END_PROPERTY

BEGIN_PROPERTY (CProxy_HOST)

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->proxy.host);
		return;
	}

	if (*(THIS->parent_status) > 0)
	{
		GB.Error ("Proxy Host property can not be changed while working");
		return;
  	}

	if (THIS->proxy.host) GB.FreeString (&THIS->proxy.host);
	GB.StoreString(PROP(GB_STRING), &THIS->proxy.host);


END_PROPERTY

/***************************************************************
	METHODS
****************************************************************/
/*BEGIN_METHOD_VOID(CProxy_NEW)

	Adv_proxy_NEW(&THIS->proxy);

END_METHOD

BEGIN_METHOD_VOID(CProxy_FREE)

	Adv_proxy_CLEAR(&THIS->proxy);

END_METHOD*/

/***************************************************************
 Here we declare the public interface of Proxy class
 ***************************************************************/
GB_DESC CProxyDesc[] =
{
  GB_DECLARE(".CurlProxy", sizeof(CPROXY)), GB_VIRTUAL_CLASS(),

  //GB_METHOD("_new", NULL, CProxy_NEW, NULL),
  //GB_METHOD("_free", NULL, CProxy_FREE, NULL),

  GB_PROPERTY("Host", "s", CProxy_HOST),
  GB_PROPERTY("User", "s", CProxy_USER),
  GB_PROPERTY("Password", "s", CProxy_PASSWORD),
  GB_PROPERTY("Type", "i", CProxy_TYPE),
  GB_PROPERTY("Auth", "i", CProxy_Auth),

  GB_END_DECLARE
};




