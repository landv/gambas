/***************************************************************************

  CHttpClient.c

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>
  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CHTTPCLIENT_C

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
#include "CHttpClient.h"
#include "CProxy.h"

#define ERR_STILL_ACTIVE "Still active"
#define ERR_INVALID_CONTENT_TYPE "Invalid content type"
#define ERR_INVALID_DATA "Invalid data"

#define SEND_POST 1
#define SEND_PUT 2
#define SEND_FILE 4

static void http_parse_header(CHTTPCLIENT *_object)
{
	char *header;
	int len;
	char *p;
	int ret;

	if (!THIS_HTTP->headers || GB.Array.Count(THIS_HTTP->headers) == 0)
		return;
	
	header = *(char **)GB.Array.Get(THIS_HTTP->headers, 0);
	len = GB.StringLength(header);

	p = strchr(header, ' ');
	if (!p)
		return;
	p++;
	
	ret = 0;
	while (isdigit(*p))
	{
		ret = (ret * 10) + (*p - '0');
		p++;
	}
	
	if (*p != ' ')
		return;
	
	THIS_HTTP->return_code = ret;
	//GB.FreeString(&THIS_HTTP->return_string);
	THIS_HTTP->return_string = GB.NewString(p, header + len - p);
}


static int http_header_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{
	if (!THIS_HTTP->headers)
	{
		GB.Array.New(&THIS_HTTP->headers, GB_T_STRING, 0);
		GB.Ref(THIS_HTTP->headers);
	}

	if (nmemb > 2)
		*(char **)GB.Array.Add(THIS_HTTP->headers) = GB.NewString(buffer, (nmemb - 2) * size);
	
	if ((THIS_STATUS == NET_CONNECTING) && THIS->async)
	{
		THIS_STATUS = NET_RECEIVING_DATA;
		GB.Ref(THIS);
		GB.Post(CURL_raise_connect, (intptr_t)THIS);
	}

	return size * nmemb;
}


static size_t http_read_curl(void *ptr, size_t size, size_t nmemb, void *_object)
{
	size *= nmemb;
	
	if (size > (THIS_HTTP->len_data - THIS_HTTP->len_sent))
		size = THIS_HTTP->len_data - THIS_HTTP->len_sent;
	
	if (size > 0)
	{
		memcpy(ptr, THIS_HTTP->data + THIS_HTTP->len_sent, size);
		THIS_HTTP->len_sent += size;
	}
	
	return size;
}


static int http_write_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{
	if (!THIS_HTTP->return_code) 
		http_parse_header(THIS_HTTP);

	nmemb *= size;
	
	if (THIS_FILE)
	{
		return fwrite(buffer,size,nmemb,THIS_FILE);
	}
	else
	{
		THIS->data = GB.AddString(THIS->data, buffer, nmemb);
	}

	if (THIS->async)
	{
		GB.Ref(THIS);
		GB.Post(CURL_raise_read, (intptr_t)THIS);
	}
	
	return nmemb;
}


static void http_reset(void *_object)
{
	GB.FreeString(&THIS->data);
	
	GB.Unref(&THIS_HTTP->headers);
	THIS_HTTP->headers = NULL;
	GB.Unref(&THIS_HTTP->sent_headers);
	THIS_HTTP->sent_headers = NULL;
	
	if (THIS_HTTP->sContentType)
	{
		GB.Free((void**)POINTER(&THIS_HTTP->sContentType));
		THIS_HTTP->sContentType=NULL;
	}
	
	if (THIS_HTTP->data)
	{
		if (THIS_HTTP->send_file)
			GB.ReleaseFile(THIS_HTTP->data, THIS_HTTP->len_data);
		else
			GB.Free(POINTER(&THIS_HTTP->data));
		THIS_HTTP->data = NULL;
	}
	
	THIS_HTTP->send_file = FALSE;
}


static void http_initialize_curl_handle(void *_object, GB_ARRAY custom_headers)
{
	if (THIS_CURL)
	{
		if (CURL_check_userpwd(&THIS->user))
		{
			CURL_stop(_object);
			http_reset(_object);
			THIS_CURL = curl_easy_init();
		}
	}
	else
	{
		THIS_CURL = curl_easy_init();
	}
	
	curl_easy_setopt(THIS_CURL, CURLOPT_NOSIGNAL,1);
	curl_easy_setopt(THIS_CURL, CURLOPT_TIMEOUT,THIS->timeout);
	curl_easy_setopt(THIS_CURL, CURLOPT_VERBOSE, (bool)THIS->debug);
	curl_easy_setopt(THIS_CURL, CURLOPT_PRIVATE,(char*)_object);
	curl_easy_setopt(THIS_CURL, CURLOPT_USERAGENT, THIS_HTTP->sUserAgent);
	curl_easy_setopt(THIS_CURL, CURLOPT_ENCODING, THIS_HTTP->encoding);
	curl_easy_setopt(THIS_CURL, CURLOPT_HEADERFUNCTION, (curl_write_callback)http_header_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEFUNCTION, (curl_write_callback)http_write_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEDATA, _object);
	curl_easy_setopt(THIS_CURL, CURLOPT_WRITEHEADER, _object);
	curl_easy_setopt(THIS_CURL, CURLOPT_COOKIEFILE, THIS_HTTP->cookiesfile);
	
	if (THIS_HTTP->updatecookies)
		curl_easy_setopt(THIS_CURL, CURLOPT_COOKIEJAR, THIS_HTTP->cookiesfile);
	else
		curl_easy_setopt(THIS_CURL, CURLOPT_COOKIEJAR, NULL);

	CURL_proxy_set(&THIS->proxy.proxy, THIS_CURL);
	CURL_user_set(&THIS->user, THIS_CURL);
	curl_easy_setopt(THIS_CURL, CURLOPT_URL, THIS_URL);
	
	THIS_HTTP->return_code = 0;
	GB.FreeString(&THIS_HTTP->return_string);

	http_reset(_object);
	THIS_STATUS = NET_CONNECTING;
	
	if (custom_headers)
	{
		GB.Unref(&THIS_HTTP->sent_headers);
		THIS_HTTP->sent_headers = custom_headers;
		GB.Ref(custom_headers);
	}
	
	CURL_init_options(THIS);
	CURL_init_stream(THIS);
}


static bool check_request(void *_object, char *contentType, char *data, int len)
{
	int i;
	unsigned char c;
	
	if (THIS_STATUS > 0)
	{ 
		GB.Error(ERR_STILL_ACTIVE); 
		return TRUE; 
	}
	
	if (!contentType)
	{ 
		GB.Error(ERR_INVALID_CONTENT_TYPE);
		return TRUE;
	}
	
	for (i = 0; i < strlen(contentType); i++)
	{
		c = contentType[i];
		if (isalnum(c) || c == '-' || c == '+' || c == '.' || c == '/' || c == ';' || c == ' ' || c == '=')
			continue;
		GB.Error(ERR_INVALID_CONTENT_TYPE);
		return TRUE;
	}

	/*if (!data || !len)
	{ 
		GB.Error(ERR_INVALID_DATA); 
		return TRUE; 
	};*/
	
	return FALSE;
}

