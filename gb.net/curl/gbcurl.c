/***************************************************************************

  gbcurl.c

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

/*****************************
 NOTE THAT :
 libcurl <= 7.10.7 lacks CURLE_LDAP_INVALID_URL and CURLE_FILESIZE_EXCEEDED constants
 libcurl <= 7.10.6 lacks proxy authentication support
 libcurl <= 7.10.5 lacks user authentication support
 *****************************/
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "gbcurl.h"

static char *_protocols[] = { "ftp://", "http://", "https://", NULL };

static void warning(const char *msg)
{
	fprintf(stderr, "gb.net.curl: warning: %s\n", msg);
}

#ifndef CURLAUTH_NONE
static void warning_auth(void)
{
	warning("This component was compiled without authentication support.");
	warning("Use libcurl version 7.10.7 or later.");
}
#define CURLAUTH_NONE 0
#define LACKS_AUTH
#warning #######################################################################
#warning COMPILING WITHOUT AUTHENTICATION SUPPORT - YOU MUST USE LIBCURL>=7.10.6
#warning #######################################################################
#endif


static void warning_proxy_auth(void)
{
	warning("This component was compiled without proxy authentication support.");
	warning("Use libcurl version 7.10.8 or later.");
}


char *CURL_get_protocol(char *url, char *default_protocol)
{
	char **p;
	char *pos;
	
	for (p = _protocols; *p; p++)
	{
		if (!strncmp(url, *p, strlen(*p)))
			return *p;
	}
	
	pos = strstr(url, "://");
	if (pos)
		return "?";
	
	return default_protocol;
}

#if 0
void Adv_correct_url(char **buf,char *protocol)
{
	char *buftmp;
	int len;
	int myloop,myloop2;
	int pos=-1;
	int myok=1;

	len=strlen(*buf);
	for (myloop=0;myloop<len;myloop++)
	{
		if ( (*buf)[myloop]==':' )
		{
			if (myloop==(len-1))
			{
				pos=myloop;
				break;
			}
			else
			{
				if ( (*buf)[myloop+1]=='/' )
				{
					pos=myloop;
					break;
				}
				else
				{
					for (myloop2=myloop+1;myloop2<len;myloop2++)
					{
						if ( (*buf)[myloop2]=='/' ) break;
						if ( ((*buf)[myloop2]<0x30) || ((*buf)[myloop2]>0x39) )
						{
							myok=0;
							break;
						}
					}
					if (!myok) pos=myloop;
					break;
				}
			}
		}
	}

	myok=0;

	if (pos==-1)
	{

		GB.Alloc((void**)POINTER(&buftmp),len+1);
		strcpy(buftmp,*buf);
		GB.Free((void**)POINTER(buf));
		GB.Alloc((void**)POINTER(buf),len+strlen(protocol)+1);
	}
	else
	{
		GB.Alloc((void**)POINTER(&buftmp),(len-pos)+1);
		strcpy(buftmp,*buf+pos+1);
		GB.Free((void**)POINTER(buf));
		GB.Alloc((void**)POINTER(buf),strlen(buftmp)+strlen(protocol)+1);
	}

	strcpy(*buf,protocol);
	if (strlen(buftmp)>=2)
	{
		if ( buftmp[0]=='/') myok++;
		if ( buftmp[1]=='/') myok++;
	}
	strcat(*buf,buftmp+myok);
	GB.Free((void**)POINTER(&buftmp));
}
#endif

bool CURL_check_userpwd(CURL_USER *user)
{
	char *tmp = NULL;
	bool ret;
	
	if (user->user || user->pwd)
	{
		tmp = GB.AddString(tmp, user->user, 0);
		tmp = GB.AddChar(tmp, ':');
		tmp = GB.AddString(tmp, user->pwd, 0);
	}
	
	if (tmp && user->userpwd)
		ret = (strcmp(tmp, user->userpwd) != 0);
	else
		ret = (tmp == user->userpwd);
	
	GB.FreeString(&tmp);
	return ret;
}

/***************************************************************************/

