/***************************************************************************

  CServerSocket.c

  Network component

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

  This is the implementation of the SocketServer class

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

#define __CSERVERSOCKET_C


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "main.h"
#include "tools.h"

#include "CServerSocket.h"
#include "CSocket.h"

#define UNIXPATHMAX 108


DECLARE_EVENT (Connection);
DECLARE_EVENT (CSERVERSOCKET_Error);

void srvsock_post_error(CSERVERSOCKET* mythis)
{
	GB.Raise((void*)mythis,CSERVERSOCKET_Error,0);
	GB.Unref(POINTER(&mythis));
}

// BM: Fix bug in allocations

void CServerSocket_NewChild(CSERVERSOCKET *mythis,void *cli_obj)
{
	if (mythis->nchildren++)
		GB.Realloc ( POINTER(&mythis->children),mythis->nchildren * sizeof(*mythis->children));
	else
		GB.Alloc   ( POINTER(&mythis->children),mythis->nchildren * sizeof(*mythis->children));

	mythis->children[mythis->nchildren-1]=cli_obj;
}

void CServerSocket_DeleteChild(CSERVERSOCKET *mythis,void *cli_obj)
{
	int myloop;
	int myloop2;

	if (!mythis->nchildren) return;

	for (myloop=0;myloop<mythis->nchildren;myloop++)
	{
		if (mythis->children[myloop]==cli_obj)
		{
			for (myloop2=myloop;myloop2<(mythis->nchildren-1);myloop2++)
				mythis->children[myloop2]=mythis->children[myloop2+1];
			if ( --mythis->nchildren)
			{
				GB.Realloc ( POINTER(&mythis->children),mythis->nchildren * sizeof(*mythis->children));
			}
			else
			{
				GB.Free (POINTER(&mythis->children));
				mythis->children=NULL;
			}
			return;
		}
	}
}

void CServerSocket_OnClose(void *sck)
{
	CSOCKET *chd=(CSOCKET*)sck;
	CSERVERSOCKET *mythis;

	if (!chd) return;
	if (!chd->c_parent) return;
	CServerSocket_DeleteChild(chd->c_parent,sck);
	mythis=(CSERVERSOCKET*)chd->c_parent;
	mythis->iCurConn--;

}

void CServerSocket_CallBack(int fd,int type,intptr_t lParam)
{
	int okval=0;
	char *rem_ip_buf;
	unsigned int ClientLen;
	CSERVERSOCKET *mythis;

	mythis=(CSERVERSOCKET*)lParam;
	if ( mythis->iStatus != 1) return;

	mythis->iStatus=2;
	ClientLen=sizeof(struct sockaddr_in);
	mythis->Client=accept(mythis->ServerSocket,(struct sockaddr*)&mythis->so_client.in,&ClientLen);
	if (mythis->Client == -1)
	{
		close(mythis->Client);
		mythis->iStatus=1;
		return;
	}
	rem_ip_buf=inet_ntoa(mythis->so_client.in.sin_addr);
	if ( (!mythis->iMaxConn) || (mythis->iCurConn < mythis->iMaxConn) ) okval=1;
	if ( (!mythis->iPause) && (okval) )
		GB.Raise(mythis,Connection,1,GB_T_STRING,rem_ip_buf,0);
	if  ( mythis->iStatus == 2) close(mythis->Client);
	mythis->iStatus=1;
}

void CServerSocket_CallBackUnix(int fd,int type,intptr_t lParam)
{
	//int position=0;
	int okval=0;
	unsigned int ClientLen;
	CSERVERSOCKET *mythis;

	mythis=(CSERVERSOCKET*)lParam;
	if ( mythis->iStatus != 1) return;

	mythis->iStatus=2;
	ClientLen=sizeof(struct sockaddr_un);
	mythis->Client=accept(mythis->ServerSocket,(struct sockaddr*)&mythis->so_client.un,&ClientLen);
	if (mythis->Client == -1)
	{
		close(mythis->Client);
		mythis->iStatus=1;
		return;
	}
	if ( (!mythis->iMaxConn) || (mythis->iCurConn < mythis->iMaxConn) ) okval=1;
	if ( (!mythis->iPause) && (okval) )
		GB.Raise(mythis,Connection,1,GB_T_STRING,NULL,0);
	if  ( mythis->iStatus == 2) close(mythis->Client);
	mythis->iStatus=1;

}


/***************************************************
 This property reflects current status of the
 socket (closed, listening...)
 ***************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_MaxPathLength )

	GB.ReturnInteger(UNIXPATHMAX);

END_PROPERTY
/***************************************************
 This property reflects current status of the
 socket (closed, listening...)
 ***************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Status )

	GB.ReturnInteger(THIS->iStatus);

END_PROPERTY

/******************************************************************
 This property gets/sets the port to listen to (TCP or UDP sockets)
 ******************************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Port )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->iPort);
		return;
	}
	if (THIS->iStatus>0)
	{
		GB.Error("Port value can not be changed when socket is active");
		return;
	}
	if ( (VPROP(GB_INTEGER)<1) || (VPROP(GB_INTEGER)>65535) )
	{
		GB.Error("Invalid Port Value");
		return;
	}
	THIS->iPort=VPROP(GB_INTEGER);

END_PROPERTY


/***************************************************************
 This property gets/sets the address to listen to (UNIX socket)
 ***************************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Path )

	char *tmpstr=NULL;

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->sPath);
		return;
	}
	if (THIS->iStatus>0)
	{
		GB.Error("Path value can not be changed when socket is active");
		return;
	}
	tmpstr=GB.ToZeroString ( PROP(GB_STRING) );
	if ( (strlen(tmpstr)<1) || (strlen(tmpstr)>UNIXPATHMAX) )
	{
		GB.Error ("Invalid path length");
		return;
	}
	GB.StoreString(PROP(GB_STRING), &THIS->sPath);

END_PROPERTY


/***************************************************************
 This property gets/sets the socket type (0 -> TCP, 1 -> UNIX)
 ***************************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Type )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->iSockType);
		return;
	}
	if (THIS->iStatus>0)
	{
		GB.Error("Socket Type can not be changed when socket is active");
		return;
	}
	if ( (VPROP(GB_INTEGER)<0) || (VPROP(GB_INTEGER)>1) )
	{
		GB.Error("Invalid Socket Type Value");
		return;
	}
	THIS->iSockType=VPROP(GB_INTEGER);

END_PROPERTY
/***********************************************
 Gambas object "Constructor"
 ***********************************************/
