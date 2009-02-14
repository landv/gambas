/***************************************************************************

  CServerSocket.h

  Network component

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#ifndef __CSERVERSOCKET_H
#define __CSERVERSOCKET_H

#include "gambas.h"
#include "CNet.h"

#ifndef __CSERVERSOCKET_C
extern GB_DESC CServerSocketDesc[];

#else

#define THIS ((CSERVERSOCKET *)_object)

#endif


typedef union
{
	struct sockaddr_in in;
	struct sockaddr_un un;
} st_so_sock;

typedef  struct
{
    GB_BASE ob;
    int iSockType;
    int iPort;
    char *sPath;
    int ServerSocket;
    int iStatus;
    int iPause;
    int iMaxConn;
    int iCurConn;
    //struct sockaddr_in Server; /* Struct for TCP connections */
    //struct sockaddr_un UServer; /* Struct for UNIX connections */
    st_so_sock so_server;
    st_so_sock so_client;
    int Client;
    void **children;
    int nchildren;
}  CSERVERSOCKET;

int srvsock_listen(CSERVERSOCKET* mythis,int mymax);
void srvsock_post_error(CSERVERSOCKET* mythis);

#endif
