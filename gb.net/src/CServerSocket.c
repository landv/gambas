/***************************************************************************

  CServerSocket.c

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

#define __CSERVERSOCKET_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <net/if.h>
#include <errno.h>

#include "main.h"
#include "tools.h"

#include "CServerSocket.h"
#include "CSocket.h"

DECLARE_EVENT(EVENT_Connection);
DECLARE_EVENT(EVENT_Error);

static void srvsock_post_error(CSERVERSOCKET *_object)
{
	GB.Raise(THIS, EVENT_Error, 0);
	GB.Unref(POINTER(&_object));
}

static void CServerSocket_CallBack(int fd, int type, intptr_t lParam)
{
	int okval = 0;
	char *remote_ip;
	unsigned int clen;
	CSERVERSOCKET *_object = (CSERVERSOCKET*)lParam;
	
	if (SOCKET->status != NET_ACTIVE) return;

	SOCKET->status = NET_PENDING;
	clen = sizeof(struct sockaddr_in);
	THIS->Client = accept(SOCKET->socket, (struct sockaddr*)&THIS->so_client.in, &clen);
	
	if (THIS->Client == -1)
	{
		//close(THIS->Client);
		SOCKET->status = NET_ACTIVE;
		return;
	}
	
	if ((!THIS->iMaxConn) || (THIS->iCurConn < THIS->iMaxConn))
		okval = 1;
	
	if ((!THIS->iPause) && (okval))
	{
		remote_ip = GB.NewZeroString(inet_ntoa(THIS->so_client.in.sin_addr));
		GB.Raise(THIS, EVENT_Connection, 1, GB_T_STRING, remote_ip, GB.StringLength(remote_ip));
		GB.FreeString(&remote_ip);
	}
	
	if (SOCKET->status == NET_PENDING)
	{
		close(THIS->Client);
		THIS->Client = -1;
	}
	
	SOCKET->status = NET_ACTIVE;
}

static void CServerSocket_CallBackUnix(int fd, int type, intptr_t lParam)
{
	//int position=0;
	int okval=0;
	unsigned int ClientLen;
	CSERVERSOCKET *_object = (CSERVERSOCKET*)lParam;
	
	if ( SOCKET->status != NET_ACTIVE) return;

	SOCKET->status = NET_PENDING;
	ClientLen=sizeof(struct sockaddr_un);
	THIS->Client=accept(SOCKET->socket,(struct sockaddr*)&THIS->so_client.un,&ClientLen);
	if (THIS->Client == -1)
	{
		close(THIS->Client);
		SOCKET->status = NET_ACTIVE;
		return;
	}
	if ( (!THIS->iMaxConn) || (THIS->iCurConn < THIS->iMaxConn) ) okval=1;
	if ( (!THIS->iPause) && (okval) )
		GB.Raise(THIS,EVENT_Connection,1,GB_T_STRING,NULL,0);
	if  ( SOCKET->status == NET_PENDING) close(THIS->Client);
	SOCKET->status = NET_ACTIVE;

}


/*********************************************************
 Starts listening (TCP/UDP/UNIX)
 **********************************************************/