BEGIN_METHOD(CSERVERSOCKET_new,GB_STRING sPath;GB_INTEGER iMaxConn;)

	int retval;
	char *buf=NULL;
	int nport=0;
	int iMax=0;

	THIS->iPort=0;
	THIS->iStatus=0;
	THIS->sPath=NULL;
	THIS->iPause=0;
	THIS->iMaxConn=0;
	THIS->iCurConn=0;
	THIS->iSockType=1;
	THIS->children=NULL;
	THIS->nchildren=0;

	if (MISSING(sPath)) return;
	if (!STRING(sPath)) return;

	if (!MISSING(iMaxConn)) iMax=VARG(iMaxConn);
	retval=IsHostPath(STRING(sPath),&buf,&nport);
	if (!retval)
	{
		GB.Error("Invalid Host / Path string");
		return;
	}
	if (retval==2)
	{
		THIS->iSockType=0;
		buf=GB.ToZeroString ( (GB_STRING*)STRING(sPath) );
		if ( (strlen(buf)<1) || (strlen(buf)>UNIXPATHMAX) )
		{
			GB.Error ("Invalid path length");
			return;
		}
		GB.StoreString((GB_STRING*)STRING(sPath),&THIS->sPath);
		return;
	}
	else
	{
		if (buf)
		{
			GB.Free(POINTER(&buf));
			GB.Error("Invalid Host String");
			return;
		}
		if (nport<1)
		{
			GB.Error("Invalid Port value");
			return;
		}

		THIS->iSockType=1;
		THIS->iPort=nport;
	}

	switch(srvsock_listen(THIS,iMax))
	{
		case 1:
			GB.Error("Socket is already listening");
			return;
		case 7:
			GB.Error ("You must define Path");
			return;
		case 8:
			GB.Error ("Error. You must define port");
			return;
		case 13:
			GB.Error ("Invalid maximun connections value");
			return;
	}


END_METHOD



void close_server(CSERVERSOCKET *mythis)
{
	CSOCKET *chd;

	if (mythis->iStatus <= 0) return;

	GB.Watch (mythis->ServerSocket , GB_WATCH_NONE , (void *)CServerSocket_CallBack,0);
	close(mythis->ServerSocket);
	mythis->iStatus=0;

	if (!mythis->nchildren) return;

	while(mythis->nchildren)
	{
		chd=(CSOCKET*)mythis->children[0];
		if (chd->stream.desc) CSocket_stream_close(&chd->stream);
		CServerSocket_DeleteChild(mythis,(void*)chd);
	}

}
/*************************************************
 Gambas object "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_free)


	close_server(THIS);
	GB.FreeString(&THIS->sPath);

END_METHOD

/********************************************************
 Stops listening (TCP/UDP/UNIX), and closes all client sockets associated
 to this server
 *********************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Close)

	close_server(THIS);


END_METHOD
/********************************************************
 Do not accept more connections until Resume is used
 *********************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Pause)

	THIS->iPause=1;


END_METHOD
/********************************************************
 Accept connections again
 *********************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Resume)

	THIS->iPause=0;


END_METHOD

/*********************************************************
 Starts listening (TCP/UDP/UNIX)
 **********************************************************/
