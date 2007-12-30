/***************************************************************************

  CHttpClient.h

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
#ifndef __CHTTPCLIENT_H
#define __CHTTPCLIENT_H

#include "gambas.h"
#include "gbcurl.h"
#include "CProxy.h"
#include <curl/curl.h>
#include <curl/easy.h>

#ifndef __CHTTPCLIENT_C


extern GB_DESC CHttpClientDesc[];
extern GB_STREAM_DESC HttpStream;

#else

#define THIS            ((CHTTPCLIENT *)_object)
#define THIS_STATUS     ((((CHTTPCLIENT *)_object)->stream._free[1]))
#define THIS_CURL       ((((CHTTPCLIENT *)_object)->stream._free[2]))
#define THIS_URL        ((((CHTTPCLIENT *)_object)->stream._free[3]))
#define THIS_FILE       ((((CHTTPCLIENT *)_object)->stream._free[4]))
#define THIS_PROTOCOL   ((((CHTTPCLIENT *)_object)->stream._free[5]))

#endif

typedef  struct
{
	GB_BASE    ob;
	GB_STREAM  stream;
	CPROXY     proxy;
	Adv_user   user;
	int        len_data;
	char       *buf_data;
	GB_VARIANT_VALUE tag;
	int mode; // 0 -> Async, sync
	long TimeOut;
	
	int auth;

	char *cookiesfile;
	int updatecookies;

	char *sContentType;
	char *sPostData;
	int iMethod; // 0->Get, 1->Post
	
	char *sUserAgent;

	char **buf_header;
	int len_header;

	int   ReturnCode;
	char *ReturnString;
}  CHTTPCLIENT;


/*****/
int http_find_info (CURL *curlfind);
/*****/
int http_header_curl(void *buffer, size_t size, size_t nmemb, void *c_handle);
int http_write_curl(void *buffer, size_t size, size_t nmemb, void *c_handle);
/*****/
void http_parse_header(CHTTPCLIENT *mythis);
/*****/
void http_reset(void *_object);
void http_stop(void *_object);
/*****/
//


#define HTTP_PROPERTIES "URL=127.0.0.1:80,User,Password,Auth=0,Async=TRUE,TimeOut=0,CookiesFile,UpdateCookies=FALSE,UserAgent=Gambas Http/1.0"
#endif
