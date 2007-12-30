/***************************************************************************

  CFtpClient.c

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
extern GB_STREAM_DESC CurlStream;
extern CURLM *CCURL_multicurl;
/*******************************************************************
####################################################################
	CALLBACKS FROM CURL LIBRARY
####################################################################
********************************************************************/

int ftp_read_curl (void *buffer, size_t size, size_t nmemb, void *_object)
{
/* BM */
        FILE *file = (FILE*)THIS_FILE;
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
		return fwrite(buffer,size,nmemb,(FILE*)THIS_FILE);
	}
	else
	{
		if (!THIS->len_data)
			GB.Alloc((void**)&THIS->buf_data,nmemb);
		else
			GB.Realloc((void**)&THIS->buf_data,nmemb+THIS->len_data);
		memcpy(THIS->buf_data+THIS->len_data,buffer,nmemb);
		THIS->len_data+=nmemb;
	}

	if (!THIS->mode)
	{
		GB.Ref(THIS);
		GB.Post(CCURL_raise_read,(long)THIS);
	}
	
	return nmemb;
}

void ftp_reset(CFTPCLIENT *mythis)
{

	if (mythis->buf_data)
	{
		GB.Free((void**)&mythis->buf_data);
		mythis->buf_data=NULL;
	}
	
	mythis->len_data=0;
}

void ftp_initialize_curl_handle(void *_object)
{
	if ((CURL*)THIS_CURL)
	{
		if (Adv_Comp ( THIS->user.userpwd,THIS->user.user,THIS->user.pwd))
		{
			CCURL_stop(_object);
			ftp_reset(_object);
			THIS_CURL=(long)curl_easy_init();
		}
	}
	else
	{
		THIS_CURL=(long)curl_easy_init();
	}

	if (THIS->mode)
	{
		curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_NOSIGNAL,1);
		curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_TIMEOUT,THIS->TimeOut);
	}
	
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_VERBOSE,1);
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_PRIVATE,(char*)_object);

	Adv_proxy_SET (&THIS->proxy->proxy,(CURL*)THIS_CURL);
	Adv_user_SET  (&THIS->user, (CURL*)THIS_CURL);
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_URL,THIS_URL);

	ftp_reset(THIS);
	THIS_STATUS=6;
	THIS->stream.desc=&CurlStream;
}


int ftp_get (void *_object)
{
	if (THIS_STATUS > 0) return 1;

	THIS->iMethod=0;
	
	ftp_initialize_curl_handle(THIS);
	
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_WRITEFUNCTION , ftp_write_curl);
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_WRITEDATA     , _object);
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_UPLOAD        , 0);
	
	if (!THIS->mode)
	{
		curl_multi_add_handle(CCURL_multicurl,(CURL*)THIS_CURL);
		CCURL_init_post();
		return 0;
	}
	
	CCURL_Manage_ErrCode(_object,curl_easy_perform((CURL*)THIS_CURL));
	return 0;
}

int ftp_put (void *_object)
{
	if (THIS_STATUS > 0) return 1;

	THIS->iMethod=1;
	
	ftp_initialize_curl_handle(THIS);
	
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_READFUNCTION , ftp_read_curl);
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_READDATA     , _object);
	curl_easy_setopt((CURL*)THIS_CURL, CURLOPT_UPLOAD       , 1);
	
	
	if (!THIS->mode)
	{
		curl_multi_add_handle(CCURL_multicurl,(CURL*)THIS_CURL);
		CCURL_init_post();
		return 0;
	}
	
	CCURL_Manage_ErrCode(_object,curl_easy_perform((CURL*)THIS_CURL));
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
		THIS_FILE=(long)fopen(STRING(TargetHost),"w");
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
	
	THIS_FILE=(long)fopen(STRING(SourceFile),"r");
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
	
	GB.Alloc((void**)&tmp,sizeof(char)*(1+strlen("ftp://127.0.0.1:21")));
	THIS_URL=(long)tmp;
	strcpy(tmp,"ftp://127.0.0.1:21");
	tmp=NULL;
	GB.Alloc((void**)&tmp,7);
	strcpy(tmp,"ftp://");
	THIS_PROTOCOL=(long)tmp;
	Adv_user_SETAUTH (&THIS->user,CURLAUTH_BASIC);


END_METHOD

BEGIN_METHOD_VOID(CFTPCLIENT_free)

	ftp_reset(THIS);

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
  
  GB_CONSTANT("_Properties", "s", FTP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),

  
  GB_END_DECLARE
};

