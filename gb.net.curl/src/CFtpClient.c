/***************************************************************************

  CFtpClient.c

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#define __CFTPCLIENT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>

#include "main.h"
#include "gambas.h"
#include "CCurl.h"
#include "CFtpClient.h"
#include "CProxy.h"

/*****************************************************
 CURLM : a pointer to use curl_multi interface,
 allowing asynchrnous work without using threads
 in this class. Here also a pipe will be stablished
 to link with Gambas watching interface
 ******************************************************/
extern CURLM *CCURL_multicurl;
/*******************************************************************
####################################################################
	CALLBACKS FROM CURL LIBRARY
####################################################################
********************************************************************/

int ftp_read_curl (void *buffer, size_t size, size_t nmemb, void *_object)
{
/* BM */
        FILE *file = THIS_FILE;
	THIS_STATUS=4;
	
	if (!feof(file))
		nmemb=fread(buffer,size,nmemb,file);
	else
		nmemb=0;
	
	return nmemb;
}

int ftp_write_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{

	THIS_STATUS=4;

	if (THIS_FILE)
	{
		return fwrite(buffer,size,nmemb,THIS_FILE);
	}
	else
	{
		if (!THIS->len_data)
			GB.Alloc((void**)POINTER(&THIS->buf_data),nmemb);
		else
			GB.Realloc((void**)POINTER(&THIS->buf_data),nmemb+THIS->len_data);
		memcpy(THIS->buf_data+THIS->len_data,buffer,nmemb);
		THIS->len_data+=nmemb;
	}

	if (THIS->async)
	{
		GB.Ref(THIS);
		GB.Post(CCURL_raise_read,(long)THIS);
	}
	
	return nmemb;
}

void ftp_reset(CFTPCLIENT *mythis)
{
	if (mythis->curl.buf_data)
	{
		GB.Free((void**)POINTER(&mythis->curl.buf_data));
		mythis->curl.buf_data=NULL;
	}
	
	mythis->curl.len_data=0;
}

void ftp_initialize_curl_handle(void *_object)
{
	if (THIS_CURL)
	{
		if (Adv_Comp ( THIS->user.userpwd,THIS->user.user,THIS->user.pwd))
		{
			CCURL_stop(_object);
			ftp_reset(_object);
			THIS_CURL=curl_easy_init();
			#if DEBUG
			fprintf(stderr, "-- [%p] curl_easy_init() -> %p\n", THIS, THIS_CURL);
			#endif
		}
	}
	else
	{
		THIS_CURL=curl_easy_init();
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_easy_init() -> %p\n", THIS, THIS_CURL);
		#endif
	}

	if (!THIS->async)
	{
		curl_easy_setopt(THIS_CURL, CURLOPT_NOSIGNAL,1);
		curl_easy_setopt(THIS_CURL, CURLOPT_TIMEOUT,THIS->TimeOut);
	}
	
	curl_easy_setopt(THIS_CURL, CURLOPT_VERBOSE,1);
	curl_easy_setopt(THIS_CURL, CURLOPT_PRIVATE,(char*)_object);

	Adv_proxy_SET (&THIS->proxy.proxy,THIS_CURL);
	Adv_user_SET  (&THIS->user, THIS_CURL);
	curl_easy_setopt(THIS_CURL, CURLOPT_URL,THIS_URL);

	ftp_reset(THIS_FTP);
	THIS_STATUS=6;
	
	CCURL_init_stream(THIS);
}


int ftp_get (void *_object)
{
	if (THIS_STATUS > 0) return 1;

	THIS->iMethod=0;
	
	ftp_initialize_curl_handle(THIS);
	
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEFUNCTION , ftp_write_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEDATA     , _object);
	curl_easy_setopt(THIS_CURL, CURLOPT_UPLOAD        , 0);
	
	if (THIS->async)
	{
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_multi_add_handle(%p)\n", THIS, THIS_CURL);
		#endif
		curl_multi_add_handle(CCURL_multicurl,THIS_CURL);
		CCURL_init_post();
		return 0;
	}
	
	CCURL_Manage_ErrCode(_object,curl_easy_perform(THIS_CURL));
	return 0;
}

