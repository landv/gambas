/***************************************************************************

  CUdpSocket.h

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
#ifndef __CUDPSOCKET_H
#define __CUDPSOCKET_H

#include "gambas.h"

#ifndef __CUDPSOCKET_C

extern GB_DESC CUdpSocketDesc[];
extern GB_STREAM_DESC UdpSocketStream;

#else

#define THIS ((CUDPSOCKET *)_object)

#endif

typedef	
	struct
	{
		GB_BASE ob;
		GB_STREAM stream;
		int Socket;
		int iStatus;
		int iPort;
		char *shost;
		int sport;
		char *spath;
		char *thost;
		int tport;
		char *tpath;
		int broadcast;
		char *buffer;
		int buffer_pos;
		int buffer_len;
	} 
	CUDPSOCKET;

int CUdpSocket_stream_read(GB_STREAM *stream, char *buffer, int len);
int CUdpSocket_stream_write(GB_STREAM *stream, char *buffer, int len);
int CUdpSocket_stream_eof(GB_STREAM *stream);
int CUdpSocket_stream_lof(GB_STREAM *stream, int64_t *len);
int CUdpSocket_stream_open(GB_STREAM *stream, const char *path, int mode, void *data);
int CUdpSocket_stream_seek(GB_STREAM *stream, int64_t pos, int whence);
int CUdpSocket_stream_tell(GB_STREAM *stream, int64_t *pos);
int CUdpSocket_stream_flush(GB_STREAM *stream);
int CUdpSocket_stream_close(GB_STREAM *stream);
int CUdpSocket_stream_handle(GB_STREAM *stream);

#endif
