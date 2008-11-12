/***************************************************************************

  CSocket.c

  Network component

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

  This is the implementation of Socket Gambas Class

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

#define __CSOCKET_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>

#include "main.h"
#include "tools.h"


#include "CSocket.h"
#include "CServerSocket.h"
#include "CDnsClient.h"
#define MAX_CLIENT_BUFFER_SIZE 65536
#define UNIXPATHMAX 108

DECLARE_EVENT (SocketError);
DECLARE_EVENT (Closed);
DECLARE_EVENT (HostFound);
DECLARE_EVENT (Socket_Read);
DECLARE_EVENT (Connected);


GB_STREAM_DESC SocketStream = {
	open: CSocket_stream_open,
	close: CSocket_stream_close,
	read: CSocket_stream_read,
	write: CSocket_stream_write,
	seek: CSocket_stream_seek,
	tell: CSocket_stream_tell,
	flush: CSocket_stream_flush,
	eof: CSocket_stream_eof,
	lof: CSocket_stream_lof,
	handle: CSocket_stream_handle
};



/**********************************
 Routines to call events
 **********************************/
void CSocket_post_error(void *_object)
{

	GB.Raise(THIS,SocketError,0);
	GB.Unref(POINTER(&_object));
}

void CSocket_post_closed(void *_object)
{
	GB.Raise(THIS,Closed,0);
	GB.Unref(POINTER(&_object));
}

void CSocket_post_hostfound(void *_object)
{
	GB.Raise(THIS,HostFound,0);
	GB.Unref(POINTER(&_object));
}

void CSocket_post_connected(void *_object)
{
	GB.Raise(THIS,Connected,0);
	GB.Unref(POINTER(&_object));
}

void CSocket_post_data_available(void *_object)
{
	if (THIS->iStatus==7)	GB.Raise(THIS,Socket_Read,0);
	GB.Unref(POINTER(&_object));
}

/**************************************************
 This function is called by DnsClient to inform
  that it has finished its work
  *************************************************/
void CSocket_CallBackFromDns(void *_object)
{
	int NoBlock=1;
	int myval=0;

	if ( THIS->iStatus != 5) return;
	if ( !THIS->DnsTool->sHostIP)
	{
		THIS->iStatus=-6; /* error host not found */
		dns_close_all(THIS->DnsTool);
		GB.Unref(POINTER(&THIS->DnsTool));
		THIS->DnsTool=NULL;
		GB.Ref (THIS);
		GB.Post (CSocket_post_error,(intptr_t)THIS);
		if (THIS->OnClose) THIS->OnClose(_object);
		return;
	}

	GB.FreeString (&THIS->sRemoteHostIP);
	GB.NewString ( &THIS->sRemoteHostIP ,THIS->DnsTool->sHostIP,0);
	/* Let's turn socket to async mode */
	ioctl(THIS->Socket,FIONBIO,&NoBlock);
	/* Third, we connect the socket */
	THIS->Server.sin_family=AF_INET;
  	THIS->Server.sin_port=htons(THIS->iPort);
  	THIS->Server.sin_addr.s_addr =inet_addr(THIS->DnsTool->sHostIP);
  	bzero(&(THIS->Server.sin_zero),8);
	myval=connect(THIS->Socket,(struct sockaddr*)&(THIS->Server), sizeof(struct sockaddr));
	if (!myval || errno==EINPROGRESS) /* this is the good answer : connect in progress */
	{
		THIS->iStatus=6;
		GB.Watch (THIS->Socket,GB_WATCH_WRITE,(void *)CSocket_CallBackConnecting,(long)THIS);
	}
	else
	{
		GB.Watch (THIS->Socket , GB_WATCH_NONE , (void *)CSocket_CallBack,0);
		THIS->stream.desc=NULL;
		close(THIS->Socket);
		THIS->iStatus=0;
	}
	if (THIS->DnsTool)
  	{
		dns_close_all(THIS->DnsTool);
		GB.Unref(POINTER(&THIS->DnsTool));
		THIS->DnsTool=NULL;
  	}
	if ( THIS->iStatus<=0 )
	{
		THIS->iStatus=-3;
		GB.Ref (THIS);
		GB.Post (CSocket_post_error,(intptr_t)THIS);
		if (THIS->OnClose) THIS->OnClose((void*)THIS);
		return;
	}

	GB.Ref(THIS);
	GB.Post(CSocket_post_hostfound,(intptr_t)THIS);

}
/*******************************************************************
 This CallBack is used while waiting to finish a connection process
 ******************************************************************/
