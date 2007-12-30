/***************************************************************************

  CSmtpCLient.h

  gb.net.smtp component

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CSMTPCLIENT_H
#define __CSMTPCLIENT_H

#include "gambas.h"
#include "main.h"
#include "libsmtp.h"
#include "libsmtp_mime.h"

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
		struct libsmtp_part_struct *main;
		int parent;
		struct libsmtp_part_struct **parts;
		char **data;
		bool alternative;
	  }
	CSMTPCLIENT;

#ifdef __CSMTPCLIENT_C

#define THIS ((CSMTPCLIENT *)_object)

#else

extern GB_DESC CSmtpClientDesc[];

#endif

#endif