int srvsock_listen(CSERVERSOCKET* mythis,int mymax)
{
	int NoBlock=1;
	int retval;
	int auth = 1;

	if ( (!mythis->iPort) && (mythis->iSockType) ) return 8;

	if ( mythis->iStatus >0 ) return 1;

	if (mymax<0) return 13;

	if ( (!mythis->iSockType) && (!mythis->sPath) ) return 7;


	if (mythis->iSockType)
	{
		mythis->so_server.in.sin_family=AF_INET;
		mythis->so_server.in.sin_addr.s_addr=INADDR_ANY;
		mythis->so_server.in.sin_port=htons(mythis->iPort);
		mythis->ServerSocket=socket(PF_INET,SOCK_STREAM,0);
	}
	else
	{
		unlink(mythis->sPath);
		mythis->so_server.un.sun_family=AF_UNIX;
		strcpy(mythis->so_server.un.sun_path,mythis->sPath);
		mythis->ServerSocket=socket(AF_UNIX,SOCK_STREAM,0);
	}

	if ( mythis->ServerSocket==-1 )
	{
		mythis->iStatus=-2;
		GB.Ref(mythis);
		GB.Post(srvsock_post_error,(intptr_t)mythis);
		return 2;
	}
	// thanks to Benoit : this option allows non-root users to reuse the
	// port after closing it and reopening it in a short interval of time.
	// However, If you are porting this component to other O.S., be careful,
	// as this is not a standard unix option
	setsockopt(mythis->ServerSocket, SOL_SOCKET, SO_REUSEADDR, &auth, sizeof(int));
	//
	if (mythis->iSockType)
		retval=bind(mythis->ServerSocket,(struct sockaddr*)&mythis->so_server.in, \
			sizeof(struct sockaddr_in));
	else
		retval=bind(mythis->ServerSocket,(struct sockaddr*)&mythis->so_server.un, \
			sizeof(struct sockaddr_un));
	if (retval==-1)
	{
		close(mythis->ServerSocket);
		mythis->iStatus=-10;
		GB.Ref(mythis);
		GB.Post(srvsock_post_error,(intptr_t)mythis);
		return 10;
	}

	ioctl(mythis->ServerSocket,FIONBIO,&NoBlock);

	if ( listen(mythis->ServerSocket,mymax) == -1 )
	{
		close(mythis->ServerSocket);
		mythis->iStatus=-14;
		GB.Ref(mythis);
		GB.Post(srvsock_post_error,(intptr_t)mythis);
		return 14;
	}
	mythis->iCurConn=0;
	mythis->iMaxConn=mymax;
	mythis->iStatus=1;

	//CServerSocket_AssignCallBack((intptr_t)mythis,mythis->ServerSocket);
	if (mythis->iSockType)
		GB.Watch (mythis->ServerSocket , GB_WATCH_READ , (void *)CServerSocket_CallBack,(intptr_t)mythis);
	else
		GB.Watch (mythis->ServerSocket , GB_WATCH_READ , (void *)CServerSocket_CallBackUnix,(intptr_t)mythis);
	return 0;
}

BEGIN_METHOD(CSERVERSOCKET_Listen,GB_INTEGER MaxConn;)

	int retval;
	int mymax=0;
 	if (!MISSING(MaxConn)) mymax=VARG(MaxConn);
	retval=srvsock_listen(THIS,mymax);
	switch(retval)
	{
		case 1:
			GB.Error("Socket is already listening");
			return;
		case 7:
			GB.Error ("You must define Path");
			return;
		case 8:
			GB.Error ("Error. You must define port");
			return;
		case 13:
			GB.Error ("Invalid maximun connections value");
			return;
	}

END_METHOD