static void http_fix_progress_cb(void *_object, double *dltotal, double *dlnow, double *ultotal, double *ulnow)
{
	*ultotal = THIS_HTTP->len_data;
	*ulnow = THIS_HTTP->len_sent;
}

static void http_get(void *_object, GB_ARRAY custom_headers, char *target)
{
	struct curl_slist *headers = NULL;
	int i;

	if (THIS_STATUS > 0)
	{ 
		GB.Error(ERR_STILL_ACTIVE); 
		return; 
	}

	if (!target)
		target = THIS_HTTP->target;

	if (target && *target)
	{
		target = GB.FileName(target, 0);
		THIS_FILE = fopen(target, "w");
		if (!THIS_FILE)
		{
			GB.Error("Unable to open file for writing: &1", target);
			return;
		}
	}

	THIS->method=0;
	
	http_initialize_curl_handle(_object, custom_headers);
	
	curl_easy_setopt(THIS_CURL, CURLOPT_HTTPGET, 1);
	
	if (THIS_HTTP->sent_headers)
	{
		for(i = 0; i < GB.Array.Count(THIS_HTTP->sent_headers); i++)
			headers = curl_slist_append(headers, *(char **)GB.Array.Get(THIS_HTTP->sent_headers, i));
	}
	
	curl_easy_setopt(THIS_CURL, CURLOPT_HTTPHEADER, headers);
	CURL_set_progress(THIS, TRUE, NULL);

	if (THIS->async)
	{
		CURL_start_post(THIS);
		return;
	}
	
	CURL_manage_error(_object, curl_easy_perform(THIS_CURL));
}