static int do_srvsock_listen(CSERVERSOCKET* _object,int mymax)
{
	int retval;
	int auth = 1;

	if (THIS->iPort == 0 && THIS->type == NET_TYPE_INTERNET)
		return 8;

	if (SOCKET->status > NET_INACTIVE) return 1;

	if (mymax<0) return 13;

	if (THIS->type == NET_TYPE_LOCAL && !THIS->sPath)
		return 7;

	if (THIS->type == NET_TYPE_INTERNET)
	{
		THIS->so_server.in.sin_family = AF_INET;
		THIS->so_server.in.sin_addr.s_addr = INADDR_ANY;
		THIS->so_server.in.sin_port = htons(THIS->iPort);
		SOCKET->socket = socket(PF_INET, SOCK_STREAM, 0);
	}
	else
	{
		unlink(THIS->sPath);
		THIS->so_server.un.sun_family = AF_UNIX;
		strcpy(THIS->so_server.un.sun_path, THIS->sPath);
		SOCKET->socket=socket(AF_UNIX, SOCK_STREAM,0);
	}

	if ( SOCKET->socket==-1 )
	{
		SOCKET->status = NET_CANNOT_CREATE_SOCKET;
		GB.Ref(THIS);
		GB.Post(srvsock_post_error,(intptr_t)THIS);
		return 2;
	}
	// thanks to Benoit : this option allows non-root users to reuse the
	// port after closing it and reopening it in a short interval of time.
	// However, If you are porting this component to other O.S., be careful,
	// as this is not a standard unix option
	setsockopt(SOCKET->socket, SOL_SOCKET, SO_REUSEADDR, &auth, sizeof(int));
	
	// Define specific interface: does not really work... :-/
	#ifdef SO_BINDTODEVICE
	if (THIS->interface)
	{
		if (setsockopt(SOCKET->socket, SOL_SOCKET, SO_BINDTODEVICE, THIS->interface, GB.StringLength(THIS->interface)))
		{
			fprintf(stderr, "unable to bind socket to interface: %s\n", strerror(errno));
			SOCKET->status = NET_CANNOT_BIND_INTERFACE;
			return 15;
		}
	}
	#endif
	
	SOCKET_update_timeout(SOCKET);
	//
	if (THIS->type == NET_TYPE_INTERNET)
		retval = bind(SOCKET->socket, (struct sockaddr *)&THIS->so_server.in, sizeof(struct sockaddr_in));
	else
		retval = bind(SOCKET->socket, (struct sockaddr *)&THIS->so_server.un, sizeof(struct sockaddr_un));
	
	if (retval == -1)
	{
		close(SOCKET->socket);
		SOCKET->status = NET_CANNOT_BIND_SOCKET;
		GB.Ref(THIS);
		GB.Post(srvsock_post_error,(intptr_t)THIS);
		return 10;
	}

	// Set socket to non-blocking mode
	SOCKET_set_blocking(SOCKET, FALSE);

	if ( listen(SOCKET->socket,mymax) == -1 )
	{
		close(SOCKET->socket);
		SOCKET->status = NET_CANNOT_LISTEN;
		GB.Ref(THIS);
		GB.Post(srvsock_post_error,(intptr_t)THIS);
		return 14;
	}
	THIS->iCurConn=0;
	THIS->iMaxConn=mymax;
	SOCKET->status = NET_ACTIVE;

	//CServerSocket_AssignCallBack((intptr_t)THIS,SOCKET->socket);
	if (THIS->type == NET_TYPE_INTERNET)
		GB.Watch (SOCKET->socket , GB_WATCH_READ , (void *)CServerSocket_CallBack,(intptr_t)THIS);
	else
		GB.Watch (SOCKET->socket , GB_WATCH_READ , (void *)CServerSocket_CallBackUnix,(intptr_t)THIS);
	
	return 0;
}

static void srvsock_listen(CSERVERSOCKET *_object, int max)
{
	switch(do_srvsock_listen(THIS, max))
	{
		case 1: GB.Error("Socket is already listening"); break;
		case 7: GB.Error("Path is not defined"); break;
		case 8: GB.Error("Port is not defined"); break;
		case 13: GB.Error("Invalid maximum number of connections"); break;
		case 15: GB.Error("Unable to bind socket to interface"); break;
		default: break;
	}
}

static void add_child(CSERVERSOCKET *_object, CSOCKET *child)
{
	*((CSOCKET **)GB.Add(&THIS->children)) = child;
	child->parent = THIS;
	GB.Ref(child);
}

static void unref_child_later(CSOCKET *child)
{
	GB.Unref(POINTER(&child));
}

static void remove_child(CSERVERSOCKET *_object, CSOCKET *child)
{
	int i;
	
	for (i = 0; i < GB.Count(THIS->children); i++)
	{
		if (THIS->children[i] == child)
		{
			child->parent = NULL;
			GB.Remove(&THIS->children, i, 1);
			GB.Post(unref_child_later, (intptr_t)child);
			return;
		}
	}
}

void CServerSocket_OnClose(void *child)
{
	CSERVERSOCKET *_object;

	if (!child) return;

	_object = (CSERVERSOCKET*)(((CSOCKET *)child)->parent);

	if (!THIS) return;
	
	remove_child(THIS, child);
	THIS->iCurConn--;
}

/***************************************************
 This property reflects current status of the
 socket (closed, listening...)
 ***************************************************/
BEGIN_PROPERTY(ServerSocket_Status)

	GB.ReturnInteger(SOCKET->status);

END_PROPERTY

/******************************************************************
 This property gets/sets the port to listen to (TCP or UDP sockets)
 ******************************************************************/
BEGIN_PROPERTY(ServerSocket_Port)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->iPort);
		return;
	}
	if (SOCKET->status > NET_INACTIVE)
	{
		GB.Error("Port cannot be changed when socket is active");
		return;
	}
	if ( (VPROP(GB_INTEGER)<1) || (VPROP(GB_INTEGER)>65535) )
	{
		GB.Error("Invalid port Value");
		return;
	}
	THIS->iPort=VPROP(GB_INTEGER);

END_PROPERTY


