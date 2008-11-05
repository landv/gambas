/***************************************************************************

  CUdpSocket.c

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

#define __CUDPSOCKET_C
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "main.h"
#include "tools.h"

#include "CUdpSocket.h"


GB_STREAM_DESC UdpSocketStream = {
	open: CUdpSocket_stream_open,
	close: CUdpSocket_stream_close,
	read: CUdpSocket_stream_read,
	write: CUdpSocket_stream_write,
	seek: CUdpSocket_stream_seek,
	tell: CUdpSocket_stream_tell,
	flush: CUdpSocket_stream_flush,
	eof: CUdpSocket_stream_eof,
	lof: CUdpSocket_stream_lof,
	handle: CUdpSocket_stream_handle
};


DECLARE_EVENT (CUDPSOCKET_Read);
DECLARE_EVENT (CUDPSOCKET_SocketError);

void CUdpSocket_post_data(intptr_t Param)
{
	CUDPSOCKET *t_obj;
	t_obj=(CUDPSOCKET*)Param;
	GB.Raise(t_obj,CUDPSOCKET_Read,0);
	GB.Unref(POINTER(&t_obj));
}
void CUdpSocket_post_error(intptr_t Param)
{
	CUDPSOCKET *t_obj;
	t_obj=(CUDPSOCKET*)Param;
	GB.Raise(t_obj,CUDPSOCKET_SocketError,0);
	GB.Unref(POINTER(&t_obj));
}

static void clear_buffer(CUDPSOCKET *_object)
{
	if (THIS->buffer)
	{
		GB.Free(POINTER(&THIS->buffer));
		THIS->buffer_pos = 0;
		THIS->buffer_len = 0;
	}
}

static void fill_buffer(CUDPSOCKET *_object)
{
	struct sockaddr_in host;
	socklen_t host_len;
	int ret, block;
	char buffer[1];
	
	fprintf(stderr, "fill_buffer\n");
	
	clear_buffer(THIS);
	
	block = GB.Stream.Block(&THIS->stream, TRUE);
	USE_MSG_NOSIGNAL(ret = recvfrom(THIS->Socket, (void*)buffer, sizeof(char), MSG_PEEK | MSG_NOSIGNAL, (struct sockaddr*)&host, &host_len));
	GB.Stream.Block(&THIS->stream, block);
	
	if (ioctl(THIS->Socket, FIONREAD, &THIS->buffer_len))
		return;
	
	fprintf(stderr, "buffer_len = %d\n", THIS->buffer_len);
	
	if (!THIS->buffer_len)
		return;
	
	GB.Alloc(POINTER(&THIS->buffer), THIS->buffer_len);
	
	host_len = sizeof(host);
	USE_MSG_NOSIGNAL(ret = recvfrom(THIS->Socket, (void *)THIS->buffer, THIS->buffer_len, MSG_NOSIGNAL, (struct sockaddr*)&host, &host_len));

	fprintf(stderr, "recvfrom() -> %d\n", ret);
	
	if (ret < 0)
	{
		CUdpSocket_stream_close(&THIS->stream);
		THIS->iStatus=-4;
		return;
	}

	THIS->sport = ntohs(host.sin_port);
	GB.FreeString(&THIS->shost);
	GB.NewString (&THIS->shost , inet_ntoa(host.sin_addr) ,0);
}

void CUdpSocket_CallBack(int t_sock,int type, intptr_t param)
{
	//struct sockaddr_in t_test;
	//unsigned int t_test_len;
	struct timespec mywait;
	CUDPSOCKET *_object = (CUDPSOCKET *)param;

	/*	Just sleeping a little to reduce CPU waste	*/
	mywait.tv_sec=0;
	mywait.tv_nsec=100000;
	nanosleep(&mywait,NULL);

	if (THIS->iStatus<=0) return;

	//t_test.sin_port=0;
	//t_test_len=sizeof(struct sockaddr);

	//USE_MSG_NOSIGNAL(numpoll=recvfrom(t_sock,POINTER(buf), sizeof(char), MSG_PEEK | MSG_NOSIGNAL, (struct sockaddr*)&t_test, &t_test_len));
	fill_buffer(THIS);
	
	if (THIS->buffer)
	{
		GB.Ref((void*)THIS);
		GB.Post(CUdpSocket_post_data, (intptr_t)THIS);
	}
}
/* not allowed methods */
int CUdpSocket_stream_open(GB_STREAM *stream, const char *path, int mode, void *data){return -1;}
int CUdpSocket_stream_seek(GB_STREAM *stream, int64_t pos, int whence){return -1;}
int CUdpSocket_stream_tell(GB_STREAM *stream, int64_t *pos)
{
	*pos=0;
	return -1; /* not allowed */
}