static void http_send(void *_object, int type, char *sContent, char *sData, int lendata, GB_ARRAY custom_headers, char *target)
{
	int mylen;
	struct curl_slist *headers = NULL;
	int i;
	
	if (check_request(_object, sContent, sData, lendata))
		return;

	if (!target)
		target = THIS_HTTP->target;

	if (target && *target)
	{
		target = GB.FileName(target, 0);
		THIS_FILE = fopen(target, "w");
		if (!THIS_FILE)
		{
			GB.Error("Unable to open file for writing: &1", target);
			return;
		}
	}
	
	http_initialize_curl_handle(_object, custom_headers);

	if (type & SEND_FILE)
	{
		if (GB.LoadFile(sData, lendata, &THIS_HTTP->data, &mylen))
			return;
		
		THIS_HTTP->len_data = mylen;
		THIS_HTTP->send_file = TRUE;
	}
	else
	{
		THIS_HTTP->send_file = FALSE;
		THIS_HTTP->len_data = lendata;
		
		if (lendata)
		{
			GB.Alloc((void*)&THIS_HTTP->data, lendata + 1);
			strncpy(THIS_HTTP->data, sData, lendata);
		}
		else
			THIS_HTTP->data = NULL;
		
	}
	
	THIS_HTTP->len_sent = 0;

	mylen = strlen(sContent) + strlen("Content-Type: ") + 1;
	GB.Alloc((void*)&THIS_HTTP->sContentType, mylen);
	
	THIS_HTTP->sContentType[0] = 0;
	strcpy(THIS_HTTP->sContentType, "Content-Type: " );
	strcat(THIS_HTTP->sContentType, sContent);
	
	THIS->method = 1;
	headers = curl_slist_append(headers, THIS_HTTP->sContentType);
	
	if (THIS_HTTP->sent_headers)
	{
		for (i = 0; i < GB.Array.Count(THIS_HTTP->sent_headers); i++)
			headers = curl_slist_append(headers, *(char **)GB.Array.Get(THIS_HTTP->sent_headers, i));
	}
	
	curl_easy_setopt(THIS_CURL, CURLOPT_HTTPHEADER, headers);
	
	if (type == SEND_PUT)
	{
		curl_easy_setopt(THIS_CURL, CURLOPT_INFILESIZE_LARGE, (curl_off_t)lendata);
		curl_easy_setopt(THIS_CURL, CURLOPT_UPLOAD, 1);
	}
	else // SEND_POST
	{
		curl_easy_setopt(THIS_CURL, CURLOPT_POSTFIELDSIZE, lendata);
		curl_easy_setopt(THIS_CURL, CURLOPT_POSTFIELDS, NULL);
	}

	curl_easy_setopt(THIS_CURL, CURLOPT_READFUNCTION, http_read_curl);
	curl_easy_setopt(THIS_CURL, CURLOPT_READDATA, _object);

	CURL_set_progress(THIS, TRUE, http_fix_progress_cb);

	if (THIS->async)
	{
		CURL_start_post(THIS);
		return;
	}
	
	CURL_manage_error(THIS, curl_easy_perform(THIS_CURL));
}


//-------------------------------------------------------------------------

BEGIN_PROPERTY(HttpClient_UpdateCookies)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS_HTTP->updatecookies);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("UpdateCookies property can not be changed if the client is active");
		return;
  	}

	if (VPROP(GB_BOOLEAN))
		THIS_HTTP->updatecookies=1;
	else
		THIS_HTTP->updatecookies=0;

END_PROPERTY


