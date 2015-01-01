/***************************************************************************

  cwebhittest.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CWEBHITTEST_H
#define __CWEBHITTEST_H

#include "main.h"

#include <QUrl>
#include <QWebHitTestResult>
#include <QWebElement>

#ifndef __CWEBHITTEST_CPP

extern GB_DESC WebHitTestDesc[];

#else

#define THIS ((CWEBHITTEST *)_object)
#define RESULT (THIS->result)

#endif

typedef
	struct 
	{
		GB_BASE ob;
		QWebHitTestResult *result;
	}
	CWEBHITTEST;
	
CWEBHITTEST *WEB_create_hit_test(const QWebHitTestResult &result);

#endif