int ftp_put (void *_object)
{
	if (THIS_STATUS > 0) return 1;

	THIS->iMethod=1;
	
	ftp_initialize_curl_handle(THIS);
	
	curl_easy_setopt(THIS_CURL, CURLOPT_READFUNCTION , ftp_read_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_READDATA     , _object);
	curl_easy_setopt(THIS_CURL, CURLOPT_UPLOAD       , 1);
	
	
	if (THIS->async)
	{
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_multi_add_handle(%p)\n", THIS, THIS_CURL);
		#endif
		curl_multi_add_handle(CCURL_multicurl,THIS_CURL);
		CCURL_init_post();
		return 0;
	}
	
	CCURL_Manage_ErrCode(_object,curl_easy_perform(THIS_CURL));
	return 0;
}

BEGIN_METHOD(CFTPCLIENT_Get,GB_STRING TargetHost;)

	if (!MISSING(TargetHost))
	{
		if (THIS_STATUS > 0)
		{
			GB.Error("Still active");
			return;
		}
		THIS_FILE=fopen(GB.ToZeroString(ARG(TargetHost)),"w");
		if (!THIS_FILE)
		{
			GB.Error("Unable to open file for writing");
			return;
		}
	}

	if (ftp_get(THIS)) GB.Error("Still active");
	

END_METHOD

/*************************************************
 PUT FTP method
 *************************************************/


BEGIN_METHOD(CFTPCLIENT_Put,GB_STRING SourceFile;)

	
	if (THIS_STATUS > 0)
	{
		GB.Error("Still active");
		return;
	}
	if (!LENGTH(SourceFile))
	{
		GB.Error("Invalid File Name\n");
		return;
	}
	
	THIS_FILE=fopen(GB.ToZeroString(ARG(SourceFile)),"r");
	if (!THIS_FILE)
	{
		GB.Error("Unable to open file for reading");
		return;
	}
	

	if (ftp_put (THIS) ) GB.Error("Still active");


END_METHOD

BEGIN_METHOD_VOID(CFTPCLIENT_Stop)

	CCURL_stop(_object);
	ftp_reset(_object);
	
END_METHOD

BEGIN_METHOD_VOID(CFTPCLIENT_new)

	char *tmp=NULL;	
	
	GB.Alloc((void**)POINTER(&tmp),sizeof(char)*(1+strlen("ftp://127.0.0.1:21")));
	THIS_URL=tmp;
	strcpy(tmp,"ftp://127.0.0.1:21");
	tmp=NULL;
	GB.Alloc((void**)POINTER(&tmp),7);
	strcpy(tmp,"ftp://");
	THIS_PROTOCOL=tmp;
	Adv_user_SETAUTH (&THIS->user,CURLAUTH_BASIC);


END_METHOD

BEGIN_METHOD_VOID(CFTPCLIENT_free)

	ftp_reset(THIS_FTP);

END_METHOD


//*************************************************************************
//#################### GAMBAS INTERFACE ###################################
//*************************************************************************
GB_DESC CFtpClientDesc[] =
{

  GB_DECLARE("FtpClient", sizeof(CFTPCLIENT)),

  GB_INHERITS("Curl"),

  GB_METHOD("_new", NULL, CFTPCLIENT_new, NULL),
  GB_METHOD("_free", NULL, CFTPCLIENT_free, NULL),

  GB_METHOD("Stop", NULL, CFTPCLIENT_Stop, NULL),
  GB_METHOD("Get", NULL, CFTPCLIENT_Get, "[(TargetFile)s]"),
  GB_METHOD("Put", NULL, CFTPCLIENT_Put, "(LocalFile)s"),
  
  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", FTP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),
  
  GB_END_DECLARE
};