BEGIN_PROPERTY(HttpClient_CookiesFile)

	const char *file;

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS_HTTP->cookiesfile);
		return;
	}

	if (CURL_check_active(THIS))
		return;

	if (THIS_HTTP->cookiesfile)
		GB.FreeString(&THIS_HTTP->cookiesfile);
	
	file = GB.FileName(PSTRING(), PLENGTH());
	
	if (file)
		THIS_HTTP->cookiesfile = GB.NewZeroString(file);

END_PROPERTY


BEGIN_PROPERTY(HttpClient_Auth)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS_HTTP->auth);
		return;
	}

	if (CURL_check_active(THIS))
		return;

	if (CURL_user_set_auth(&THIS->user, VPROP(GB_INTEGER)))
		GB.Error ("Unknown authentication method");
	else
		THIS_HTTP->auth = VPROP(GB_INTEGER);


END_PROPERTY


BEGIN_PROPERTY(HttpClient_UserAgent)

	if (READ_PROPERTY)
		GB.ReturnString(THIS_HTTP->sUserAgent);
	else
	{
		if (CURL_check_active(THIS))
			return;
		
		GB.StoreString(PROP(GB_STRING), &THIS_HTTP->sUserAgent);
	}

END_PROPERTY


BEGIN_PROPERTY(HttpClient_Encoding)

	if (READ_PROPERTY)
		GB.ReturnString(THIS_HTTP->encoding);
	else
	{
		if (CURL_check_active(THIS))
			return;
		
		GB.StoreString(PROP(GB_STRING), &THIS_HTTP->encoding);
	}

END_PROPERTY


BEGIN_PROPERTY(HttpClient_ReturnCode)

	GB.ReturnInteger(THIS_HTTP->return_code);

END_PROPERTY


BEGIN_PROPERTY(HttpClient_ReturnString)

	GB.ReturnString(THIS_HTTP->return_string);

END_PROPERTY


BEGIN_PROPERTY(HttpClient_Headers)

	GB.ReturnObject(THIS_HTTP->headers);

END_PROPERTY


BEGIN_METHOD_VOID(HttpClient_new)

	THIS_URL = GB.NewZeroString("http://127.0.0.1:80");
	THIS_HTTP->sUserAgent = GB.NewZeroString("Gambas/" GAMBAS_FULL_VERSION_STRING " (gb.net.curl; " SYSTEM ")");
	THIS->async = TRUE;
	
END_METHOD


BEGIN_METHOD_VOID(HttpClient_free)

	//CURL_stop(_object);
	http_reset(THIS);
	
	GB.FreeString(&THIS_HTTP->sUserAgent);
	GB.FreeString(&THIS_HTTP->encoding);
	GB.FreeString(&THIS_HTTP->cookiesfile);
	GB.FreeString(&THIS_HTTP->return_string);
	GB.FreeString(&THIS_HTTP->target);

END_METHOD


BEGIN_METHOD(HttpClient_Get, GB_OBJECT headers; GB_STRING target)

	http_get(THIS, VARGOPT(headers, 0), MISSING(target) ? NULL : GB.ToZeroString(ARG(target)));

END_METHOD


BEGIN_METHOD(HttpClient_Post, GB_STRING contentType; GB_STRING data; GB_OBJECT headers; GB_STRING target)

	http_send(THIS, SEND_POST, GB.ToZeroString(ARG(contentType)), STRING(data), LENGTH(data), VARGOPT(headers, NULL), MISSING(target) ? NULL : GB.ToZeroString(ARG(target)));

END_METHOD


BEGIN_METHOD(HttpClient_PostFile, GB_STRING contentType; GB_STRING file; GB_OBJECT headers; GB_STRING target)

	http_send(THIS, SEND_POST | SEND_FILE, GB.ToZeroString(ARG(contentType)), STRING(file), LENGTH(file), VARGOPT(headers, NULL), MISSING(target) ? NULL : GB.ToZeroString(ARG(target)));

END_METHOD


