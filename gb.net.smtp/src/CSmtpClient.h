/***************************************************************************

  CSmtpClient.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CSMTPCLIENT_H
#define __CSMTPCLIENT_H

#include "gambas.h"
#include "main.h"
#include "libsmtp.h"
#include "libsmtp_mime.h"

typedef
	struct {
		char *name;
		char *mime;
		char *data;
		struct libsmtp_part_struct *part;
		bool name_set;
		}
	CSMTPPART;
	
typedef
	struct {
		GB_BASE ob;
		struct libsmtp_session_struct *session;
	  char *host;
	  int port;
	  char *from;
	  GB_ARRAY to;
	  GB_ARRAY cc;
	  GB_ARRAY bcc;
	  char *subject;
	  CSMTPPART *parts;
		char *user;
		char *password;
		char *body;
		void *stream;
		bool alternative;
		bool debug;
		bool no_greeting;
	  }
	CSMTPCLIENT;

#ifdef __CSMTPCLIENT_C

#define THIS ((CSMTPCLIENT *)_object)

#else

extern GB_DESC CSmtpClientDesc[];

#endif

#endif