/***************************************************************
 This property gets/sets the address to listen to (UNIX socket)
 ***************************************************************/
BEGIN_PROPERTY(ServerSocket_Path)

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->sPath);
		return;
	}
	if (SOCKET->status > NET_INACTIVE)
	{
		GB.Error("Path cannot be changed while socket is active");
		return;
	}
	if (PLENGTH() > NET_UNIX_PATH_MAX)
	{
		GB.Error ("Path is too long");
		return;
	}
	GB.StoreString(PROP(GB_STRING), &THIS->sPath);

END_PROPERTY


BEGIN_PROPERTY(ServerSocket_Interface)

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->sPath);
	}
	else
	{
		if (SOCKET->status > NET_INACTIVE)
		{
			GB.Error("Interface cannot be changed while socket is active");
			return;
		}
		if (PLENGTH() > IFNAMSIZ)
		{
			GB.Error ("Interface name is too long");
			return;
		}

		GB.StoreString(PROP(GB_STRING), &THIS->interface);
	}

END_PROPERTY


/***************************************************************
 This property gets/sets the socket type (0 -> TCP, 1 -> UNIX)
 ***************************************************************/
BEGIN_PROPERTY ( ServerSocket_Type )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->type);
		return;
	}
	
	if (SOCKET->status > NET_INACTIVE)
	{
		GB.Error("Type cannot be changed when the socket is active");
		return;
	}
	
	switch(VPROP(GB_INTEGER))
	{
		case NET_TYPE_LOCAL: THIS->type = NET_TYPE_LOCAL; break;
		case NET_TYPE_INTERNET: THIS->type = NET_TYPE_INTERNET; break;
		default: GB.Error("Invalid socket type");
	}

END_PROPERTY


BEGIN_METHOD(ServerSocket_new, GB_STRING sPath; GB_INTEGER iMaxConn)

	char *buf = NULL;
	int nport = 0;
	int iMax;

	THIS->type = NET_TYPE_INTERNET;
	GB.NewArray(&THIS->children, sizeof(void *), 0);

	if (MISSING(sPath)) return;
	if (LENGTH(sPath) == 0) return;

	iMax = VARGOPT(iMaxConn, 0);
	
	switch(IsHostPath(STRING(sPath), LENGTH(sPath), &buf, &nport))
	{
		case 0:
			GB.Error("Invalid Host or Path");
			return;
		
		case 1:
			if (buf)
			{
				GB.Free(POINTER(&buf));
				GB.Error("Invalid Host");
				return;
			}
			if (nport<1)
			{
				GB.Error("Invalid Port");
				return;
			}

			THIS->type = NET_TYPE_INTERNET;
			THIS->iPort=nport;
			srvsock_listen(THIS, iMax);
			break;
			
		case 2:
			THIS->type = NET_TYPE_LOCAL;
			if (LENGTH(sPath) >NET_UNIX_PATH_MAX)
			{
				GB.Error ("Path is too long");
				return;
			}
			GB.StoreString(ARG(sPath), &THIS->sPath);
			break;
	}
	
END_METHOD

void close_server(CSERVERSOCKET *_object)
{
	CSOCKET *chd;

	if (SOCKET->status <= NET_INACTIVE) return;

	GB.Watch (SOCKET->socket , GB_WATCH_NONE , (void *)CServerSocket_CallBack,0);
	close(SOCKET->socket);
	SOCKET->status = NET_INACTIVE;

	while (GB.Count(THIS->children))
	{
		chd = THIS->children[0];
		if (chd->common.stream.desc) CSocket_stream_close(&chd->common.stream);
		remove_child(THIS, chd);
	}
}

BEGIN_METHOD_VOID(ServerSocket_free)

	close_server(THIS);
	GB.FreeArray(&THIS->children);
	GB.FreeString(&THIS->sPath);
	GB.FreeString(&THIS->interface);

END_METHOD

/********************************************************
 Stops listening (TCP/UDP/UNIX), and closes all client sockets associated
 to this server
 *********************************************************/
BEGIN_METHOD_VOID(ServerSocket_Close)

	close_server(THIS);

END_METHOD

/********************************************************
 Do not accept more connections until Resume is used
 *********************************************************/
BEGIN_METHOD_VOID(ServerSocket_Pause)

	THIS->iPause=1;

END_METHOD

/********************************************************
 Accept connections again
 *********************************************************/
BEGIN_METHOD_VOID(ServerSocket_Resume)

	THIS->iPause=0;

END_METHOD

BEGIN_METHOD(ServerSocket_Listen, GB_INTEGER MaxConn)

	srvsock_listen(THIS, VARGOPT(MaxConn, 0));

