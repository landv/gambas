/***************************************************************************

  cwebhittest.cpp

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CWEBHITTEST_CPP

#include "cwebhittest.h"

BEGIN_PROPERTY(WebHitTest_Document)

	GB.ReturnBoolean(RESULT->linkUrl().isEmpty() && RESULT->imageUrl().isEmpty());

END_PROPERTY

BEGIN_PROPERTY(WebHitTest_Link)

	GB.ReturnBoolean(!RESULT->linkUrl().isEmpty());

END_PROPERTY

BEGIN_PROPERTY(WebHitTest_Image)

	GB.ReturnBoolean(!RESULT->imageUrl().isEmpty());

END_PROPERTY

BEGIN_PROPERTY(WebHitTest_Selected)

	GB.ReturnBoolean(RESULT->isContentSelected());

END_PROPERTY

BEGIN_PROPERTY(WebHitTest_Editable)

	GB.ReturnBoolean(RESULT->isContentEditable());

END_PROPERTY

BEGIN_PROPERTY(WebHitTest_Url)

	QUrl url;
	
	url = RESULT->linkUrl();
	if (url.isEmpty())
		url = RESULT->imageUrl();

	GB.ReturnNewZeroString(TO_UTF8(url.toString()));

END_PROPERTY

BEGIN_METHOD_VOID(WebHitTest_new)

	RESULT = new QWebHitTestResult;

END_METHOD

BEGIN_METHOD_VOID(WebHitTest_free)

	delete RESULT;

END_METHOD

GB_DESC CWebHitTestDesc[] =
{
  GB_DECLARE("WebHitTest", sizeof(CWEBHITTEST)),
  
  GB_METHOD("_new", NULL, WebHitTest_new, NULL),
  GB_METHOD("_free", NULL, WebHitTest_free, NULL),
  
	GB_PROPERTY_READ("Document", "b", WebHitTest_Document),
	GB_PROPERTY_READ("Link", "b", WebHitTest_Link),
	GB_PROPERTY_READ("Image", "b", WebHitTest_Image),
	//GB_PROPERTY_READ("Media", "b", WebHitTest_Media),
	GB_PROPERTY_READ("Selected", "b", WebHitTest_Selected),
	GB_PROPERTY_READ("Editable", "s", WebHitTest_Editable),
	GB_PROPERTY_READ("Url", "s", WebHitTest_Url),

	GB_END_DECLARE
};

/***************************************************************************/

CWEBHITTEST *WEB_create_hit_test(const QWebHitTestResult &result)
{
	CWEBHITTEST *_object;
	
  GB.New(POINTER(&_object), GB.FindClass("WebHitTest"), NULL, NULL);
	*(THIS->result) = result;
	
	return THIS;
}