int CUdpSocket_stream_flush(GB_STREAM *stream)
{
	return 0; /* OK */
}

int CUdpSocket_stream_handle(GB_STREAM *stream)
{
	void *_object = stream->tag;
	return THIS->Socket;
}

int CUdpSocket_stream_close(GB_STREAM *stream)
{
	void *_object = stream->tag;

	if ( !_object ) return -1;
	stream->desc=NULL;
	if (THIS->iStatus > 0)
	{
		GB.Watch (THIS->Socket,GB_WATCH_NONE,(void *)CUdpSocket_CallBack,(intptr_t)THIS);
		close(THIS->Socket);
		THIS->iStatus=0;
	}
	if (THIS->shost) GB.FreeString(&THIS->shost);
	if (THIS->thost) GB.FreeString(&THIS->thost);
	THIS->shost=NULL;
	THIS->thost=NULL;
	THIS->sport=0;
	THIS->tport=0;
	THIS->iStatus=0;
	clear_buffer(THIS);
	return 0;
}

int CUdpSocket_stream_lof(GB_STREAM *stream, int64_t *len)
{
	void *_object = stream->tag;
	*len = THIS->buffer_len - THIS->buffer_pos;
	return 0;
}

int CUdpSocket_stream_eof(GB_STREAM *stream)
{
	void *_object = stream->tag;
	return THIS->buffer_pos >= THIS->buffer_len;
}

int CUdpSocket_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = stream->tag;
	int len_max;

	if ( !_object ) return TRUE;
	
	len_max = THIS->buffer_len - THIS->buffer_pos;
	
	if (len_max <= 0)
		return TRUE;
	
	if (len > len_max)
		len = len_max;
		
	memcpy(buffer, &THIS->buffer[THIS->buffer_pos], len);
	THIS->buffer_pos += len;
	
	GB.Stream.SetBytesRead(stream, len);
	
	return 0;
}

int CUdpSocket_stream_write(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = stream->tag;
	int retval;
	//int NoBlock=0;
	struct sockaddr_in remhost;
	struct in_addr rem_ip;

	if ( !_object ) return -1;

	if (!THIS->thost) return -1;
	if ( (THIS->tport<1) || (THIS->tport>65535) ) return -1;
	if (!inet_aton ( (const char*)THIS->thost,&rem_ip)) return -1;
	remhost.sin_family=AF_INET;
	remhost.sin_port=htons(THIS->tport);
	remhost.sin_addr.s_addr=rem_ip.s_addr;
	bzero(&(remhost.sin_zero),8);
	//ioctl(THIS->Socket,FIONBIO,&NoBlock);
	USE_MSG_NOSIGNAL(retval=sendto(THIS->Socket,(void*)buffer,len*sizeof(char) \
		              ,MSG_NOSIGNAL,(struct sockaddr*)&remhost,sizeof(struct sockaddr)));
	//NoBlock++;
	//ioctl(THIS->Socket,FIONBIO,&NoBlock);
	if (retval>=0) return 0;
	CUdpSocket_stream_close(stream);
	THIS->iStatus= -5;
	return -1;
}

/************************************************************************************************
 ################################################################################################
 --------------------UDPSOCKET CLASS GAMBAS INTERFACE IMPLEMENTATION------------------------------
 ################################################################################################
 ***********************************************************************************************/