void CSocket_CallBackConnecting(int t_sock,int type,long param)
{
	struct sockaddr_in myhost;
	int mylen;
	void *_object = (void *)param;

	GB.Watch(THIS->Socket, GB_WATCH_NONE, (void *)CSocket_CallBackConnecting, 0);
	
	if (THIS->iStatus!=6) return;
	/****************************************************
	Checks if Connection was Stablished or there was
	an error trying to connect
	****************************************************/
	THIS->iStatus=CheckConnection(THIS->Socket);
	if (THIS->iStatus == 0)
	{
		GB.Watch (THIS->Socket , GB_WATCH_NONE , (void *)CSocket_CallBack,0);
		THIS->stream.desc=NULL;
		close(THIS->Socket);
		THIS->iStatus=-3;
		GB.Ref (THIS);
		GB.Post (CSocket_post_error,(intptr_t)THIS);
		if (THIS->OnClose) THIS->OnClose((void*)THIS);
		return;
	}
	if (THIS->iStatus != 7) return;
	// we obtain local IP and host
	mylen=sizeof(struct sockaddr);
	getsockname (THIS->Socket,(struct sockaddr*)&myhost,(socklen_t *)&mylen);
	THIS->iLocalPort=ntohs(myhost.sin_port);
	GB.FreeString( &THIS->sLocalHostIP);
	GB.NewString ( &THIS->sLocalHostIP ,inet_ntoa(myhost.sin_addr),0);

	//GB.Watch (THIS->Socket,GB_WATCH_NONE,(void *)CSocket_CallBack,(long)THIS);
	GB.Watch (THIS->Socket,GB_WATCH_READ,(void *)CSocket_CallBack,(long)THIS);

	THIS->stream.desc=&SocketStream;
	GB.Ref(THIS);
	GB.Post(CSocket_post_connected,(intptr_t)THIS);



}
/*******************************************************************
 This CallBack is used while socket is connected to remote host
 ******************************************************************/
void CSocket_CallBack(int t_sock,int type,long lParam)
{
	char buf[1];
	struct pollfd mypoll;
	int numpoll;
	struct timespec mywait;
	void *_object;

	/*	Just sleeping a little to reduce CPU waste	*/
	mywait.tv_sec=0;
	mywait.tv_nsec=100000;
	nanosleep(&mywait,NULL);
	/* is there data avilable or an error? */
	_object=(void*)lParam;

	if (THIS->iStatus!=7) return;

	mypoll.fd=t_sock;
	mypoll.events=POLLIN | POLLNVAL;
	mypoll.revents=0;
	numpoll=poll(&mypoll,1,0);
	if (numpoll<=0) return;
	/* there's data available */
	USE_MSG_NOSIGNAL(numpoll=recv(t_sock,(void*)buf,sizeof(char),MSG_PEEK | MSG_NOSIGNAL));
	if (!numpoll)
	{ /* socket error, no valid data received */
		GB.Watch (THIS->Socket , GB_WATCH_NONE , (void *)CSocket_CallBack,0);
		THIS->stream.desc=NULL;
		close(t_sock);
		THIS->iStatus=0;
		GB.Ref(THIS);
		GB.Post(CSocket_post_closed,(intptr_t)THIS);
		if (THIS->OnClose) THIS->OnClose((void*)THIS);
		return;
	}
	/******************************************************
	There's data available to read, so we'll raise event
	Socket_Read
	*******************************************************/

	GB.Ref(_object);
	GB.Post(CSocket_post_data_available,(intptr_t)THIS);


}


