/***************************************************************************

  CCurl.c

  Advanced Network component

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

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
#define __CCURL_C

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
#include "CProxy.h"


DECLARE_EVENT (CURL_FINISHED);
DECLARE_EVENT (CURL_ERROR);
DECLARE_EVENT (CURL_CONNECT);
DECLARE_EVENT (CURL_READ);

/*****************************************************
 CURLM : a pointer to use curl_multi interface,
 allowing asynchrnous work without using threads
 in this class.
 ******************************************************/
CURLM *CCURL_multicurl;
int CCURL_pipe[2]={-1,-1};

/******************************************************
 Events from this class
 ******************************************************/
GB_STREAM_DESC CurlStream = {
	open: CCURL_stream_open,
	close: CCURL_stream_close,
	read: CCURL_stream_read,
	write: CCURL_stream_write,
	seek: CCURL_stream_seek,
	tell: CCURL_stream_tell,
	flush: CCURL_stream_flush,
	eof: CCURL_stream_eof,
	lof: CCURL_stream_lof,
	handle: CCURL_stream_handle,
};

////////////////////////////////////////////////////////////////////
//	STREAM							  //
////////////////////////////////////////////////////////////////////

/* not allowed stream methods */

int CCURL_stream_handle(GB_STREAM *stream) { return 0;}
int CCURL_stream_open(GB_STREAM *stream, const char *path, int mode, void *data){return -1;}
int CCURL_stream_seek(GB_STREAM *stream, int64_t pos, int whence){	return -1;}
int CCURL_stream_tell(GB_STREAM *stream, int64_t *pos){return -1; }
int CCURL_stream_flush(GB_STREAM *stream) {	return 0;}
int CCURL_stream_close(GB_STREAM *stream) { return -1;}
int CCURL_stream_write(GB_STREAM *stream, char *buffer, int len){return -1;}

int CCURL_stream_lof(GB_STREAM *stream, int64_t *len)
{
	void *_object = STREAM_TO_OBJECT(stream);
	
	*len = 0;

	if ((THIS_STATUS !=4 ) && (THIS_STATUS != 0)) 
		return -1;
		
	*len = THIS->len_data;
	return 0;
}

int CCURL_stream_eof(GB_STREAM *stream)
{
	void *_object = STREAM_TO_OBJECT(stream);
	
	if ((THIS_STATUS !=4 ) && (THIS_STATUS != 0)) return -1;
	if (!THIS->len_data) return -1;
	return 0;
}

int CCURL_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = STREAM_TO_OBJECT(stream);
	
	if ((THIS_STATUS !=4 ) && (THIS_STATUS != 0)) return -1;
	if (THIS->len_data < len) return -1;

	memcpy(buffer,THIS->buf_data,len);

	if (THIS->len_data == len)
	{
		THIS->len_data=0;
		GB.Free(POINTER(&THIS->buf_data));
		return 0;
	}

	THIS->len_data-=len;
	memmove(THIS->buf_data,len+THIS->buf_data,THIS->len_data);
	GB.Realloc(POINTER(&THIS->buf_data),THIS->len_data);

	return 0;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

/*******************************************************************
####################################################################
	POSTED FUNCTIONS TO RAISE EVENTS
####################################################################
********************************************************************/
void CCURL_raise_finished(intptr_t lParam)
{
	void *mythis;
	mythis=(void*)lParam;
	GB.Raise(mythis,CURL_FINISHED,0);
	GB.Unref(&mythis);
}
void CCURL_raise_error(intptr_t lParam)
{
	void *mythis;
	mythis=(void*)lParam;
	GB.Raise(mythis,CURL_ERROR,0);
	GB.Unref(&mythis);
}
void CCURL_raise_connect(intptr_t lParam)
{
	void *mythis;
	mythis=(void*)lParam;
	GB.Raise(mythis,CURL_CONNECT,0);
	GB.Unref(&mythis);
}
void CCURL_raise_read(intptr_t lParam)
{
	void *mythis;
	mythis=(void*)lParam;
	GB.Raise(mythis,CURL_READ,0);
	GB.Unref(&mythis);
}


void CCURL_Manage_ErrCode(void *_object,long ErrCode)
{
	if (THIS_FILE)
	{
		fclose(THIS_FILE);
		THIS_FILE=NULL;
	}
		
	switch ( ErrCode )
	{
		case CURLE_OK:
			if (THIS->async) 
			{
				#if DEBUG
				fprintf(stderr, "-- [%p] curl_multi_remove_handle(%p)\n", THIS, THIS_CURL);
				#endif
				curl_multi_remove_handle(CCURL_multicurl,THIS_CURL);
			}
			GB.Ref(THIS);
			GB.Post(CCURL_raise_finished,(long)THIS);
			CCURL_stop(THIS);
			THIS_STATUS = 0;
			break;
		default:
			if (THIS->async) 
			{
				#if DEBUG
				fprintf(stderr, "-- [%p] curl_multi_remove_handle(%p)\n", THIS, THIS_CURL);
				#endif
				curl_multi_remove_handle(CCURL_multicurl,THIS_CURL);
			}
			GB.Ref(THIS);
			GB.Post(CCURL_raise_error,(long)THIS);
			CCURL_stop(THIS);
			THIS_STATUS = -1*(1000+ErrCode);
			break;
	}
}

