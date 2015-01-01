/***************************************************************************

  ccookiejar.h

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

#ifndef __CCOOKIEJAR_H
#define __CCOOKIEJAR_H

#include "main.h"

#include <QUrl>
#include <QNetworkCookie>
#include <QNetworkCookieJar>

#ifndef __CCOOKIEJAR_CPP

extern GB_DESC CookieDesc[];
//extern GB_DESC CCookieJarDesc[];

#else

#define THIS_COOKIE ((CCOOKIE *)_object)
#define COOKIE (THIS_COOKIE->cookie)

#endif

class MyCookieJar : public QNetworkCookieJar
{
	Q_OBJECT

public:
	
	MyCookieJar(QObject *parent = 0);

	QList<QNetworkCookie> allCookies () const { return QNetworkCookieJar::allCookies(); }
	void setAllCookies(const QList<QNetworkCookie> &cookieList) { QNetworkCookieJar::setAllCookies(cookieList); }
	
	//virtual QList<QNetworkCookie> cookiesForUrl(const QUrl & url) const;
	//virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);
};

typedef
	struct 
	{
		GB_BASE ob;
		MyCookieJar *jar;
	}
	CCOOKIEJAR;
	
typedef
	struct {
		GB_BASE ob;
		QNetworkCookie *cookie;
	}
	CCOOKIE;

CCOOKIE *WEB_create_cookie(const QNetworkCookie &cookie);
	
#endif
