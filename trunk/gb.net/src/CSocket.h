/***************************************************************************

  CSocket.h

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

#ifndef __CSOCKET_H
#define __CSOCKET_H

#include "gambas.h"
#include "gb_common.h"

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
#define SOCKET (&THIS->common)

#endif

typedef
	struct {
		GB_BASE ob;
		GB_STREAM stream;
		int socket;
		int status;
		int timeout;
	}
	CSOCKET_COMMON;

typedef
	struct
	{
		CSOCKET_COMMON common;
		struct sockaddr_in Server;  /* struct for TCP connections  */
		struct sockaddr_un UServer; /* struct for UNIX connections */
		int iUsePort;
		int iPort;
		int iLocalPort;
		int conn_type;
		char *sPath;
		char *sLocalHostIP;
		char *sRemoteHostIP;
		char *Host;
		char *Path;
		CDNSCLIENT *DnsTool;
		void *parent;
		void (*OnClose)(void *sck);
		bool watch_write;
	}  
	CSOCKET;

void CSocket_CallBack(int t_sock,int type, CSOCKET *_object);
void CSocket_CallBackConnecting(int t_sock,int type,intptr_t lParam);

void CSOCKET_init_connected(CSOCKET *_object);

void CSocket_post_connected(void *_object);
//
int CSocket_connect_unix(void *_object, char *sPath, int lenpath);
int CSocket_connect_socket(void *_object, char *sHost,int lenhost,int myport);
int CSocket_peek_data(void *_object,char **buf,int MaxLen);
//
void CSocket_stream_internal_error(void *_object,int ncode, bool post);
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

bool SOCKET_update_timeout(CSOCKET_COMMON *socket);
void SOCKET_set_blocking(CSOCKET_COMMON *socket, bool block);

DECLARE_METHOD(Socket_Timeout);

#endif