void CCURL_init_stream(void *_object)
{
	THIS->stream.desc = &CurlStream;
	THIS->stream.tag = THIS;
}

/***************************************************************
 This CallBack is called each event loop by Gambas to test
 the status of curl descriptors
 ***************************************************************/

static void stop_post()
{
	if (CCURL_pipe[0] < 0) return;
	
	GB.Watch (CCURL_pipe[0] ,GB_WATCH_NONE,CCURL_post_curl,0);
	close(CCURL_pipe[0]);
	close(CCURL_pipe[1]);
	CCURL_pipe[0]=-1;
}
 
void CCURL_stop(void *_object)
{
	if (THIS_FILE)
	{
		fclose(THIS_FILE);
		THIS_FILE=NULL;
	}
	
	if (THIS_CURL)
	{
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_multi_remove_handle(%p)\n", THIS, THIS_CURL);
		#endif
		curl_multi_remove_handle(CCURL_multicurl,THIS_CURL);
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_easycleanup(%p)\n", THIS, THIS_CURL);
		#endif
		curl_easy_cleanup(THIS_CURL);
		THIS_CURL=NULL;
	}

	THIS_STATUS = 0;
}

void CCURL_init_post(void)
{
	if (CCURL_pipe[0]!=-1) return;
	
	pipe(CCURL_pipe);
	
	GB.Watch (CCURL_pipe[0] ,GB_WATCH_READ,CCURL_post_curl,0);
	write(CCURL_pipe[1],"1",sizeof(char));
}

void CCURL_post_curl(intptr_t data)
{
	CURLMsg *Msg;
	int nread;
	int post=1;
	void *_object;
	char *tmp;
	struct timespec mywait;

	do
	{
		mywait.tv_sec=0;
		mywait.tv_nsec=1000000;
		nanosleep(&mywait,NULL);	
	}
	while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(CCURL_multicurl,&nread));
	
	if (!nread) post=0;

	do
	{
		Msg=curl_multi_info_read(CCURL_multicurl,&nread);
		if (!Msg) nread=0;
		if (Msg)
		{
			curl_easy_getinfo(Msg->easy_handle,CURLINFO_PRIVATE,&tmp);
			_object=(void*)tmp;
			CCURL_Manage_ErrCode(THIS,Msg->data.result);
		}
	} 
	while (nread);

	if (!post)
		stop_post();
}

/*********************************************
 FTP User ( User:Password format )
 *********************************************/
BEGIN_PROPERTY ( CCURL_sUser )

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->user.user);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("User property is read-only while working");
		return;
  	}
	

	if ( THIS->user.user ) GB.FreeString ( &(THIS->user.user) );
	GB.StoreString(PROP(GB_STRING), &(THIS->user.user) );
	


END_PROPERTY


BEGIN_PROPERTY( CCURL_Async )

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->async);
		return;
	}
	
	if (THIS_STATUS > 0)
	{
		GB.Error ("Async property is read-only while working");
		return;
  	}
	
	THIS->async = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY( CCURL_TimeOut )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->TimeOut);
		return;
	}
	
	if (THIS_STATUS > 0)
	{
		GB.Error ("Timeout property is read-only while working");
		return;
  	}
	
	if (VPROP(GB_INTEGER)<0) 
		THIS->TimeOut=0;
	else
		THIS->TimeOut=VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY ( CCURL_Password )


	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->user.pwd);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("User property is read-only while working");
		return;
  	}

	
	if ( THIS->user.pwd ) GB.FreeString ( &(THIS->user.pwd) );
	GB.StoreString(PROP(GB_STRING), &(THIS->user.pwd) );

	
END_PROPERTY

/*********************************************
 Status : inactive, working or Error code
 *********************************************/
BEGIN_PROPERTY ( CCURL_Status )

	GB.ReturnInteger(THIS_STATUS);

END_PROPERTY

/*****************************************************************
 URL to work with
 *****************************************************************/


