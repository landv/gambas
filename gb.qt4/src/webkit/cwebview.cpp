/***************************************************************************

  cwebview.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CWEBVIEW_CPP

#include <QUrl>
#include <QWebPage>
#include <QWebFrame>

#include "cwebview.h"

DECLARE_EVENT(EVENT_CLICK);
DECLARE_EVENT(EVENT_LINK);
DECLARE_EVENT(EVENT_LOAD);
DECLARE_EVENT(EVENT_ERROR);
DECLARE_EVENT(EVENT_ICON);
DECLARE_EVENT(EVENT_TITLE);
DECLARE_EVENT(EVENT_SELECT);
DECLARE_EVENT(EVENT_STATUS);
DECLARE_EVENT(EVENT_NEW_WINDOW);

BEGIN_METHOD(WebView_new, GB_OBJECT parent)

  MyWebView *wid = new MyWebView(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);

  QObject::connect(wid, SIGNAL(iconChanged()), &CWebView::manager, SLOT(iconChanged()));
  QObject::connect(wid, SIGNAL(linkClicked(const QUrl &)), &CWebView::manager, SLOT(linkClicked(const QUrl &)));
  QObject::connect(wid, SIGNAL(loadFinished(bool)), &CWebView::manager, SLOT(loadFinished(bool)));
  QObject::connect(wid, SIGNAL(loadProgress(int)), &CWebView::manager, SLOT(loadProgress(int)));
  QObject::connect(wid, SIGNAL(loadStarted()), &CWebView::manager, SLOT(loadStarted()));
  QObject::connect(wid, SIGNAL(selectionChanged()), &CWebView::manager, SLOT(selectionChanged()));
  QObject::connect(wid, SIGNAL(statusBarMessage(const QString &)), &CWebView::manager, SLOT(statusBarMessage(const QString &)));
  QObject::connect(wid, SIGNAL(titleChanged(const QString &)), &CWebView::manager, SLOT(titleChanged(const QString &)));
  
	QObject::connect(wid->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)), &CWebView::manager, 
										SLOT(linkHovered(const QString &, const QString &, const QString &)));

END_METHOD

BEGIN_PROPERTY(WebView_Url)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->url().toString()));
	else
		WIDGET->setUrl(QUrl(QSTRING_PROP()));

END_PROPERTY

BEGIN_PROPERTY(WebView_HTML)

	GB.ReturnNewZeroString(TO_UTF8(WIDGET->page()->mainFrame()->toHtml()));

END_PROPERTY

BEGIN_PROPERTY(WebView_Text)

	GB.ReturnNewZeroString(TO_UTF8(WIDGET->page()->mainFrame()->toPlainText()));

END_PROPERTY

BEGIN_PROPERTY(WebView_Icon)

	QIcon icon = WIDGET->icon();
	if (icon.isNull())
		GB.ReturnNull();
	else
		GB.ReturnObject(QT.CreatePicture(icon.pixmap(32, 32)));

END_PROPERTY

BEGIN_PROPERTY(WebView_SelectedText)

	GB.ReturnNewZeroString(TO_UTF8(WIDGET->selectedText()));

END_PROPERTY

BEGIN_PROPERTY(WebView_Zoom)

	if (READ_PROPERTY)
		GB.ReturnFloat(WIDGET->zoomFactor());
	else
		WIDGET->setZoomFactor(VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(WebView_TextZoom)

	if (READ_PROPERTY)
		GB.ReturnFloat(WIDGET->textSizeMultiplier());
	else
		WIDGET->setTextSizeMultiplier(VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(WebView_Title)

	GB.ReturnNewZeroString(TO_UTF8(WIDGET->title()));

END_PROPERTY

BEGIN_METHOD_VOID(WebView_Back)

	WIDGET->back();

END_METHOD

BEGIN_METHOD_VOID(WebView_Forward)

	WIDGET->forward();

END_METHOD

BEGIN_METHOD_VOID(WebView_Reload)

	WIDGET->reload();

END_METHOD

BEGIN_METHOD_VOID(WebView_Stop)

	WIDGET->stop();

END_METHOD

BEGIN_PROPERTY(WebView_NewView)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->new_view);
	else
		GB.StoreObject(PROP(GB_OBJECT), &THIS->new_view);

END_PROPERTY

GB_DESC CWebViewDesc[] =
{
  GB_DECLARE("WebView", sizeof(CWEBVIEW)), GB_INHERITS("Control"),
	
  GB_METHOD("_new", NULL, WebView_new, "(Parent)Container;"),
	
	GB_PROPERTY("Url", "s", WebView_Url),
	GB_PROPERTY_READ("HTML", "s", WebView_HTML),
	GB_PROPERTY_READ("Text", "s", WebView_Text),
	GB_PROPERTY_READ("Icon", "Picture", WebView_Icon),
	GB_PROPERTY_READ("SelectedText", "s", WebView_SelectedText),
	GB_PROPERTY("Zoom", "f", WebView_Zoom),
	GB_PROPERTY("TextZoom", "f", WebView_TextZoom),
	GB_PROPERTY_READ("Title", "s", WebView_Title),
	
	GB_METHOD("Back", NULL, WebView_Back, NULL),
	GB_METHOD("Forward", NULL, WebView_Forward, NULL),
	GB_METHOD("Reload", NULL, WebView_Reload, NULL),
	GB_METHOD("Stop", NULL, WebView_Stop, NULL),
	
	GB_PROPERTY("NewView", "WebView", WebView_NewView),

	GB_CONSTANT("_Properties", "s", "*"),
	
	GB_EVENT("Click", NULL, NULL, &EVENT_CLICK),
	GB_EVENT("Link", NULL, "(Url)s", &EVENT_LINK),
	GB_EVENT("Load", NULL, "(Progress)f", &EVENT_LOAD),
	GB_EVENT("Error", NULL, NULL, &EVENT_ERROR),
	GB_EVENT("Icon", NULL, NULL, &EVENT_ICON),
	GB_EVENT("Title", NULL, NULL, &EVENT_TITLE),
	GB_EVENT("Select", NULL, NULL, &EVENT_SELECT),
	GB_EVENT("Status", NULL, "(Status)s", &EVENT_STATUS),
	GB_EVENT("NewWindow", NULL, "(Modal)b", &EVENT_NEW_WINDOW),

	GB_END_DECLARE
};

/***************************************************************************/