BEGIN_METHOD(HttpClient_Put, GB_STRING contentType; GB_STRING data; GB_OBJECT headers; GB_STRING target)

	http_send(THIS, SEND_PUT, GB.ToZeroString(ARG(contentType)), STRING(data), LENGTH(data), VARGOPT(headers, NULL), MISSING(target) ? NULL : GB.ToZeroString(ARG(target)));

END_METHOD


BEGIN_METHOD(HttpClient_PutFile, GB_STRING contentType; GB_STRING file; GB_OBJECT headers; GB_STRING target)

	http_send(THIS, SEND_PUT | SEND_FILE, GB.ToZeroString(ARG(contentType)), STRING(file), LENGTH(file), VARGOPT(headers, NULL), MISSING(target) ? NULL : GB.ToZeroString(ARG(target)));

END_METHOD


BEGIN_METHOD_VOID(HttpClient_Stop)

	CURL_stop(THIS);
	http_reset(_object);

	GB.Ref(THIS);
	CURL_raise_cancel(THIS);

END_METHOD

#define COPY_STRING(_field) \
{ \
	GB.FreeString(&THIS_HTTP->_field); \
	THIS_HTTP->_field = from->_field; \
	if (THIS_HTTP->_field) THIS_HTTP->_field = GB.NewString(THIS_HTTP->_field, GB.StringLength(THIS_HTTP->_field)); \
}

BEGIN_METHOD(HttpClient_CopyFrom, GB_OBJECT from)

	CHTTPCLIENT *from = (CHTTPCLIENT *)VARG(from);

	if (GB.CheckObject(from))
		return;

	if (CURL_copy_from((CCURL *)THIS, (CCURL *)from))
		return;

	THIS_HTTP->updatecookies = from->updatecookies;
	THIS_HTTP->auth = from->auth;
	COPY_STRING(sUserAgent);
	COPY_STRING(encoding);
	COPY_STRING(cookiesfile);

END_METHOD


BEGIN_PROPERTY(HttpClient_TargetFile)

	if (READ_PROPERTY)
		GB.ReturnString(THIS_HTTP->target);
	else
		GB.StoreString(PROP(GB_STRING), &THIS_HTTP->target);

END_PROPERTY


GB_DESC CHttpClientDesc[] =
{
  GB_DECLARE("HttpClient", sizeof(CHTTPCLIENT)),

  GB_INHERITS("Curl"),

  GB_METHOD("_new", NULL, HttpClient_new, NULL),
  GB_METHOD("_free", NULL, HttpClient_free, NULL),
  GB_METHOD("Stop", NULL, HttpClient_Stop, NULL),
  GB_METHOD("Get", NULL, HttpClient_Get, "[(Headers)String[];(TargetFile)s]"),
  GB_METHOD("Post", NULL, HttpClient_Post, "(ContentType)s(Data)s[(Headers)String[];(TargetFile)s]"),
  GB_METHOD("Put", NULL, HttpClient_Put, "(ContentType)s(Data)s[(Headers)String[];(TargetFile)s]"),
  GB_METHOD("PostFile", NULL, HttpClient_PostFile, "(ContentType)s(Path)s[(Headers)String[];(TargetFile)s]"),
  GB_METHOD("PutFile", NULL, HttpClient_PutFile, "(ContentType)s(Path)s[(Headers)String[];(TargetFile)s]"),

  GB_PROPERTY("Auth", "i", HttpClient_Auth),
  GB_PROPERTY("CookiesFile", "s",HttpClient_CookiesFile),
  GB_PROPERTY("UpdateCookies", "b",HttpClient_UpdateCookies),
  GB_PROPERTY_READ("Headers", "String[]", HttpClient_Headers),
  GB_PROPERTY("UserAgent", "s", HttpClient_UserAgent),
  GB_PROPERTY("Encoding", "s", HttpClient_Encoding),
  GB_PROPERTY("TargetFile", "s", HttpClient_TargetFile),

  GB_PROPERTY_READ("Code", "i", HttpClient_ReturnCode),
  GB_PROPERTY_READ("Reason", "s", HttpClient_ReturnString),

  GB_METHOD("CopyFrom", NULL, HttpClient_CopyFrom, "(HttpClient)Source"),
  
  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", HTTP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),
  
  GB_END_DECLARE
};
