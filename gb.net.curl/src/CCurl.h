/***************************************************************************

  CCurl.h

  Advanced Network component

  (c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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
#ifndef __CCURL_H
#define __CCURL_H

#include "gambas.h"
#include "gbcurl.h"
#include "CProxy.h"
#include <curl/curl.h>
#include <curl/easy.h>

#ifndef __CCURL_C


extern GB_DESC CCurlDesc[];
extern GB_STREAM_DESC CurlStream;

#else

#define THIS            ((CCURL *)_object)
#define THIS_STATUS     ((((CCURL *)_object)->stream._free[1]))
#define THIS_CURL       ((((CCURL *)_object)->stream._free[2]))
#define THIS_URL        ((((CCURL *)_object)->stream._free[3]))
#define THIS_FILE       ((((CCURL *)_object)->stream._free[4]))
#define THIS_PROTOCOL   ((((CCURL *)_object)->stream._free[5]))

#endif

typedef  struct
{
	GB_BASE    ob;
	GB_STREAM  stream;
	CPROXY    *proxy;
	Adv_user   user;
	int        len_data;
	char       *buf_data;
	GB_VARIANT_VALUE tag;
	int mode; // 0 -> Async, sync
	long TimeOut;

	int iMethod; // 0->Get, 1->Put


	int   ReturnCode;
	char *ReturnString;

	

}  CCURL;

void CCURL_stop(void *_object);

void CCURL_stream_init  (GB_STREAM *stream,int fd);
int  CCURL_stream_read  (GB_STREAM *stream, char *buffer, long len);
int  CCURL_stream_write (GB_STREAM *stream, char *buffer, long len);
int  CCURL_stream_eof   (GB_STREAM *stream);
int  CCURL_stream_lof   (GB_STREAM *stream, long long *len);
int  CCURL_stream_open  (GB_STREAM *stream, const char *path, int mode, void *data);
int  CCURL_stream_seek  (GB_STREAM *stream, long long pos, int whence);
int  CCURL_stream_tell  (GB_STREAM *stream, long long *pos);
int  CCURL_stream_flush (GB_STREAM *stream);
int  CCURL_stream_close (GB_STREAM *stream);
int  CCURL_stream_handle(GB_STREAM *stream);

void CCURL_raise_finished (long lParam);
void CCURL_raise_error    (long lParam);
void CCURL_raise_connect  (long lParam);
void CCURL_raise_read     (long lParam);

void CCURL_init_post(void);
void CCURL_post_curl(long data);

void CCURL_Manage_ErrCode(void *_object,long ErrCode);


#endif
