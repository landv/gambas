/***************************************************************************

  main.c

  Advanced Network Component

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

#define __MAIN_C

#include <stdio.h>
#include <curl/curl.h>

#include "CCurl.h"
#include "CHttpClient.h"
#include "CFtpClient.h"
#include "CProxy.h"
#include "CNet.h"

#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif



GB_INTERFACE GB EXPORT;



GB_DESC *GB_CLASSES[] EXPORT =
{
  CNetDesc,
  CProxyDesc,
  CCurlDesc,
  CHttpClientDesc,
  CFtpClientDesc,
  NULL
};



int EXPORT GB_INIT(void)
{
  curl_global_init(0);
  return 0;
}



void EXPORT GB_EXIT()
{
  curl_global_cleanup();
}


#ifdef _cpluscplus
}
#endif