BEGIN_PROPERTY ( CCURL_URL )

	char *tmp=NULL;
	
	if (READ_PROPERTY)
	{
		GB.ReturnNewString(THIS_URL,0);
		return;
	}

	if (THIS_STATUS > 0)
	{
		GB.Error ("URL property is read-only while working");
		return;
  	}

	if (THIS_URL)
	{
		tmp=THIS_URL;
		GB.Free(POINTER(&tmp));
	}
	GB.Alloc(POINTER(&tmp),(strlen(GB.ToZeroString(PROP(GB_STRING)))+1)*sizeof(char));
	strcpy(tmp,GB.ToZeroString(PROP(GB_STRING)));
	Adv_correct_url(&tmp,THIS_PROTOCOL);
	THIS_URL=tmp;

END_PROPERTY

BEGIN_PROPERTY(CCURL_tag)

	if (READ_PROPERTY)
		GB.ReturnPtr(GB_T_VARIANT, &THIS->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD

BEGIN_METHOD_VOID(CCURL_new)

	#if DEBUG
	fprintf(stderr, "CCURL_new: %p\n", THIS);
	#endif

	/*curlData *data=NULL;
	
	GB.Alloc(POINTER(&data),sizeof(curlData));
	((void**)THIS->stream._free)[0]=(void*)data;*/

	THIS->stream.desc=NULL;
	THIS_CURL=NULL;
	THIS_URL=NULL;
	THIS_FILE=NULL;
	THIS_PROTOCOL=NULL;
	THIS_STATUS=0;
	GB.StoreVariant(NULL, (void *)&THIS->tag);
	Adv_user_NEW  (&THIS->user);
	Adv_proxy_NEW(&THIS->proxy.proxy);
	THIS->proxy.parent_status=(int*)&THIS_STATUS;

END_METHOD

BEGIN_METHOD_VOID(CCURL_free)
	
	#if DEBUG
	fprintf(stderr, "CCURL_free: %p\n", THIS);
	#endif
	
	char *tmp=THIS_URL;
	
	if (tmp) GB.Free(POINTER(&tmp));
	if (THIS_FILE) fclose(THIS_FILE);
	if (THIS_CURL) 
	{
		#if DEBUG
		fprintf(stderr, "-- [%p] curl_easy_cleanup(%p)\n", THIS, THIS_CURL);
		#endif
		curl_easy_cleanup(THIS_CURL);
	}
	Adv_user_CLEAR  (&THIS->user);
	Adv_proxy_CLEAR(&THIS->proxy.proxy);
	tmp=THIS_PROTOCOL;
	GB.Free(POINTER(&tmp));
	
END_METHOD

BEGIN_METHOD_VOID(CCURL_init)

	#if DEBUG
	fprintf(stderr, "-- curl_multi_init()\n");
	#endif
	CCURL_multicurl=curl_multi_init();

END_METHOD

BEGIN_METHOD_VOID(CCURL_exit)

	#if DEBUG
	fprintf(stderr, "-- curl_multi_cleanup()\n");
	#endif
	curl_multi_cleanup(CCURL_multicurl);

END_METHOD

BEGIN_METHOD_VOID(CCURL_Peek)

	if ( (THIS->len_data) && (THIS->buf_data) )
	{
		GB.ReturnNewString(THIS->buf_data,THIS->len_data);
		return;
	}
	GB.ReturnNewString(NULL,0);

END_METHOD

//*************************************************************************
//#################### GAMBAS INTERFACE ###################################
//*************************************************************************
GB_DESC CCurlDesc[] =
{

  GB_DECLARE("Curl", sizeof(CCURL)), GB_NOT_CREATABLE(),

  GB_INHERITS("Stream"),

  GB_METHOD("_new", NULL, CCURL_new, NULL),
  GB_METHOD("_free", NULL, CCURL_free, NULL),
  GB_METHOD("Peek","s", CCURL_Peek, NULL),
  
  GB_STATIC_METHOD("_init",NULL,CCURL_init, NULL),
  GB_STATIC_METHOD("_exit",NULL,CCURL_exit, NULL),

  GB_EVENT("Finished", NULL, NULL, &CURL_FINISHED),
  GB_EVENT("Connect", NULL, NULL, &CURL_CONNECT),
  GB_EVENT("Read", NULL, NULL, &CURL_READ),
  GB_EVENT("Error", NULL,NULL, &CURL_ERROR),

  GB_PROPERTY("URL", "s",CCURL_URL),
  GB_PROPERTY("User","s",CCURL_sUser),
  GB_PROPERTY("Password","s",CCURL_Password),  
  GB_PROPERTY("Tag", "v", CCURL_tag),
  GB_PROPERTY("Async","b",CCURL_Async),
  GB_PROPERTY("Timeout","i",CCURL_TimeOut),
  GB_PROPERTY_SELF("Proxy",".CurlProxy"),
  GB_PROPERTY_READ("Status","i",CCURL_Status),

  GB_END_DECLARE
};

