/***************************************************************************

  CNet.h

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

#ifndef __CNET_H
#define __CNET_H

#include "gambas.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#ifndef __CNET_C

extern GB_DESC CNetDesc[];

#else

#define THIS ((CNET *)_object)

#endif

typedef
	union
	{	
		struct sockaddr a;
		struct sockaddr_in in;
		struct sockaddr_un un;
	} 
	NET_ADDRESS;

#define NET_UNIX_PATH_MAX 108

enum
{
	NET_INACTIVE = 0,
	NET_ACTIVE = 1,
	NET_PENDING = 2,
	NET_ACCEPTING = 3,
	NET_RECEIVING_DATA = 4,
	NET_SEARCHING = 5,
	NET_CONNECTING = 6,
	NET_CONNECTED = 7,
	NET_CANNOT_CREATE_SOCKET = -2,
	NET_CONNECTION_REFUSED = -3,
	NET_CANNOT_READ = -4,
	NET_CANNOT_WRITE = -5,
	NET_HOST_NOT_FOUND = -6,
	NET_CANNOT_BIND_SOCKET = -10,
	NET_CANNOT_LISTEN = -14,
	NET_CANNOT_BIND_INTERFACE = -15,
	NET_CANNOT_AUTHENTICATE = -16 // For gb.net.pop3
};

enum
{
	NET_TYPE_LOCAL = 0,
	NET_TYPE_INTERNET = 1
};

size_t NET_get_address_size(NET_ADDRESS *addr);

#endif
