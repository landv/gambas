/***************************************************************************

  CNet.c

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CNET_C
#include "main.h"
#include <stdio.h>
#include <string.h>

#include "CNet.h"


size_t NET_get_address_size(NET_ADDRESS *addr)
{
	switch (addr->a.sa_family)
	{
		case PF_UNIX: return sizeof(struct sockaddr_un);
		case PF_INET: return sizeof(struct sockaddr_in);
		default: return 0;
	}
}

void ToIPv4(char *src,char *dst,int leadzero)
{
	int myloop;
	int zone=0;
	int np=0;
	int nc[4]={0,0,0,0};
	
	dst[0]=0;
	
	if (!src) return;
	
	for (myloop=0;myloop<strlen(src);myloop++)
	{
		switch(zone)
		{
			case 0:
				if (src[myloop]!=' ')
				{
				zone=1;
				myloop--;
				}
				break;
			
			case 1:
				if (src[myloop]==' ')
				{
					zone=2;
				}
				else
				{
					if ( (src[myloop]<48) || (src[myloop]>57) )
					{
						if (src[myloop]=='.') { if (++np > 3) return; }
						else { return; }
					}
					else
					{
						nc[np]*=10;
						nc[np]+=(src[myloop]-48);
						if (nc[np]>255) return;
					}
				}
				break;
			
			case 2:
				if (src[myloop]!=' ') return;
				break;
		
		}
	}

	if (!leadzero)
		sprintf(dst,"%d.%d.%d.%d",nc[0],nc[1],nc[2],nc[3]);
	else
		sprintf(dst,"%03d.%03d.%03d.%03d",nc[0],nc[1],nc[2],nc[3]);
}

BEGIN_METHOD(CNET_Format,GB_STRING IpString;GB_INTEGER Format;GB_BOOLEAN LeadZero;)

	int leadzero=0;

	char dst[16];

	if (!MISSING(Format))
		if (VARG(Format)!=0) { GB.Error("Unknown Format"); return; }
	
	if (!MISSING(LeadZero)) leadzero=VARG(LeadZero);
	if (!LENGTH(IpString)) return;

	ToIPv4 (STRING(IpString),dst,leadzero);
	GB.ReturnNewZeroString(dst);
	

END_METHOD

/***************************************************************
Here we declare the public interface of NetCode class
***************************************************************/
GB_DESC CNetDesc[] =
{
	GB_DECLARE("Net", 0), GB_VIRTUAL_CLASS(),

	/* IP Formatting */
	GB_CONSTANT("IPv4","i",0),
	/* normal operation */
	GB_CONSTANT("Inactive", "i", NET_INACTIVE),
	GB_CONSTANT("Active", "i", NET_ACTIVE),
	GB_CONSTANT("Pending", "i", NET_PENDING),
	GB_CONSTANT("Accepting", "i", NET_ACCEPTING),
	GB_CONSTANT("ReceivingData", "i", NET_RECEIVING_DATA),
	GB_CONSTANT("Searching", "i", NET_SEARCHING),
	GB_CONSTANT("Connecting", "i", NET_CONNECTING),
	GB_CONSTANT("Connected", "i", NET_CONNECTED),
	/* net error codes */
	GB_CONSTANT("CannotCreateSocket", "i", NET_CANNOT_CREATE_SOCKET),
	GB_CONSTANT("ConnectionRefused", "i", NET_CONNECTION_REFUSED),
	GB_CONSTANT("CannotRead", "i", NET_CANNOT_READ),
	GB_CONSTANT("CannotWrite", "i", NET_CANNOT_WRITE),
	GB_CONSTANT("HostNotFound", "i", NET_HOST_NOT_FOUND),
	GB_CONSTANT("CannotBindSocket", "i", NET_CANNOT_BIND_SOCKET),
	GB_CONSTANT("CannotListen", "i", NET_CANNOT_LISTEN),
	GB_CONSTANT("CannotBindInterface", "i", NET_CANNOT_BIND_INTERFACE),
	GB_CONSTANT("CannotAuthenticate", "i", NET_CANNOT_AUTHENTICATE),
	/* SeverSocket, type */
	GB_CONSTANT("Internet", "i", NET_TYPE_INTERNET),
	GB_CONSTANT("Local", "i", NET_TYPE_LOCAL),
	GB_CONSTANT("Unix", "i", NET_TYPE_LOCAL),
	
	// Max path length for local sockets
	//GB_CONSTANT("MaxPathLength", "i", NET_UNIX_PATH_MAX),

	GB_STATIC_METHOD("Format", "s", CNET_Format, "(IpString)s[(Format)i(LeadZero)b]"),
	
	GB_END_DECLARE
};