static bool update_broadcast(CUDPSOCKET *_object)
{
	if (THIS->Socket == 0)
		return FALSE;

	if (setsockopt(THIS->Socket, SOL_SOCKET, SO_BROADCAST, (char *)&THIS->broadcast, sizeof(int)) < 0)
	{
		GB.Error("Cannot set broadcast socket option");
		return TRUE;
	}
	else
		return FALSE;
}

static int dgram_start(CUDPSOCKET *mythis,int myport)
{
	//int NoBlock=1;
	struct sockaddr_in Srv;

	if (mythis->iStatus > 0) return 1;
	if ( (myport <0) || (myport>65535) ) return 8;

	if ( (mythis->Socket = socket(AF_INET,SOCK_DGRAM,0))<1 )
	{
		mythis->iStatus=-2;
		GB.Ref(mythis);
		GB.Post(CUdpSocket_post_error,(intptr_t)mythis);
		return 2;
	}

	if (update_broadcast(mythis))
	{
		mythis->iStatus=-2;
		GB.Ref(mythis);
		GB.Post(CUdpSocket_post_error,(intptr_t)mythis);
		return 2;
	}

	Srv.sin_family=AF_INET;
	Srv.sin_addr.s_addr=htonl(INADDR_ANY);
	Srv.sin_port=htons(myport);
	bzero(&(Srv.sin_zero),8);

	if ( bind (mythis->Socket,(struct sockaddr*)&Srv,sizeof(struct sockaddr)) < 0)
	{
		close (mythis->Socket);
		mythis->iStatus=-10;
		GB.Ref(mythis);
		GB.Post(CUdpSocket_post_error,(intptr_t)mythis);
		return 10;
	}

	mythis->iStatus=1;
	//ioctl(mythis->Socket,FIONBIO,&NoBlock);
	GB.Watch (mythis->Socket,GB_WATCH_READ,(void *)CUdpSocket_CallBack,(intptr_t)mythis);
	mythis->stream.desc=&UdpSocketStream;
	GB.Stream.SetSwapping(&mythis->stream, htons(1234) != 1234);
	return 0;
}


/**********************************************************
 This property gets status : 0 --> Inactive, 1 --> Working
 **********************************************************/
BEGIN_PROPERTY ( CUDPSOCKET_Status )

  GB.ReturnInteger(THIS->iStatus);

END_PROPERTY

BEGIN_PROPERTY ( CUDPSOCKET_SourceHost )

  GB.ReturnString(THIS->shost);

END_PROPERTY

BEGIN_PROPERTY ( CUDPSOCKET_SourcePort )

  GB.ReturnInteger(THIS->sport);

END_PROPERTY

BEGIN_PROPERTY ( CUDPSOCKET_TargetHost )

  	char *strtmp;
  	struct in_addr rem_ip;
  	if (READ_PROPERTY)
  	{
  		GB.ReturnString(THIS->thost);
		return;
  	}

  	strtmp=GB.ToZeroString(PROP(GB_STRING));
  	if ( !inet_aton(strtmp,&rem_ip) )
	{
		GB.Error ("Invalid IP address");
		return;
	}
  	GB.StoreString(PROP(GB_STRING), &THIS->thost);

END_PROPERTY

BEGIN_PROPERTY ( CUDPSOCKET_TargetPort )

  if (READ_PROPERTY)
  {
  	GB.ReturnInteger(THIS->sport);
	return;
  }

  if ( (VPROP(GB_INTEGER)<1) || (VPROP(GB_INTEGER)>65535) )
  {
  	GB.Error("Invalid Port value");
	return;
  }
  THIS->tport=VPROP(GB_INTEGER);

END_PROPERTY

/*************************************************
 Gambas object "Constructor"
 *************************************************/
BEGIN_METHOD(CUDPSOCKET_new,GB_INTEGER Port;)

	THIS->stream.tag = _object;
	if (MISSING (Port) ) return;
	dgram_start(THIS,VARG(Port));

END_METHOD