void CSocket_stream_internal_error(void *_object,int ncode)
{
	/* fatal socket error handling */
	GB.Watch (THIS->Socket,GB_WATCH_NONE,(void *)CSocket_CallBack,0);
	THIS->stream.desc=NULL;
	close(THIS->Socket);
	THIS->iStatus = ncode;
}

//////////////////////////////////////////////////////////////////////////////////
//################################################################################
/*********************************************************************************
	"PUBLIC" C/C++ INTERFACE
**********************************************************************************/
//################################################################################
//////////////////////////////////////////////////////////////////////////////////

/* not allowed methods */
int CSocket_stream_open(GB_STREAM *stream, const char *path, int mode, void *data){return -1;}
int CSocket_stream_seek(GB_STREAM *stream, int64_t pos, int whence){return -1;}
int CSocket_stream_tell(GB_STREAM *stream, int64_t *pos){return -1;}

int CSocket_stream_flush(GB_STREAM *stream)
{
	return 0; /* OK */
}

int CSocket_stream_handle(GB_STREAM *stream)
{
	void *_object = stream->tag;
	return THIS->Socket;
}

int CSocket_stream_close(GB_STREAM *stream)
{
  void *_object = stream->tag;

  if (!THIS ) return -1;

  if (THIS->DnsTool)
  {
	dns_close_all(THIS->DnsTool);
	GB.Unref(POINTER(&THIS->DnsTool));
	THIS->DnsTool=NULL;
  }
  if (THIS->iStatus > 0) /* if it's not connected, does nothing */
  {
	GB.Watch (THIS->Socket , GB_WATCH_NONE , (void *)CSocket_CallBack,0);
	stream->desc=NULL;
  	close(THIS->Socket);
  	THIS->iStatus=0;
  }
  if (THIS->OnClose) THIS->OnClose(_object);
  return 0;
}

int CSocket_stream_lof(GB_STREAM *stream, int64_t *len)
{
  void *_object = stream->tag;
	int bytes;

	*len=0;
	if (!THIS) return -1;

	if (ioctl(THIS->Socket,FIONREAD,&bytes))
	{
		CSocket_stream_internal_error(THIS,-4);
		if (THIS->OnClose) THIS->OnClose(_object);
		return -1;
	}
	*len=bytes;
	return 0;
}

int CSocket_stream_eof(GB_STREAM *stream)
{
  void *_object = stream->tag;
	int bytes;

	if (!THIS) return -1;

	if (ioctl(THIS->Socket,FIONREAD,&bytes))
	{
		CSocket_stream_internal_error(THIS,-4);
		if (THIS->OnClose) THIS->OnClose(_object);
		return -1;
	}
	if (!bytes) return -1;
	return 0;
}

int CSocket_stream_read(GB_STREAM *stream, char *buffer, int len)
{
  void *_object = stream->tag;
	int npos=-1;
	int NoBlock=0;
	int bytes;

	if (!THIS) return -1;

	if (ioctl(THIS->Socket,FIONREAD,&bytes))
	{
		CSocket_stream_internal_error(THIS,-4);
		if (THIS->OnClose) THIS->OnClose(_object);
		return -1;
	}
	if (bytes < len) return -1;

	ioctl(THIS->Socket,FIONBIO,&NoBlock);
	USE_MSG_NOSIGNAL(npos=recv(THIS->Socket,(void*)buffer,len*sizeof(char),MSG_NOSIGNAL));
	NoBlock++;
  	ioctl(THIS->Socket,FIONBIO,&NoBlock);
  	if (npos==len) return 0;
	if (npos<0)
	{
		CSocket_stream_internal_error(THIS,-4);
		if (THIS->OnClose) THIS->OnClose(_object);
		return -1;
	}
	if (THIS->OnClose) THIS->OnClose(_object);
	return -1;
}