END_METHOD

/******************************************************************
 To accept a pending connection and delegate it to a Socket object
*******************************************************************/

BEGIN_METHOD_VOID(ServerSocket_Accept)

	CSOCKET *socket;
	struct sockaddr_in myhost;
	unsigned int mylen;

	if ( SOCKET->status != NET_PENDING)
	{
		GB.Error("No connection to accept");
		return; 
	}

	socket = GB.New(GB.FindClass("Socket"), "Socket", NULL);
	socket->common.socket = THIS->Client;
	socket->common.status = NET_CONNECTED;
	socket->OnClose = CServerSocket_OnClose;
	
	THIS->iCurConn++;
	
	GB.FreeString(&socket->sRemoteHostIP);
	GB.FreeString(&socket->sLocalHostIP);
	GB.FreeString(&socket->sPath);
	
	socket->iLocalPort = 0;
	socket->iPort = 0;
	socket->conn_type = THIS->type;
	
	if (THIS->type == NET_TYPE_INTERNET)
	{
		socket->sRemoteHostIP = GB.NewZeroString(inet_ntoa(THIS->so_client.in.sin_addr));
		socket->Host = GB.NewZeroString (inet_ntoa(THIS->so_client.in.sin_addr));
		mylen = sizeof(struct sockaddr);
		getsockname(socket->common.socket, (struct sockaddr*)&myhost, &mylen);
		socket->sLocalHostIP = GB.NewZeroString(inet_ntoa(myhost.sin_addr));
		socket->iLocalPort = ntohs(myhost.sin_port);
		socket->iPort = ntohs(THIS->so_client.in.sin_port);
		socket->iUsePort = ntohs(THIS->so_client.in.sin_port);
	}
	else
	{
		socket->conn_type=1;
		socket->sPath = GB.NewZeroString(THIS->sPath);
		socket->Path = GB.NewZeroString(THIS->sPath);
	}

	add_child(THIS, socket);

	CSOCKET_init_connected(socket);
	// Socket returned by accept is non-blocking by default
	GB.Stream.Block(&socket->common.stream, FALSE);
	//socket->stream._free[0]=(intptr_t)socket;

	GB.Ref(socket);
	GB.Post(CSocket_post_connected,(intptr_t)socket);
	SOCKET->status = NET_ACCEPTING;
	
	GB.ReturnObject((void*)socket);

END_METHOD

// BM: Enumeration of child sockets

BEGIN_METHOD_VOID(ServerSocket_next)

  int *index = (int *)GB.GetEnum();

  if (*index >= GB.Count(THIS->children))
    GB.StopEnum();
  else
  {
    GB.ReturnObject(THIS->children[*index]);
    (*index)++;
  }

END_METHOD

BEGIN_PROPERTY(ServerSocket_count)

	GB.ReturnInteger(GB.Count(THIS->children));

END_PROPERTY


/****************************************************************
 Here we declare public structure of the ServerSocket class
*****************************************************************/
GB_DESC CServerSocketDesc[] =
{
  GB_DECLARE("ServerSocket", sizeof(CSERVERSOCKET)),

  GB_EVENT("Connection", NULL, "(RemoteHostIP)s", &EVENT_Connection),
  GB_EVENT("Error", NULL,NULL, &EVENT_Error),

  GB_METHOD("_new", NULL, ServerSocket_new, "[(Path)s(MaxConn)i]"),
  GB_METHOD("_free", NULL, ServerSocket_free, NULL),
  GB_METHOD("Listen",NULL, ServerSocket_Listen, "[(MaxConn)i]"),
  GB_METHOD("Pause", NULL, ServerSocket_Pause, NULL),
  GB_METHOD("Resume", NULL, ServerSocket_Resume, NULL),
  GB_METHOD("Accept","Socket",ServerSocket_Accept,NULL),
  GB_METHOD("Close",NULL,ServerSocket_Close,NULL),

  GB_PROPERTY("Type","i",ServerSocket_Type),
  GB_PROPERTY("Path","s",ServerSocket_Path),
  GB_PROPERTY("Port", "i", ServerSocket_Port),
  GB_PROPERTY("Interface", "s", ServerSocket_Interface),
  GB_PROPERTY_READ("Status","i",ServerSocket_Status),

  GB_METHOD("_next", "Socket", ServerSocket_next, NULL),
  GB_PROPERTY_READ("Count", "i", ServerSocket_count),

	GB_PROPERTY("Timeout", "i", Socket_Timeout),

  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", "Type=Local,Path,Port"),
  GB_CONSTANT("_DefaultEvent", "s", "Connection"),

  GB_END_DECLARE
};




