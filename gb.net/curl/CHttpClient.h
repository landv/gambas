/***************************************************************************

  CHttpClient.h

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

#ifndef __CHTTPCLIENT_H
#define __CHTTPCLIENT_H

#include "gambas.h"
#include "gbcurl.h"
#include "CCurl.h"
#include "CProxy.h"
#include <curl/curl.h>
#include <curl/easy.h>

#ifndef __CHTTPCLIENT_C


extern GB_DESC CHttpClientDesc[];
extern GB_STREAM_DESC HttpStream;

#else

#define THIS_HTTP ((CHTTPCLIENT *)_object)

#endif

typedef
	struct {
		CCURL curl;
		int auth;
		char *cookiesfile;
		int updatecookies;
		char *sContentType;
		char *sUserAgent;
		char *encoding;
		GB_ARRAY headers;
		GB_ARRAY sent_headers;
		int return_code;
		char *return_string;
		char *data;
		char *target;
		size_t len_data;
		size_t len_sent;
		bool send_file;
		}
	CHTTPCLIENT;


/*int http_find_info (CURL *curlfind);
int http_header_curl(void *buffer, size_t size, size_t nmemb, void *c_handle);
int http_write_curl(void *buffer, size_t size, size_t nmemb, void *c_handle);
void http_parse_header(CHTTPCLIENT *mythis);
void http_reset(void *_object);
void http_stop(void *_object);*/

#define HTTP_PROPERTIES "URL=127.0.0.1:80,User,Password,Auth=0,Async=True,Timeout=0,CookiesFile,UpdateCookies=FALSE,UserAgent=Gambas (gb.net.curl) HTTP/1.0"

#endif