int CSocket_stream_write(GB_STREAM *stream, char *buffer, int len)
{
  void *_object = stream->tag;
	int npos=-1;
	int NoBlock=0;

	if (!THIS) return -1;

	ioctl(THIS->Socket,FIONBIO,&NoBlock);
	USE_MSG_NOSIGNAL(npos=send(THIS->Socket,(void*)buffer,len*sizeof(char),MSG_NOSIGNAL));
	NoBlock++;
	ioctl(THIS->Socket,FIONBIO,&NoBlock);
	if (npos>=0) return 0;
	CSocket_stream_internal_error(THIS,-5);
	if (THIS->OnClose) THIS->OnClose(_object);
	return -1;
}



/**************************************************************************
 To start a UNIX connection
 **************************************************************************/
int CSocket_connect_unix(void *_object,char *sPath,int lenpath)
{
	int NoBlock=1;

	if ( THIS->iStatus > 0 ) return 1;
	if (!sPath) return 7;
	if ( (lenpath<1) || (lenpath>UNIXPATHMAX) ) return 7;

	GB.FreeString(&THIS->sRemoteHostIP);
  	GB.FreeString(&THIS->sLocalHostIP);

	THIS->UServer.sun_family=AF_UNIX;
	strcpy(THIS->UServer.sun_path,sPath);
  	if ( (THIS->Socket=socket(AF_UNIX,SOCK_STREAM,0))==-1 )
  	{
    		THIS->iStatus=-2;
		GB.Ref (THIS);
  		CSocket_post_error(_object); /* Unable to create socket */
		return 2;
  	}


  	GB.FreeString(&THIS->sPath);
	GB.NewString ( &THIS->sPath , THIS->UServer.sun_path ,0);
  	THIS->conn_type=1;
  	if (connect(THIS->Socket,(struct sockaddr*)&THIS->UServer,sizeof(struct sockaddr_un))==0)
  	{
		THIS->iStatus=7;
		ioctl(THIS->Socket,FIONBIO,&NoBlock);
		GB.Watch (THIS->Socket,GB_WATCH_WRITE,(void *)CSocket_CallBack,(long)THIS);
		THIS->stream.desc=&SocketStream;
                // $BM
                if (THIS->Host) GB.FreeString(&THIS->Host);
                if (THIS->Path) GB.FreeString(&THIS->Path);

                GB.NewString(&THIS->Path,sPath,0);
		GB.Ref (THIS);
		CSocket_post_connected(_object);

		return 0;
  	}
	/* Error */
	THIS->stream.desc=NULL;
	close(THIS->Socket);
	GB.FreeString(&THIS->sPath);
	THIS->iStatus=-3;

	GB.Ref (THIS);
	CSocket_post_error(_object); /* Unable to connect to remote host */

	return 3;

}
/**************************************************************************
 To start a TCP connection
 **************************************************************************/
int CSocket_connect_socket(void *_object,char *sHost,int lenhost,int myport)
{
	if ( THIS->iStatus > 0 ) return 1;
	if (!lenhost) return 9;
	if (!sHost)   return 9;
	if ( (myport<1) || (myport>65535) ) return 8;

  	GB.FreeString(&THIS->sRemoteHostIP);
  	GB.FreeString(&THIS->sLocalHostIP);


	if ( (THIS->Socket=socket(AF_INET,SOCK_STREAM,0))==-1 )
	{
		THIS->iStatus=-2;
		GB.Ref (THIS);
		CSocket_post_error(_object);
		return 2;
	}

	THIS->iPort=myport;
	THIS->conn_type=0;
	/******************************************
	Let's turn hostname into host IP
	*******************************************/
	if (!THIS->DnsTool)
	{
		GB.New(POINTER(&THIS->DnsTool),GB.FindClass("DnsClient"),NULL,NULL);
		THIS->DnsTool->CliParent=_object;
	}

  	if (THIS->DnsTool->iStatus > 0 ) dns_close_all(THIS->DnsTool);

  	dns_set_async_mode(1,THIS->DnsTool);
  	GB.FreeString (&(THIS->DnsTool->sHostName));
  	GB.NewString(&THIS->DnsTool->sHostName,sHost,lenhost);
  	THIS->DnsTool->finished_callback=CSocket_CallBackFromDns;
	/********************************************
	We start DNS lookup, when it is finished
	it will call to CSocket_CallBack_fromDns,
	and we'll continue there connection proccess
	********************************************/
	THIS->iStatus=5; /* looking for IP */
	dns_thread_getip(THIS->DnsTool);
	THIS->stream.desc=&SocketStream;
  	THIS->iUsePort=THIS->iPort;

        // $BM
        if (THIS->Host) GB.FreeString(&THIS->Host);
        if (THIS->Path) GB.FreeString(&THIS->Path);

	GB.NewString(&THIS->Host,sHost,0);
	return 0;

}