/******************************************************************
 To accept a pending connection and delegate it to a Socket object
*******************************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Accept)

 CSOCKET *cli_obj;
 struct sockaddr_in myhost;
 unsigned int mylen;

 if ( THIS->iStatus != 2){ GB.Error("No connection to accept");return; }

 GB.New(POINTER(&cli_obj),GB.FindClass("Socket"),"Socket",NULL);
 cli_obj->Socket=THIS->Client;
 cli_obj->iStatus=7;
 cli_obj->c_parent=(void*)THIS;
 cli_obj->OnClose=CServerSocket_OnClose;
 THIS->iCurConn++;
 GB.FreeString ( &cli_obj->sRemoteHostIP);
 GB.FreeString ( &cli_obj->sLocalHostIP);
 GB.FreeString ( &cli_obj->sPath);
 cli_obj->iLocalPort=0;
 cli_obj->iPort=0;
 cli_obj->conn_type=0;
 if (THIS->iSockType)
 {
 	GB.NewString ( &cli_obj->sRemoteHostIP , inet_ntoa(THIS->so_client.in.sin_addr) ,0);
 	GB.NewString ( &cli_obj->Host, inet_ntoa(THIS->so_client.in.sin_addr) ,0);
	mylen=sizeof(struct sockaddr);
 	getsockname (cli_obj->Socket,(struct sockaddr*)&myhost,&mylen);
 	GB.NewString ( &cli_obj->sLocalHostIP , inet_ntoa(myhost.sin_addr) ,0);
 	cli_obj->iLocalPort=ntohs(myhost.sin_port);
 	cli_obj->iPort=ntohs(THIS->so_client.in.sin_port);
	cli_obj->iUsePort=ntohs(THIS->so_client.in.sin_port);
 }
 else
 {
 	cli_obj->conn_type=1;
 	GB.NewString ( &cli_obj->sPath,THIS->sPath,0);
	GB.NewString ( &cli_obj->Path,THIS->sPath,0);
 }

 cli_obj->stream.desc=&SocketStream;
 cli_obj->stream._free[0]=(intptr_t)cli_obj;
 GB.Watch (cli_obj->Socket, GB_WATCH_READ, (void *)CSocket_CallBack,(intptr_t)cli_obj);

 CServerSocket_NewChild(THIS,cli_obj);

 GB.Ref(cli_obj);
 GB.Post(CSocket_post_connected,(intptr_t)cli_obj);
 THIS->iStatus=3;
 GB.ReturnObject((void*)cli_obj);

END_METHOD

// BM: Enumeration of child sockets

BEGIN_METHOD_VOID(CSERVERSOCKET_next)

  int *index = (int *)GB.GetEnum();

  if (*index >= THIS->nchildren)
    GB.StopEnum();
  else
  {
    GB.ReturnObject(THIS->children[*index]);
    (*index)++;
  }

END_METHOD

BEGIN_PROPERTY(CSERVERSOCKET_count)

	GB.ReturnInteger(THIS->nchildren);

END_PROPERTY


/****************************************************************
 Here we declare public structure of the ServerSocket class
*****************************************************************/
GB_DESC CServerSocketDesc[] =
{

  GB_DECLARE("ServerSocket", sizeof(CSERVERSOCKET)),

  GB_EVENT("Connection", NULL, "(RemoteHostIP)s", &Connection),
  GB_EVENT("Error", NULL,NULL, &CSERVERSOCKET_Error),

  GB_METHOD("_new", NULL, CSERVERSOCKET_new,"[(Path)s(MaxConn)i]"),
  GB_METHOD("_free", NULL, CSERVERSOCKET_free, NULL),
  GB_METHOD("Listen",NULL, CSERVERSOCKET_Listen, "[(MaxConn)i]"),
  GB_METHOD("Pause", NULL, CSERVERSOCKET_Pause, NULL),
  GB_METHOD("Resume", NULL, CSERVERSOCKET_Resume, NULL),
  GB_METHOD("Accept","Socket",CSERVERSOCKET_Accept,NULL),
  GB_METHOD("Close",NULL,CSERVERSOCKET_Close,NULL),

  GB_PROPERTY("Type","i<Net,Internet,Local>",CSERVERSOCKET_Type),
  GB_PROPERTY("Path","s",CSERVERSOCKET_Path),
  GB_PROPERTY("Port", "i", CSERVERSOCKET_Port),
  GB_PROPERTY_READ("Status","i",CSERVERSOCKET_Status),
  GB_STATIC_PROPERTY_READ("MaxPathLength","i",CSERVERSOCKET_MaxPathLength),

  GB_METHOD("_next", "Socket", CSERVERSOCKET_next, NULL),
  GB_PROPERTY_READ("Count", "i", CSERVERSOCKET_count),


  GB_CONSTANT("_Properties", "s", "Type=0,Path,Port"),
  GB_CONSTANT("_DefaultEvent", "s", "Connection"),

  GB_END_DECLARE
};




