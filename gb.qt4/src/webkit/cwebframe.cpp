/***************************************************************************

  cwebframe.cpp

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

#define __CWEBFRAME_CPP

#include <QUrl>
#include <QWebElement>
#include <QWebPage>
#include <QWebFrame>
#include <QDateTime>

#include "cwebelement.h"
#include "cwebframe.h"
#include "../cprinter.h"

CWEBFRAME *CWEBFRAME_get(QWebFrame *frame)
{
	void *_object;
	
	if (!frame) return 0;
	
	_object = QT.GetLink(frame);
	
	if (!_object)
	{
		_object = GB.New(GB.FindClass("WebFrame"), 0, 0);
		//qDebug("create WebFrame %p", _object);
		QT.Link(frame, _object);
		THIS->frame = frame;
	}
	
	return (CWEBFRAME *)_object;
}

void CWEBFRAME_eval(QWebFrame *frame, const QString &javascript)
{
	QVariant result = frame->evaluateJavaScript(javascript);
	MAIN_return_qvariant(result);
}

BEGIN_METHOD_VOID(WebFrame_free)

	//qDebug("WebFrame_free: %p", THIS);

END_METHOD

BEGIN_PROPERTY(WebFrame_Name)

	GB.ReturnNewZeroString(TO_UTF8(FRAME->frameName()));

END_PROPERTY

BEGIN_PROPERTY(WebFrame_Parent)

	GB.ReturnObject(CWEBFRAME_get(FRAME->parentFrame()));

END_PROPERTY

BEGIN_PROPERTY(WebFrame_Url)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(FRAME->url().toString()));
	else
		FRAME->setUrl(QUrl(QSTRING_PROP()));

END_PROPERTY


BEGIN_PROPERTY(WebFrameChildren_Count)

	GB.ReturnInteger(FRAME->childFrames().count());

END_PROPERTY

BEGIN_METHOD(WebFrameChildren_get, GB_INTEGER index)

	int index = VARG(index);
	QList<QWebFrame *> children = FRAME->childFrames();
	
	if (index < 0 || index >= children.count())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	GB.ReturnObject(CWEBFRAME_get(children.at(index)));

END_METHOD

BEGIN_METHOD(WebFrame_Print, GB_OBJECT printer)

	CPRINTER *printer = (CPRINTER *)VARG(printer);
	
	if (GB.CheckObject(printer))
		return;
	
	FRAME->print(printer->printer);

END_METHOD

BEGIN_PROPERTY(WebFrame_HTML)

	GB.ReturnNewZeroString(TO_UTF8(FRAME->toHtml()));

END_PROPERTY

BEGIN_PROPERTY(WebFrame_Text)

	GB.ReturnNewZeroString(TO_UTF8(FRAME->toPlainText()));

END_PROPERTY

BEGIN_METHOD(WebFrame_EvalJavaScript, GB_STRING javascript)

	CWEBFRAME_eval(FRAME, QSTRING_ARG(javascript));

END_METHOD

BEGIN_PROPERTY(WebFrame_Document)

	GB.ReturnObject(CWEBELEMENT_create(FRAME->documentElement()));

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC WebFrameChildrenDesc[] =
{
  GB_DECLARE(".WebFrame.Children", sizeof(CWEBFRAME)), GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Count", "i", WebFrameChildren_Count),
	GB_METHOD("_get", "WebFrame", WebFrameChildren_get, "(Index)i"),
	
	GB_END_DECLARE
};

GB_DESC WebFrameDesc[] =
{
  GB_DECLARE("WebFrame", sizeof(CWEBFRAME)),
	
	GB_METHOD("_free", NULL, WebFrame_free, NULL),
	
	GB_PROPERTY_READ("Name", "s", WebFrame_Name),
	GB_PROPERTY_SELF("Children", ".WebFrame.Children"),
	GB_PROPERTY_READ("Parent", "WebFrame", WebFrame_Parent),
	GB_PROPERTY("Url", "s", WebFrame_Url),
	GB_METHOD("Print", NULL, WebFrame_Print, "(Printer)Printer;"),
	GB_PROPERTY_READ("HTML", "s", WebFrame_HTML),
	GB_PROPERTY_READ("Text", "s", WebFrame_Text),
	GB_METHOD("Eval", "v", WebFrame_EvalJavaScript, "(JavaScript)s"),
	GB_PROPERTY_READ("Document", "WebElement", WebFrame_Document),
	
	GB_END_DECLARE
};