/**********************************************
This function is used to peek data from socket,
you have to pass 3 parameters:

_object-> CSocket object
buf    -> Data Buffer (you have to free it after using it!)
MaxLen -> 0 no limit, >0 max. data t read
***********************************************/
int CSocket_peek_data(void *_object,char **buf,int MaxLen)
{
  int retval=0;
  int NoBlock=0;
  int nread=0;
  int bytes=0;

  (*buf)=NULL;
  nread=ioctl(THIS->Socket,FIONREAD,&bytes); /* Is there anythig to read? */
  if (nread)
  	retval=-1;
  else
  	retval=0;
  if ( (!retval) && (bytes) ) /* if there's anything to receive */
  {
  	if (bytes > MAX_CLIENT_BUFFER_SIZE) bytes=MAX_CLIENT_BUFFER_SIZE;
	if (MaxLen >0 ) bytes=MaxLen;
	GB.Alloc((void**)buf,bytes);
	(*buf)[0]='\0';
  	ioctl(THIS->Socket,FIONBIO,&NoBlock);
	USE_MSG_NOSIGNAL(retval=recv(THIS->Socket,(void*)(*buf),bytes*sizeof(char),MSG_PEEK|MSG_NOSIGNAL));
	NoBlock++;
  	ioctl(THIS->Socket,FIONBIO,&NoBlock);
  }

  if (retval==-1)
  {
  	/* An error happened while trying to receive data : SOCKET ERROR */
	if (*buf)
	{
		GB.Free(POINTER(buf));
		buf=NULL;
	}
	GB.Watch (THIS->Socket , GB_WATCH_NONE , (void *)CSocket_CallBack,0);
	THIS->stream.desc=NULL;
	close(THIS->Socket);
	THIS->iStatus=-4;
	GB.Ref (THIS);
	CSocket_post_error(_object);
	return -1;

  }

  return retval;

}
/*****************************************************************************************
##########################################################################################
-------------------------SOCKET GAMBAS INTERFACE IMPLEMENTATION---------------------
##########################################################################################
******************************************************************************************/
/********************************************************************
Returns current Status of the socket (connected,connecting,closed)
*********************************************************************/
BEGIN_PROPERTY ( CSOCKET_Status )

  GB.ReturnInteger(THIS->iStatus);

END_PROPERTY

/********************************************************************
 Port to connect to. Can be 'Net.Local' for Unix sockets, or
 1-65535 for TCP sockets
 ********************************************************************/
BEGIN_PROPERTY(CSOCKET_Port)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->iUsePort);
		return;
	}

	if (THIS->iStatus>0)
	{
		GB.Error("Port property can not be changed while working");
		return;
	}
	if (VPROP(GB_INTEGER)<0)	{ THIS->iUsePort=0; return; }
	if (VPROP(GB_INTEGER)>65535) 	{ THIS->iUsePort=65535; return; }
	THIS->iUsePort=VPROP(GB_INTEGER);


END_PROPERTY
/*********************************************************************
	Host or Path to connect to. If 'Port' value is zero, this
	will be a local path, else, a remote host name
**********************************************************************/
// $BM

BEGIN_PROPERTY (CSOCKET_Host)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(THIS->Host,0);
		return;
	}
	GB.StoreString(PROP(GB_STRING), &THIS->Host);

END_PROPERTY

