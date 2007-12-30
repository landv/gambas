/***************************************************************************

  gbcurl.h

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
#include <curl/curl.h>
#include "main.h"

#ifndef GB_CURL
#define GB_CURL
typedef void** Adv_ARRAY;

typedef struct
{
	int   type;
	int   auth;
	char *host;
	char *user;
	char *pwd;
	char *userpwd;
} Adv_proxy;

typedef struct
{
	char *user;
	char *pwd;
	char *userpwd;
	int auth;
} Adv_user;


void Adv_correct_url(char **buf,char *protocol);

void Adv_add_info      (Adv_ARRAY *Array, int *narray, void *Obj);
void Adv_remove_info   (Adv_ARRAY *Array,int *narray,void *Obj);

int Adv_Comp(char *str1,char *user, char *pwd);

void Adv_proxy_NEW     (Adv_proxy *proxy);
void Adv_proxy_CLEAR   (Adv_proxy *proxy);
void Adv_proxy_SET     (Adv_proxy *proxy,CURL *curl);
int  Adv_proxy_SETAUTH (Adv_proxy *user,int auth);
int  Adv_proxy_SETTYPE (Adv_proxy *proxy,int type);

void Adv_user_NEW      (Adv_user *user);
void Adv_user_CLEAR    (Adv_user *user);
void Adv_user_SET      (Adv_user *user,CURL *curl);
int  Adv_user_SETAUTH  (Adv_user *user,int auth);
#endif



