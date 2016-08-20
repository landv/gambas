/***************************************************************************

  CCurl.h

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CCURL_H
#define __CCURL_H

#include "gambas.h"
#include "gb_common.h"
#include "gbcurl.h"
#include "CNet.h"
#include "CProxy.h"
#include <curl/curl.h>
#include <curl/easy.h>

//#define DEBUG 1

#ifndef __CCURL_C

extern GB_DESC CurlDesc[];
extern GB_DESC CurlSSLDesc[];
extern GB_STREAM_DESC CurlStream;

#endif

#define THIS            ((CCURL *)_object)
#define THIS_STATUS     THIS->status
#define THIS_CURL       THIS->curl
#define THIS_URL        THIS->url
#define THIS_FILE       THIS->file

typedef
	struct {
		int *parent_status;
		CURL_PROXY proxy;
	}
	CPROXY;

typedef
	void (*CURL_FIX_PROGRESS_CB)(void *, double *, double *, double *, double *);

typedef  
	struct {
		GB_BASE ob;
		GB_STREAM stream;
		GB_LIST list; // List of async curl objects
		int status;
		CURL *curl;
		char *url;
		FILE *file;
		CPROXY proxy;
		CURL_USER user;
		int timeout;
		int method; // 0->Get, 1->Put
		char *data;
		int64_t dltotal;
		int64_t dlnow;
		int64_t ultotal;
		int64_t ulnow;
		CURL_FIX_PROGRESS_CB progresscb;
		unsigned async : 1;
		unsigned in_list : 1;
		unsigned debug : 1;
		unsigned ssl_verify_peer : 1;
		unsigned ssl_verify_host : 1;
	}
	CCURL;

#define STREAM_TO_OBJECT(_stream) (_stream->tag)

void CCURL_stream_init(GB_STREAM *stream,int fd);
int CCURL_stream_read(GB_STREAM *stream, char *buffer, int len);
int CCURL_stream_write(GB_STREAM *stream, char *buffer, int len);
int CCURL_stream_eof(GB_STREAM *stream);
int CCURL_stream_lof(GB_STREAM *stream, int64_t *len);
int CCURL_stream_open(GB_STREAM *stream, const char *path, int mode, void *data);
int CCURL_stream_seek(GB_STREAM *stream, int64_t pos, int whence);
int CCURL_stream_tell(GB_STREAM *stream, int64_t *pos);
int CCURL_stream_flush(GB_STREAM *stream);
int CCURL_stream_close(GB_STREAM *stream);
int CCURL_stream_handle(GB_STREAM *stream);

void CURL_raise_finished(void *_object);
void CURL_raise_error(void *_object);
void CURL_raise_cancel(void *_object);
void CURL_raise_connect(void *_object);
void CURL_raise_read(void *_object);

void CURL_start_post(void *_object);
void CURL_stop(void *_object);

void CURL_manage_error(void *_object, int error);

void CURL_init_stream(void *_object);
void CURL_init_options(void *_object);

bool CURL_check_active(void *_object);

void CURL_set_progress(void *_object, bool progress, CURL_FIX_PROGRESS_CB cb);

bool CURL_copy_from(CCURL *dest, CCURL *src);

#endif