BEGIN_PROPERTY (CSOCKET_Path)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(THIS->Path,0);
		return;
	}
	GB.StoreString(PROP(GB_STRING), &THIS->Path);

END_PROPERTY
/********************************************************************
 Returns current TCP remote port (only when connected via TCP)
 *********************************************************************/
BEGIN_PROPERTY ( CSOCKET_RemotePort )

	if ( THIS->iStatus != 7   )   { GB.ReturnInteger(0); return; }
	if ( THIS->conn_type != 0 )   { GB.ReturnInteger(0); return; }
	GB.ReturnInteger(THIS->iPort);

END_PROPERTY

/********************************************************************
 Returns current TCP local port (only when connected via TCP)
 *********************************************************************/
BEGIN_PROPERTY ( CSOCKET_LocalPort )

	if ( THIS->iStatus != 7   )   { GB.ReturnInteger(0); return; }
	if ( THIS->conn_type != 0 )   { GB.ReturnInteger(0); return; }
	GB.ReturnInteger(THIS->iLocalPort);

END_PROPERTY

/***********************************************************************
 Returns current foreing host IP (only when connected via TCP)
 ***********************************************************************/
BEGIN_PROPERTY ( CSOCKET_RemoteHost )

  if ( THIS->iStatus==7 && (THIS->conn_type==0) )
  {
  	GB.ReturnString(THIS->sRemoteHostIP);
  	return;
  }
  GB.ReturnString(NULL);

END_PROPERTY

/***********************************************************************
 Returns current local host IP (only when connected via TCP)
 ***********************************************************************/
BEGIN_PROPERTY ( CSOCKET_LocalHost )

  if ( THIS->iStatus==7 && (THIS->conn_type==0) )
  {
  	GB.ReturnString(THIS->sLocalHostIP);
	return;
  }
  GB.ReturnString(NULL);

END_PROPERTY

// $BM
#if 0
/***********************************************************************
 Returns current Path (only when connected via UNIX socket)
 ***********************************************************************/
BEGIN_PROPERTY ( CSOCKET_Path )

	int mylen;
	char numtmp[6];
	char *strtmp;

	if ( (THIS->iStatus==7) && (THIS->conn_type==1)  )
	{
		GB.ReturnString(THIS->sPath);
		return;
	}

	if ( (THIS->iStatus==7) && (THIS->conn_type==0)  )
	{
		snprintf(numtmp,6,"%d",THIS->iPort);
		mylen=strlen(THIS->sRemoteHostIP);
		mylen++;
		mylen+=strlen(numtmp);
		mylen++;
		GB.Alloc((void**)&strtmp,sizeof(char)*mylen);
		strcpy(strtmp,THIS->sRemoteHostIP);
		strcat(strtmp,":");
		strcat(strtmp,numtmp);
		GB.ReturnNewString(strtmp,0);
		GB.Free((void**)&strtmp);
		return;
	}

	GB.ReturnString(NULL);

END_PROPERTY
#endif

/****************************************************
 Gambas object "Constructor"
 ****************************************************/
BEGIN_METHOD_VOID(CSOCKET_new)

  THIS->stream.tag = THIS;
  THIS->iUsePort = 80;

END_METHOD

/**************************************************
 Gambas object "Destructor"
 **************************************************/
BEGIN_METHOD_VOID(CSOCKET_free)

  if (THIS->DnsTool)
  {
	dns_close_all(THIS->DnsTool);
	GB.Unref(POINTER(&THIS->DnsTool));
	THIS->DnsTool=NULL;
  }
  if (THIS->iStatus>0)
  {
  	GB.Watch (THIS->Socket , GB_WATCH_NONE , (void *)CSocket_CallBack,0);
	THIS->stream.desc=NULL;
	close(THIS->Socket);
	if (THIS->OnClose) THIS->OnClose((void*)THIS);
  	THIS->iStatus=0;

  }
  GB.FreeString(&THIS->sPath);
  GB.FreeString(&THIS->sLocalHostIP);
  GB.FreeString(&THIS->sRemoteHostIP);
  // $BM
  GB.FreeString(&THIS->Host);
  GB.FreeString(&THIS->Path);

