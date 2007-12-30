/***************************************************************************

  CFtpClient.h

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
#ifndef __CFTPCLIENT_H
#define __CFTPCLIENT_H

#include "gambas.h"
#include "gbcurl.h"
#include "CProxy.h"
#include <curl/curl.h>
#include <curl/easy.h>

#ifndef __CFTPCLIENT_C


extern GB_DESC CFtpClientDesc[];
extern GB_STREAM_DESC FtpStream;

#else

#define THIS            ((CFTPCLIENT *)_object)
#define THIS_STATUS     ((((CFTPCLIENT *)_object)->stream._free[1]))
#define THIS_CURL       ((((CFTPCLIENT *)_object)->stream._free[2]))
#define THIS_URL        ((((CFTPCLIENT *)_object)->stream._free[3]))
#define THIS_FILE       ((((CFTPCLIENT *)_object)->stream._free[4]))
#define THIS_PROTOCOL   ((((CFTPCLIENT *)_object)->stream._free[5]))

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
	
	int iMethod; // 0->Get, 1->Put


}  CFTPCLIENT;


/*****/
int ftp_find_info (CURL *curlfind);
/*****/
int ftp_header_curl(void *buffer, size_t size, size_t nmemb, void *c_handle);
int ftp_write_curl(void *buffer, size_t size, size_t nmemb, void *c_handle);
/*****/
void ftp_parse_header(CFTPCLIENT *mythis);
/*****/
void ftp_reset(CFTPCLIENT *mythis);
void ftp_stop(void *_object);
/*****/
//


#define FTP_PROPERTIES "URL=127.0.0.1:21,Async=TRUE,TimeOut=0,User,Password"
#endif