/*************************************************
 Gambas object "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CUDPSOCKET_free)

	CUdpSocket_stream_close(&THIS->stream);

END_METHOD


BEGIN_METHOD_VOID (CUDPSOCKET_Peek)

	char *sData=NULL;
	struct sockaddr_in remhost;
	unsigned int rem_host_len;
	int retval=0;
	//int NoBlock=0;
	int peeking;
	int bytes=0;
	if (THIS->iStatus <= 0)
	{
		GB.Error ("Inactive");
		return;
	}


	peeking=MSG_NOSIGNAL | MSG_PEEK;

	ioctl(THIS->Socket,FIONREAD,&bytes);
	if (bytes)
	{
		GB.Alloc( POINTER(&sData),bytes*sizeof(char) );
		rem_host_len=sizeof(struct sockaddr);
		//ioctl(THIS->Socket,FIONBIO,&NoBlock);
		USE_MSG_NOSIGNAL(retval=recvfrom(THIS->Socket,(void*)sData,1024*sizeof(char) \
		       ,peeking,(struct sockaddr*)&remhost,&rem_host_len));
		if (retval<0)
		{
			GB.Free(POINTER(&sData));
			CUdpSocket_stream_close(&THIS->stream);
			THIS->iStatus=-4;
			GB.Raise(THIS,CUDPSOCKET_SocketError,0);
			GB.ReturnNewString(NULL,0);
			return;
		}
		//NoBlock++;
		//ioctl(THIS->Socket,FIONBIO,&NoBlock);
		THIS->sport=ntohs(remhost.sin_port);
		GB.FreeString(&THIS->shost);
		GB.NewString ( &THIS->shost , inet_ntoa(remhost.sin_addr) ,0);
		if (retval>0)
			GB.ReturnNewString(sData,retval);
		else
			GB.ReturnNewString(NULL,0);
		GB.Free(POINTER(&sData));
	}
	else
	{
		GB.FreeString(&THIS->shost);
		THIS->shost=NULL;
		THIS->sport=0;
		GB.ReturnNewString(NULL,0);
	}



END_METHOD

BEGIN_METHOD (CUDPSOCKET_Bind,GB_INTEGER Port;)

	switch( dgram_start(THIS,VARG(Port)) )
	{
		case 1:
			GB.Error("Already working");
			return;
		case 8:
			GB.Error("Port value is not valid");
			return;
	}

END_METHOD


BEGIN_PROPERTY(CUDPSOCKET_broadcast)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->broadcast);
	}
	else
	{
		THIS->broadcast = VPROP(GB_BOOLEAN);
		update_broadcast(THIS);
	}

END_PROPERTY


/***************************************************************
 Here we declare the public interface of UdpSocket class
 ***************************************************************/
GB_DESC CUdpSocketDesc[] =
{

  GB_DECLARE("UdpSocket", sizeof(CUDPSOCKET)),

  GB_INHERITS("Stream"),

  GB_EVENT("Error", NULL, NULL, &CUDPSOCKET_SocketError),
  GB_EVENT("Read", NULL, NULL, &CUDPSOCKET_Read),

  GB_METHOD("_new", NULL, CUDPSOCKET_new, "[(Port)i]"),
  GB_METHOD("_free", NULL, CUDPSOCKET_free, NULL),
  GB_METHOD("Bind", NULL, CUDPSOCKET_Bind,"(Port)i"),
  GB_METHOD("Peek","s",CUDPSOCKET_Peek,NULL),

  GB_PROPERTY_READ("Status", "i", CUDPSOCKET_Status),
  GB_PROPERTY_READ("SourceHost", "s", CUDPSOCKET_SourceHost),
  GB_PROPERTY_READ("SourcePort", "i", CUDPSOCKET_SourcePort),
  GB_PROPERTY("TargetHost", "s", CUDPSOCKET_TargetHost),
  GB_PROPERTY("TargetPort", "i", CUDPSOCKET_TargetPort),

  GB_PROPERTY("Broadcast", "b", CUDPSOCKET_broadcast),

  GB_CONSTANT("_Properties", "s", "TargetHost,TargetPort,Broadcast"),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),

  GB_END_DECLARE
};