MyWebView::MyWebView(QWidget *parent) : QWebView(parent)
{
}

QWebView *MyWebView::createWindow(QWebPage::WebWindowType type)
{
	void *_object = QT.GetObject(this);
	QWebView *new_view;
	
	GB.Raise(THIS, EVENT_NEW_WINDOW, 1, GB_T_BOOLEAN, type == QWebPage::WebModalDialog);
	
	if (!THIS->new_view)
		return 0;
	
	new_view = (QWebView *)(((CWEBVIEW *)THIS->new_view)->widget.widget);
	GB.Unref(POINTER(&THIS->new_view));
	THIS->new_view = 0;
	return new_view;
}

/***************************************************************************/

CWebView CWebView::manager;

void CWebView::iconChanged()
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_ICON, 0);
}

void CWebView::linkClicked(const QUrl &url)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_CLICK, 0);
}

void CWebView::loadFinished(bool ok)
{
	GET_SENDER();
	if (ok)
		GB.Raise(THIS, EVENT_LOAD, 1, GB_T_FLOAT, 1.0);
	else
		GB.Raise(THIS, EVENT_ERROR, 0);
}

void CWebView::loadProgress(int progress)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_LOAD, 1, GB_T_FLOAT, progress / 100.0);
}

void CWebView::loadStarted()
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_LOAD, 1, GB_T_FLOAT, 0.0);
}

void CWebView::selectionChanged()
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_SELECT, 0);
}

void CWebView::statusBarMessage(const QString &text)
{
	GET_SENDER();
	const char *str = TO_UTF8(text);
	GB.Raise(THIS, EVENT_STATUS, 1, GB_T_STRING, str, strlen(str));
}
	
void CWebView::titleChanged(const QString &title)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_TITLE, 0);
}

void CWebView::linkHovered(const QString &link, const QString &title, const QString &textContent)
{
	GET_SENDER();
	const char *str = TO_UTF8(link);
	GB.Raise(THIS, EVENT_LINK, 1, GB_T_STRING, str, strlen(str));
}