void CURL_proxy_init(CURL_PROXY *proxy)
{
	proxy->type = CURLPROXY_HTTP;
	proxy->auth = CURLAUTH_NONE;
	proxy->user = NULL;
	proxy->pwd = NULL;
	proxy->host = NULL;
	proxy->userpwd = NULL;
}


void CURL_proxy_clear(CURL_PROXY *proxy)
{
	GB.FreeString(&proxy->host);
	GB.FreeString(&proxy->user);
	GB.FreeString(&proxy->pwd);
	GB.FreeString(&proxy->userpwd);
}


void CURL_proxy_set(CURL_PROXY *proxy, CURL *curl)
{
	GB.FreeString(&proxy->userpwd);
	
	if (proxy->user || proxy->pwd)
	{
		proxy->userpwd = GB.AddString(proxy->userpwd, proxy->user, 0);
		proxy->userpwd = GB.AddChar(proxy->userpwd, ':');
		proxy->userpwd = GB.AddString(proxy->userpwd, proxy->pwd, 0);
	}
	
	if (!proxy->host)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, NULL);
		if ( LIBCURL_VERSION_NUM >= 0x070a08 )
			curl_easy_setopt(curl, 111, CURLAUTH_NONE);
		return;
	}

	curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxy->type);
	curl_easy_setopt(curl, CURLOPT_PROXY, proxy->host);
	if ( LIBCURL_VERSION_NUM >= 0x070a08 )
	{
		curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy->userpwd);
		curl_easy_setopt(curl, 111, proxy->auth);
	}
	else
		warning_proxy_auth();
}

bool CURL_proxy_set_auth(CURL_PROXY *proxy, int auth)
{
	#ifdef LACKS_AUTH
	return FALSE;
	#else
	if (LIBCURL_VERSION_NUM < 0x070a08)
	{
		if (auth) 
			warning_proxy_auth();
		return FALSE;
	}
	
	switch (auth)
	{
		case CURLAUTH_NONE:
		case CURLAUTH_BASIC:
		case CURLAUTH_NTLM:
			proxy->auth=auth; 
			return FALSE;
		default: 
			return TRUE;
	}
	#endif
}

bool CURL_proxy_set_type(CURL_PROXY *proxy, int type)
{
	switch (type)
	{
		case CURLPROXY_HTTP:
		case CURLPROXY_SOCKS5:
			proxy->type = type; 
			return FALSE;
		default: 
			return TRUE;
	}
}

/***************************************************************************/

void CURL_user_init(CURL_USER *user)
{
	user->auth = CURLAUTH_NONE;
	user->user = NULL;
	user->pwd = NULL;
	user->userpwd = NULL;
}


void CURL_user_clear(CURL_USER *user)
{
	GB.FreeString(&user->user);
	GB.FreeString(&user->pwd);
	GB.FreeString(&user->userpwd);
}


void CURL_user_set(CURL_USER *user, CURL *curl)
{
	#ifdef LACKS_AUTH
	return;
	#else
	
	if (user->auth == CURLAUTH_NONE)
	{
		curl_easy_setopt(curl, CURLOPT_USERPWD, NULL);
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_NONE);
		return;
	}
	
	GB.FreeString(&user->userpwd);
	user->userpwd = GB.AddString(user->userpwd, user->user, 0);
	user->userpwd = GB.AddChar(user->userpwd, ':');
	user->userpwd = GB.AddString(user->userpwd, user->pwd, 0);

	curl_easy_setopt(curl, CURLOPT_USERPWD, user->userpwd);
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, user->auth);
	
	#endif
}

bool CURL_user_set_auth(CURL_USER *user, int auth)
{
	#ifdef LACKS_AUTH
	if (auth) 
		warning_auth();
	return FALSE;
	#else
	switch (auth)
	{
		case CURLAUTH_NONE:
		case CURLAUTH_BASIC:
		case CURLAUTH_NTLM:
		case CURLAUTH_GSSNEGOTIATE:
		case CURLAUTH_DIGEST:
			user->auth = auth;
			return FALSE;
			
		default: 
			return TRUE;
	}
	#endif
}
