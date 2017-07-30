/***************************************************************************

  gbcurl.h

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

#ifndef __GB_CURL_H
#define __GB_CURL_H

#include <curl/curl.h>
#include "main.h"

typedef 
	struct
	{
		int type;
		int auth;
		char *host;
		char *user;
		char *pwd;
		char *userpwd;
	}
	CURL_PROXY;

typedef 
	struct
	{
		char *user;
		char *pwd;
		char *userpwd;
		int auth;
	} 
	CURL_USER;

char *CURL_get_protocol(char *url, char *default_protocol);

bool CURL_check_userpwd(CURL_USER *user);

void CURL_proxy_init(CURL_PROXY *proxy);
void CURL_proxy_clear(CURL_PROXY *proxy);
void CURL_proxy_set(CURL_PROXY *proxy, CURL *curl);
bool CURL_proxy_set_auth(CURL_PROXY *user, int auth);
bool CURL_proxy_set_type(CURL_PROXY *proxy, int type);

void CURL_user_init(CURL_USER *user);
void CURL_user_clear(CURL_USER *user);
void CURL_user_set(CURL_USER *user,CURL *curl);
bool CURL_user_set_auth(CURL_USER *user,int auth);

#endif



