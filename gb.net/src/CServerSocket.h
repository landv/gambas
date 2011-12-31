/***************************************************************************

  CServerSocket.h

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

#ifndef __CSERVERSOCKET_H
#define __CSERVERSOCKET_H

#include "gambas.h"
#include "CNet.h"
#include "CSocket.h"

#ifndef __CSERVERSOCKET_C

extern GB_DESC CServerSocketDesc[];

#else

#define THIS ((CSERVERSOCKET *)_object)
#define SOCKET (&THIS->common)

#endif


typedef 
	union
	{
		struct sockaddr_in in;
		struct sockaddr_un un;
	} 
	st_so_sock;

typedef
	struct
	{
		CSOCKET_COMMON common;
		int type;
		int iPort;
		char *sPath;
		int iPause;
		int iMaxConn;
		int iCurConn;
		st_so_sock so_server;
		st_so_sock so_client;
		int Client;
		CSOCKET **children;
		char *interface;
	}  
	CSERVERSOCKET;

#endif
