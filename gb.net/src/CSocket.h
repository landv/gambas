/***************************************************************************

  CSocket.h

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
#ifndef __CSOCKET_H
#define __CSOCKET_H

#include "gambas.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include "CDnsClient.h"

#ifndef __CSOCKET_C

extern GB_DESC CSocketDesc[];
extern GB_STREAM_DESC SocketStream;

#else

#define THIS ((CSOCKET *)_object)

#endif

void CSocket_CallBack(int t_sock,int type,long lParam);
void CSocket_CallBackConnecting(int t_sock,int type,long lParam);


typedef struct SOCKET_STREAM  {
	GB_STREAM_DESC *desc;
    	int _reserved;
    	void *handle;
} SOCKET_STREAM;

typedef  struct
{
   GB_BASE ob;
   GB_STREAM stream;
   int Socket;
   struct sockaddr_in Server;  /* struct for TCP connections  */
   struct sockaddr_un UServer; /* struct for UNIX connections */
   int iStatus;
   int iUsePort;
   int iPort;
   int iLocalPort;
   int conn_type;
   char *sPath;
   char *sLocalHostIP;
   char *sRemoteHostIP;
   // $BM
   //char *HostOrPath;
   char *Host;
   char *Path;
   CDNSCLIENT *DnsTool;
   //
   void *c_parent;
   //
   void (*OnClose)(void *sck);
   //
}  CSOCKET;

//
void CSocket_post_error(CSOCKET *mythis);
void CSocket_post_closed(CSOCKET *mythis);
void CSocket_post_hostfound(CSOCKET *mythis);
void CSocket_post_connected(CSOCKET *mythis);
void CSocket_post_data_available(CSOCKET *mythis);
//
int CSocket_connect_unix(CSOCKET *mythis,char *sPath,int lenpath);
int CSocket_connect_socket(CSOCKET *mythis,char *sHost,int lenhost,int myport);
int CSocket_peek_data(CSOCKET *mythis,char **buf,int MaxLen);
//
void CSocket_stream_internal_error(CSOCKET *mythis,int ncode);
//
int CSocket_stream_read(GB_STREAM *stream, char *buffer, int len);
int CSocket_stream_write(GB_STREAM *stream, char *buffer, int len);
int CSocket_stream_eof(GB_STREAM *stream);
int CSocket_stream_lof(GB_STREAM *stream, int64_t *len);
int CSocket_stream_open(GB_STREAM *stream, const char *path, int mode, void *data);
int CSocket_stream_seek(GB_STREAM *stream, int64_t pos, int whence);
int CSocket_stream_tell(GB_STREAM *stream, int64_t *pos);
int CSocket_stream_flush(GB_STREAM *stream);
int CSocket_stream_close(GB_STREAM *stream);
int CSocket_stream_handle(GB_STREAM *stream);
#endif
