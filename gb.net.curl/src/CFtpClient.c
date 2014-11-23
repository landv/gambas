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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CFTPCLIENT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>

#include "main.h"
#include "gambas.h"
#include "CCurl.h"
#include "CFtpClient.h"
#include "CProxy.h"

#define EXEC_GET 0
#define EXEC_PUT 1
#define EXEC_CMD 2


static int ftp_read_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{
	FILE *file = THIS_FILE;
	THIS_STATUS = NET_RECEIVING_DATA;
	
	if (!feof(file))
		nmemb = fread(buffer, size, nmemb, file);
	else
		nmemb = 0;
	
	return nmemb;
}


static int ftp_write_curl(void *buffer, size_t size, size_t nmemb, void *_object)
{
	THIS_STATUS = NET_RECEIVING_DATA;
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

static void ftp_reset(void *_object)
{
	GB.FreeString(&THIS->data);
	GB.Unref(&THIS_FTP->commands);
}


static void ftp_initialize_curl_handle(void *_object)
{
	if (THIS_CURL)
	{
		if (CURL_check_userpwd(&THIS->user))
		{
			CURL_stop(_object);
			ftp_reset(_object);
			THIS_CURL = curl_easy_init();
			#if DEBUG
			fprintf(stderr, "-- [%p] curl_easy_init() -> %p\n", THIS, THIS_CURL);
			#endif
		}
	}
	else
	{
		THIS_CURL = curl_easy_init();
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_easy_init() -> %p\n", THIS, THIS_CURL);
		#endif
	}

	curl_easy_setopt(THIS_CURL, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(THIS_CURL, CURLOPT_TIMEOUT, THIS->timeout);
	curl_easy_setopt(THIS_CURL, CURLOPT_VERBOSE, (bool)THIS->debug);
	curl_easy_setopt(THIS_CURL, CURLOPT_PRIVATE,(char*)_object);
	
	CURL_proxy_set(&THIS->proxy.proxy,THIS_CURL);
	CURL_user_set(&THIS->user, THIS_CURL);
	curl_easy_setopt(THIS_CURL, CURLOPT_URL,THIS_URL);

	curl_easy_setopt(THIS_CURL, CURLOPT_FTP_USE_EPSV, (long)(THIS_FTP->no_epsv ? 0 : 1));

	ftp_reset(THIS_FTP);
	THIS_STATUS = NET_CONNECTING;
	
	CURL_init_options(THIS);
	CURL_init_stream(THIS);
}


static int ftp_exec(void *_object, int what, GB_ARRAY commands)
{
	struct stat info;
	struct curl_slist *list;
	int i;

	if (THIS_STATUS > 0)
		return 1;

	THIS->method = what == EXEC_PUT ? 1 : 0;
	
	ftp_initialize_curl_handle(THIS);
	
	switch(what)
	{
		case EXEC_GET:
			
			curl_easy_setopt(THIS_CURL, CURLOPT_WRITEFUNCTION , (curl_write_callback)ftp_write_curl);
			curl_easy_setopt(THIS_CURL, CURLOPT_WRITEDATA     , _object);
			curl_easy_setopt(THIS_CURL, CURLOPT_UPLOAD        , 0);
			
			CURL_set_progress(THIS, TRUE, NULL);
			
			break;
			
		case EXEC_PUT:
			
			if (THIS_FILE && fstat(fileno(THIS_FILE), &info) == 0)
				curl_easy_setopt(THIS_CURL, CURLOPT_INFILESIZE_LARGE, (curl_off_t)info.st_size);
			
			curl_easy_setopt(THIS_CURL, CURLOPT_READFUNCTION , (curl_read_callback)ftp_read_curl);
			curl_easy_setopt(THIS_CURL, CURLOPT_READDATA     , _object);
			curl_easy_setopt(THIS_CURL, CURLOPT_UPLOAD       , 1);
			
			CURL_set_progress(THIS, TRUE, NULL);
			
			break;
			
		case EXEC_CMD:
			
			curl_easy_setopt(THIS_CURL, CURLOPT_NOBODY, 1);
			
			if (commands)
			{
				char *cmd;

				GB.Unref(&THIS_FTP->commands);
				THIS_FTP->commands = commands;
				GB.Ref(commands);
				
				list = NULL;
				for (i = 0; i < GB.Array.Count(commands); i++) {
					cmd = *((char **) GB.Array.Get(commands, i));
					if (!cmd)
						continue;
					list = curl_slist_append(list, cmd);
				}
				if (list)
					curl_easy_setopt(THIS_CURL, CURLOPT_QUOTE, list);
			}
			
			break;
	}
	
	if (THIS->async)
	{
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_multi_add_handle(%p)\n", THIS, THIS_CURL);
		#endif
		CURL_start_post(THIS);
		return 0;
	}
	
	CURL_manage_error(_object, curl_easy_perform(THIS_CURL));
	return 0;
}


BEGIN_METHOD(FtpClient_Get, GB_STRING target)

	const char *path = NULL;

	if (!MISSING(target))
		path = GB.FileName(STRING(target), LENGTH(target));
	
	if (path && *path)
	{
		if (THIS_STATUS > 0)
		{
			GB.Error("Still active");
			return;
		}
		
		THIS_FILE = fopen(path, "w");
		
		if (!THIS_FILE)
		{
			GB.Error("Unable to open file for writing");
			return;
		}
	}

	if (ftp_exec(THIS, EXEC_GET, NULL)) 
		GB.Error("Still active");

END_METHOD


BEGIN_METHOD(FtpClient_Put, GB_STRING SourceFile)
	
	if (THIS_STATUS > 0)
	{
		GB.Error("Still active");
		return;
	}
	
	THIS_FILE = fopen(GB.FileName(STRING(SourceFile), LENGTH(SourceFile)),"r");
	if (!THIS_FILE)
	{
		GB.Error("Unable to open file for reading");
		return;
	}

	if (ftp_exec(THIS, EXEC_PUT, NULL)) 
		GB.Error("Still active");

END_METHOD


BEGIN_METHOD(FtpClient_Exec, GB_OBJECT commands)

	if (THIS_STATUS > 0)
	{
		GB.Error("Still active");
		return;
	}
	
	ftp_exec(THIS, EXEC_CMD, VARG(commands));

END_METHOD


BEGIN_METHOD_VOID(FtpClient_Stop)

	CURL_stop(THIS);
	ftp_reset(THIS);

	GB.Ref(THIS);
	CURL_raise_cancel(THIS);

END_METHOD


BEGIN_METHOD_VOID(FtpClient_new)

	THIS_URL = GB.NewZeroString("ftp://127.0.0.1:21");

	THIS->async = TRUE;
	
	CURL_user_set_auth(&THIS->user, CURLAUTH_BASIC);
	
	THIS->user.user = GB.NewZeroString("anonymous");

END_METHOD


BEGIN_METHOD_VOID(FtpClient_free)

	//CURL_stop(_object);
	ftp_reset(THIS_FTP);

END_METHOD

BEGIN_PROPERTY(FtpClient_NoEPSV)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS_FTP->no_epsv);
	else
		THIS_FTP->no_epsv = VPROP(GB_BOOLEAN);

END_PROPERTY


GB_DESC CFtpClientDesc[] =
{
  GB_DECLARE("FtpClient", sizeof(CFTPCLIENT)),

  GB_INHERITS("Curl"),

  GB_METHOD("_new", NULL, FtpClient_new, NULL),
  GB_METHOD("_free", NULL, FtpClient_free, NULL),

  GB_METHOD("Stop", NULL, FtpClient_Stop, NULL),
  GB_METHOD("Get", NULL, FtpClient_Get, "[(TargetFile)s]"),
  GB_METHOD("Exec", NULL, FtpClient_Exec, "(Commands)String[]"),
  GB_METHOD("Put", NULL, FtpClient_Put, "(LocalFile)s"),
  
  GB_PROPERTY("NoEPSV", "b", FtpClient_NoEPSV),

  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", FTP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),
  
  GB_END_DECLARE
};

