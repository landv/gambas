/***************************************************************************

  gbcurl.c

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


#ifndef CURLAUTH_NONE
void Adv_WarningAuth(void)
{
	printf("WARNING : This component was compiled without authentication support\n");
	printf("          use libcurl version 7.10.7 or later\n");
}
#define CURLAUTH_NONE 0
#define LACKS_AUTH
#warning #######################################################################
#warning COMPILING WITHOUT AUTHENTICATION SUPPORT - YOU MUST USE LIBCURL>=7.10.6
#warning #######################################################################
#endif


void Adv_WarningProxyAuth(void)
{
	printf("WARNING : This component was compiled without proxy authentication support\n");
	printf("          use libcurl version 7.10.8 or later\n");
}



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


int Adv_Comp(char *str1,char *user, char *pwd)
{
	char *str2=NULL;
	int len=2;

	if (user || pwd)
	{
		if (user) len+=strlen(user);
		if (pwd) len+=strlen(pwd);
		GB.Alloc ((void**)POINTER(&str2),len);
		str2[0]=0;
		if (user) strcat (str2,user);
		strcat(str2,":");
		if (pwd) strcat (str2,pwd);

	}

	if ( (!str1) && (!str2) ) return 0;

	if (!str1)
	{
		if ( strlen(str2) )
		{
			GB.Free((void**)POINTER(&str2));
			return 1;
		}
		GB.Free((void**)POINTER(&str2));
		return 0;
	}

	if (!str2)
	{
		if ( strlen(str1) ) return 1;
		return 0;
	}

	if ( strcmp (str1,str2) )
	{
		GB.Free((void**)POINTER(&str2));
		return 1;
	}

	GB.Free((void**)POINTER(&str2));
	return 0;
}


/************************************************************************
 PROXY
 ************************************************************************/
void Adv_proxy_NEW(Adv_proxy *proxy)
{
	proxy->type=CURLPROXY_HTTP;
	proxy->auth=CURLAUTH_NONE;
	proxy->user=NULL;
	proxy->pwd=NULL;
	proxy->host=NULL;
	proxy->userpwd=NULL;
}


void Adv_proxy_CLEAR(Adv_proxy *proxy)
{
	if (proxy->host) GB.FreeString(&proxy->host);
	if (proxy->user) GB.FreeString(&proxy->user);
	if (proxy->pwd)  GB.FreeString(&proxy->pwd);
	if (proxy->userpwd) GB.Free((void**)POINTER(&proxy->userpwd));
	proxy->user=NULL;
	proxy->host=NULL;
	proxy->pwd=NULL;
	proxy->userpwd=NULL;
}


void Adv_proxy_SET(Adv_proxy *proxy,CURL *curl)
{
	int len=2;

	if ( proxy->user ) len+=strlen(proxy->user);
	if ( proxy->pwd )  len+=strlen(proxy->pwd);
	if ( proxy->userpwd ) GB.Free ((void**)POINTER(&proxy->userpwd));

	GB.Alloc ((void**)POINTER(&proxy->userpwd),sizeof(char)*len);
	proxy->userpwd[0]=0;

	if ( proxy->user ) strcat(proxy->userpwd,proxy->user);
	strcat(proxy->userpwd,":");
	if ( proxy->pwd ) strcat(proxy->userpwd,proxy->pwd);

	if ( !proxy->host )
	{
		curl_easy_setopt(curl,CURLOPT_PROXY,NULL);
		if ( LIBCURL_VERSION_NUM >= 0x070a08 )
			curl_easy_setopt(curl,111,CURLAUTH_NONE);
		return;
	}


	curl_easy_setopt(curl,CURLOPT_PROXYTYPE,proxy->type);
	curl_easy_setopt(curl,CURLOPT_PROXY,proxy->host);
	if ( LIBCURL_VERSION_NUM >= 0x070a08 )
	{
		curl_easy_setopt(curl,CURLOPT_PROXYUSERPWD,proxy->userpwd);
		curl_easy_setopt(curl,111,proxy->auth);
	}
	else
		Adv_WarningProxyAuth();



}

int Adv_proxy_SETAUTH(Adv_proxy *proxy,int auth)
{
	#ifdef LACKS_AUTH
	return 0;
	#else
	if ( LIBCURL_VERSION_NUM < 0x070a08 )
	{
		if (auth!=0) Adv_WarningProxyAuth();
		return 0;
	}
	switch (auth)
	{
		case CURLAUTH_NONE:
		case CURLAUTH_BASIC:
		case CURLAUTH_NTLM:   proxy->auth=auth; return 0;
		default: return -1;
	}
	#endif

}

int Adv_proxy_SETTYPE(Adv_proxy *proxy,int type)
{
	switch (type)
	{
		case CURLPROXY_HTTP:
		case CURLPROXY_SOCKS5:   proxy->type=type; return 0;
		default: return -1;
	}
}



/************************************************************************
 USERS
 ************************************************************************/
void Adv_user_NEW(Adv_user *user)
{
	user->auth=CURLAUTH_NONE;
	user->user=NULL;
	user->pwd=NULL;
	user->userpwd=NULL;
}


void Adv_user_CLEAR(Adv_user *user)
{
	if (user->user) GB.FreeString(&user->user);
	if (user->pwd)  GB.FreeString(&user->pwd);
	if (user->userpwd) GB.Free((void**)POINTER(&user->userpwd));
	user->user=NULL;
	user->pwd=NULL;
}


void Adv_user_SET(Adv_user *user,CURL *curl)
{
	#ifdef LACKS_AUTH
	return;
	#else
	int len=2;
	
	if (user->auth==CURLAUTH_NONE)
	{
		curl_easy_setopt(curl,CURLOPT_USERPWD,NULL);
		curl_easy_setopt(curl,CURLOPT_HTTPAUTH,CURLAUTH_NONE);
		return;
	}
	

	if (user->user) len += strlen(user->user);
	if (user->auth) len += strlen(user->pwd);
	if (user->userpwd) GB.Free((void**)POINTER(&user->userpwd));
	GB.Alloc ((void**)POINTER(&user->userpwd),len);
	user->userpwd[0]=0;
	if (user->user) strcat (user->userpwd,user->user);
	strcat (user->userpwd,":");
	if (user->pwd) strcat (user->userpwd,user->pwd);

	curl_easy_setopt(curl,CURLOPT_USERPWD,user->userpwd);
	curl_easy_setopt(curl,CURLOPT_HTTPAUTH,user->auth);
	#endif
}

int Adv_user_SETAUTH(Adv_user *user,int auth)
{
	#ifdef LACKS_AUTH
	if (auth!=0) Adv_WarningAuth();
	return 0;
	#else
	switch (auth)
	{
		case CURLAUTH_NONE:
		case CURLAUTH_BASIC:
		case CURLAUTH_NTLM:
		case CURLAUTH_GSSNEGOTIATE:
		case CURLAUTH_DIGEST:         user->auth=auth; return 0;
		default: return -1;
	}
	#endif
}