END_METHOD

BEGIN_METHOD_VOID(CSOCKET_exit)

END_METHOD


/*************************************************************
 To Peek data arrived from the other side of the socket
 **************************************************************/
BEGIN_METHOD_VOID(CSOCKET_Peek)

  char *buf=NULL;
  int retval=0;

  if (THIS->iStatus != 7) /* if socket is not connected we can't receive anything */
  {
      GB.Error("Socket is not active. Connect it first");
      return;
  }

  retval=CSocket_peek_data(_object,&buf,0);

  if (retval==-1)
  {
  	/* An error happened while trying to receive data : SOCKET ERROR */
	if (buf) GB.Free(POINTER(&buf));
	GB.ReturnNewString(NULL,0);
	return;

  }

  if ( retval>0)
  	GB.ReturnNewString (buf,retval);
  else
  	GB.ReturnNewString (NULL,0);

  if (buf) GB.Free(POINTER(&buf));

END_METHOD



/**************************************************************************
 To start a TCP or UNIX connection
 **************************************************************************/

// $BM

BEGIN_METHOD(CSOCKET_Connect,GB_STRING HostOrPath;GB_INTEGER Port;)

  int port;
  int err;

  port = VARGOPT(Port, THIS->iUsePort);

  if (!port)
  {
    if (MISSING(HostOrPath))
      err = CSocket_connect_unix(_object,THIS->Path,GB.StringLength(THIS->Path));
    else
      err = CSocket_connect_unix(_object,STRING(HostOrPath),LENGTH(HostOrPath));
  }
  else
  {
    if (MISSING(HostOrPath))
      err = CSocket_connect_socket(_object,THIS->Host,GB.StringLength(THIS->Host),port);
    else
      err = CSocket_connect_socket(_object,STRING(HostOrPath),LENGTH(HostOrPath),port);
  }

  switch (err)
  {
    case 1: GB.Error("Socket is already connected"); return;
    case 2: GB.Error("Invalid path length"); return;
    case 8: GB.Error("Port value out of range"); return;
    case 9: GB.Error("Invalid host name"); return;
  }

END_METHOD


/**********************************************************
 Here we declare public structure of Socket Class
 ***********************************************************/

GB_DESC CSocketDesc[] =
{

  GB_DECLARE("Socket", sizeof(CSOCKET)),

  GB_INHERITS("Stream"),


  GB_EVENT("Error", NULL, NULL, &SocketError),
  GB_EVENT("Read", NULL, NULL, &Socket_Read),
  GB_EVENT("Ready", NULL, NULL, &Connected),
  GB_EVENT("Closed", NULL, NULL, &Closed),
  GB_EVENT("Found", NULL, NULL, &HostFound),

  GB_STATIC_METHOD ( "_exit" , NULL , CSOCKET_exit , NULL ),

  GB_METHOD("_new", NULL, CSOCKET_new, NULL),
  GB_METHOD("_free", NULL, CSOCKET_free, NULL),
  GB_METHOD("Peek", "s", CSOCKET_Peek, NULL),
  GB_METHOD("Connect",NULL, CSOCKET_Connect,"[(HostOrPath)s(Port)i]"),

  GB_PROPERTY_READ("Status", "i", CSOCKET_Status),
  GB_PROPERTY_READ("RemotePort", "i", CSOCKET_RemotePort),
  GB_PROPERTY_READ("LocalPort", "i", CSOCKET_LocalPort),
  GB_PROPERTY_READ("RemoteHost", "s", CSOCKET_RemoteHost),
  GB_PROPERTY_READ("LocalHost","s",CSOCKET_LocalHost),

  GB_PROPERTY("Host","s",CSOCKET_Host),
  GB_PROPERTY("Path","s",CSOCKET_Path),
  GB_PROPERTY("Port","i",CSOCKET_Port),

  GB_CONSTANT("_Properties", "s", "Host,Path,Port=80"),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),

  GB_END_DECLARE
};




