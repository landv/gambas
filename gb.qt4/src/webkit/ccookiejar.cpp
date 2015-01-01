/***************************************************************************

  ccookiejar.cpp

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

#define __CCOOKIEJAR_CPP

#include "ccookiejar.h"

#include <QDateTime>

BEGIN_PROPERTY(Cookie_Domain)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(COOKIE->domain()));
	else
		COOKIE->setDomain(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(Cookie_ExpirationDate)

	GB_DATE date;
	GB_DATE_SERIAL ds;
	QDateTime d;
	
	if (READ_PROPERTY)
	{
		if (COOKIE->isSessionCookie())
		{
			GB.ReturnDate(NULL);
			return;
		}
		
		d = COOKIE->expirationDate();
		
		ds.year = d.date().year();
		ds.month = d.date().month();
		ds.day = d.date().day();
		ds.hour = d.time().hour();
		ds.min = d.time().minute();
		ds.sec = d.time().second();
		ds.msec = d.time().msec();
		
		GB.MakeDate(&ds, &date);
		GB.ReturnDate(&date);
	}
	else
	{
		ds = *GB.SplitDate(PROP(GB_DATE));
		d = QDateTime(QDate(ds.year, ds.month, ds.day), QTime(ds.hour, ds.min, ds.sec, ds.msec));
		COOKIE->setExpirationDate(d);
	}

END_PROPERTY

BEGIN_PROPERTY(Cookie_HttpOnly)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COOKIE->isHttpOnly());
	else
		COOKIE->setHttpOnly(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Cookie_Secure)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COOKIE->isSecure());
	else
		COOKIE->setSecure(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Cookie_Session)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COOKIE->isSessionCookie());
	else
	{
		QDateTime d;
		COOKIE->setExpirationDate(d);
	}

END_PROPERTY

BEGIN_PROPERTY(Cookie_Name)

	if (READ_PROPERTY)
	{
		QByteArray ba = COOKIE->name();
		GB.ReturnNewString((const char *)ba, ba.size());
	}
	else
	{
		COOKIE->setName(QByteArray(PSTRING(), PLENGTH()));
	}

END_PROPERTY

BEGIN_PROPERTY(Cookie_Path)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(COOKIE->path()));
	else
		COOKIE->setPath(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(Cookie_Value)

	if (READ_PROPERTY)
	{
		QByteArray ba = COOKIE->value();
		GB.ReturnNewString((const char *)ba, ba.size());
	}
	else
	{
		COOKIE->setValue(QByteArray(PSTRING(), PLENGTH()));
	}

END_PROPERTY

BEGIN_METHOD_VOID(Cookie_new)

	COOKIE = new QNetworkCookie;

END_METHOD

BEGIN_METHOD_VOID(Cookie_free)

	delete COOKIE;

END_METHOD

GB_DESC CookieDesc[] =
{
  GB_DECLARE("Cookie", sizeof(CCOOKIE)),
  
  GB_METHOD("_new", NULL, Cookie_new, NULL),
  GB_METHOD("_free", NULL, Cookie_free, NULL),
  
	GB_PROPERTY("Domain", "s", Cookie_Domain),
	GB_PROPERTY("ExpirationDate", "d", Cookie_ExpirationDate),
	GB_PROPERTY("HttpOnly", "b", Cookie_HttpOnly),
	GB_PROPERTY("Secure", "b", Cookie_Secure),
	GB_PROPERTY("Session", "b", Cookie_Session),
	GB_PROPERTY("Name", "s", Cookie_Name),
	GB_PROPERTY("Path", "s", Cookie_Path),
	GB_PROPERTY("Value", "s", Cookie_Value),

	GB_END_DECLARE
};

/***************************************************************************/

CCOOKIE *WEB_create_cookie(const QNetworkCookie &cookie)
{
	CCOOKIE *_object = (CCOOKIE *)GB.New(GB.FindClass("Cookie"), NULL, NULL);
	*(THIS_COOKIE->cookie) = cookie;
	
	return THIS_COOKIE;
}

/***************************************************************************/

MyCookieJar::MyCookieJar(QObject *parent) : QNetworkCookieJar(parent)
{
}

