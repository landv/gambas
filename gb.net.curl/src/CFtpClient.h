/***************************************************************************

  CFtpClient.h

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#ifndef __CFTPCLIENT_H
#define __CFTPCLIENT_H

#include "gambas.h"
#include "gbcurl.h"
#include "CCurl.h"
#include "CProxy.h"
#include <curl/curl.h>
#include <curl/easy.h>

#ifndef __CFTPCLIENT_C


extern GB_DESC CFtpClientDesc[];
extern GB_STREAM_DESC FtpStream;

#else

#define THIS_FTP            ((CFTPCLIENT *)_object)

#endif

typedef
	struct {
		CCURL curl;
		GB_ARRAY commands;
	}
	CFTPCLIENT;

#define FTP_PROPERTIES "URL=127.0.0.1:21,Async=TRUE,Timeout=0,User,Password"

#endif
