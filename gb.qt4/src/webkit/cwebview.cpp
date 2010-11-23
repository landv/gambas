/***************************************************************************

  cwebview.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CWEBVIEW_CPP

#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QWebPage>
#include <QWebFrame>

#include "ccookiejar.h"
#include "cwebsettings.h"
#include "cwebframe.h"
#include "cwebhittest.h"
#include "cwebview.h"

DECLARE_EVENT(EVENT_CLICK);
DECLARE_EVENT(EVENT_LINK);
DECLARE_EVENT(EVENT_PROGRESS);
DECLARE_EVENT(EVENT_LOAD);
DECLARE_EVENT(EVENT_ERROR);
DECLARE_EVENT(EVENT_ICON);
DECLARE_EVENT(EVENT_TITLE);
DECLARE_EVENT(EVENT_SELECT);
DECLARE_EVENT(EVENT_STATUS);
DECLARE_EVENT(EVENT_NEW_WINDOW);
DECLARE_EVENT(EVENT_NEW_FRAME);
DECLARE_EVENT(EVENT_AUTH);
DECLARE_EVENT(EVENT_DOWNLOAD);

static QNetworkAccessManager *_network_access_manager = 0;

BEGIN_METHOD(WebView_new, GB_OBJECT parent)

  MyWebView *wid = new MyWebView(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object, false);
	
	if (!_network_access_manager)
	{
		_network_access_manager = new QNetworkAccessManager();
		_network_access_manager->setCookieJar(new MyCookieJar);
	}
	
	wid->page()->setNetworkAccessManager(_network_access_manager);
	wid->page()->setForwardUnsupportedContent(true);

  //QObject::connect(wid, SIGNAL(linkClicked(const QUrl &)), &CWebView::manager, SLOT(linkClicked(const QUrl &)));
  QObject::connect(wid, SIGNAL(loadFinished(bool)), &CWebView::manager, SLOT(loadFinished(bool)));
  QObject::connect(wid, SIGNAL(loadProgress(int)), &CWebView::manager, SLOT(loadProgress(int)));
  QObject::connect(wid, SIGNAL(loadStarted()), &CWebView::manager, SLOT(loadStarted()));
  QObject::connect(wid, SIGNAL(selectionChanged()), &CWebView::manager, SLOT(selectionChanged()));
  QObject::connect(wid, SIGNAL(statusBarMessage(const QString &)), &CWebView::manager, SLOT(statusBarMessage(const QString &)));
  QObject::connect(wid, SIGNAL(titleChanged(const QString &)), &CWebView::manager, SLOT(titleChanged(const QString &)));
  
	QObject::connect(wid->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)), &CWebView::manager, 
										SLOT(linkHovered(const QString &, const QString &, const QString &)));
	QObject::connect(wid->page(), SIGNAL(frameCreated(QWebFrame *)), &CWebView::manager, SLOT(frameCreated(QWebFrame *)));
	QObject::connect(wid->page()->networkAccessManager(), SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), &CWebView::manager,
										SLOT(authenticationRequired(QNetworkReply *, QAuthenticator *)));
	QObject::connect(wid->page(), SIGNAL(downloadRequested(QNetworkRequest)), &CWebView::manager, SLOT(downloadRequested(QNetworkRequest)));
	
  QObject::connect(wid->page()->mainFrame(), SIGNAL(iconChanged()), &CWebView::manager, SLOT(iconChanged()));
	QObject::connect(wid->page()->mainFrame(), SIGNAL(urlChanged(const QUrl &)), &CWebView::manager, SLOT(urlChanged(const QUrl &)));
  QObject::connect(wid->page(), SIGNAL(unsupportedContent(QNetworkReply*)), &CWebView::manager, SLOT(handleUnsupportedContent(QNetworkReply*)));

END_METHOD

BEGIN_METHOD_VOID(WebView_free)

	GB.FreeString(&THIS->status);
	GB.Unref(POINTER(&THIS->icon));

END_METHOD

BEGIN_METHOD_VOID(WebView_exit)

	delete _network_access_manager;
	
END_METHOD

BEGIN_PROPERTY(WebView_Url)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->url().toString()));
	else
		WIDGET->setUrl(QUrl(QSTRING_PROP()));

END_PROPERTY

BEGIN_PROPERTY(WebView_HTML)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->page()->mainFrame()->toHtml()));
	else
		WIDGET->setHtml(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(WebView_Text)

	GB.ReturnNewZeroString(TO_UTF8(WIDGET->page()->mainFrame()->toPlainText()));

END_PROPERTY

BEGIN_PROPERTY(WebView_Icon)

	if (!THIS->icon)
	{
		QIcon icon = WIDGET->icon();
		THIS->icon = QT.CreatePicture(icon.pixmap(16, 16));
		GB.Ref(THIS->icon);
	}

	GB.ReturnObject(THIS->icon);

END_PROPERTY

BEGIN_PROPERTY(WebView_SelectedText)

	GB.ReturnNewZeroString(TO_UTF8(WIDGET->selectedText()));

END_PROPERTY

#if QT_VERSION >= QT_VERSION_CHECK(4, 5, 0)
BEGIN_PROPERTY(WebView_Zoom)

	if (READ_PROPERTY)
		GB.ReturnFloat(WIDGET->zoomFactor());
	else
		WIDGET->setZoomFactor(VPROP(GB_FLOAT));

END_PROPERTY
#endif

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

	WIDGET->stop();
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

BEGIN_PROPERTY(WebView_Progress)

	GB.ReturnFloat(THIS->progress);

END_PROPERTY

BEGIN_PROPERTY(WebView_Status)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->status);
	else
		GB.StoreString(PROP(GB_STRING), &THIS->status);

END_PROPERTY

BEGIN_PROPERTY(WebView_Frame)

	GB.ReturnObject(CWEBFRAME_get(WIDGET->page()->mainFrame()));

END_PROPERTY

BEGIN_PROPERTY(WebView_Current)

	GB.ReturnObject(CWEBFRAME_get(WIDGET->page()->currentFrame()));

END_PROPERTY


BEGIN_PROPERTY(WebViewAuth_Url)

	if (THIS->reply)
		GB.ReturnNewZeroString(TO_UTF8(THIS->reply->url().toString()));
	else
		GB.ReturnNull();

END_PROPERTY

BEGIN_PROPERTY(WebViewAuth_Realm)

	if (THIS->authenticator)
		GB.ReturnNewZeroString(TO_UTF8(THIS->authenticator->realm()));
	else
		GB.ReturnNull();

END_PROPERTY

BEGIN_PROPERTY(WebViewAuth_User)

	if (READ_PROPERTY)
	{
		if (THIS->authenticator)
			GB.ReturnNewZeroString(TO_UTF8(THIS->authenticator->user()));
		else
			GB.ReturnNull();
	}
	else
	{
		if (THIS->authenticator)
			THIS->authenticator->setUser(QSTRING_PROP());
		else
			GB.Error("No authentication required");
	}

END_PROPERTY

BEGIN_PROPERTY(WebViewAuth_Password)

	if (READ_PROPERTY)
	{
		if (THIS->authenticator)
			GB.ReturnNewZeroString(TO_UTF8(THIS->authenticator->password()));
		else
			GB.ReturnNull();
	}
	else
	{
		if (THIS->authenticator)
			THIS->authenticator->setPassword(QSTRING_PROP());
		else
			GB.Error("No authentication required");
	}

END_PROPERTY

BEGIN_PROPERTY(WebView_Cached)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->cached);
	else
	{
		bool val = VPROP(GB_BOOLEAN);
		
		if (val == THIS->cached)
			return;
		
		THIS->cached = val;
		WEBSETTINGS_set_cache(WIDGET, val);
	}

END_PROPERTY

BEGIN_PROPERTY(WebView_Cookies)

	MyCookieJar *cookieJar = static_cast<MyCookieJar *>(_network_access_manager->cookieJar());
	QList<QNetworkCookie> list;
	GB_ARRAY cookies;
	int i;
	
	if (READ_PROPERTY)
	{
		list = cookieJar->allCookies();
		
		GB.Array.New(POINTER(&cookies), GB.FindClass("Cookie"), list.count());
		
		for (i = 0; i < list.count(); i++)
		{
			CCOOKIE *cookie = WEB_create_cookie(list.at(i));
			*((CCOOKIE **)(GB.Array.Get(cookies, i))) = cookie;
			GB.Ref(cookie);
		}
		
		GB.ReturnObject(cookies);
	}
	else
	{
		// TODO
		cookies = VPROP(GB_OBJECT);
		if (GB.CheckObject(cookies))
			return;
		
		for (i = 0; i < GB.Array.Count(cookies); i++)
		{
			CCOOKIE *cookie = *((CCOOKIE **)GB.Array.Get(cookies, i));
			if (GB.CheckObject(cookie))
				continue;
			list.append(*(cookie->cookie));
		}
		
		cookieJar->setAllCookies(list);
	}

END_PROPERTY

BEGIN_METHOD(WebView_HitTest, GB_INTEGER X; GB_INTEGER Y)

	GB.ReturnObject(WEB_create_hit_test(WIDGET->page()->mainFrame()->hitTestContent(QPoint(VARG(X), VARG(Y)))));

END_METHOD

BEGIN_METHOD(WebView_FindText, GB_STRING text; GB_BOOLEAN backward; GB_BOOLEAN case_sensitive; GB_BOOLEAN wrap)

	QString text;
	QWebPage::FindFlags flags = 0;
	
	if (!MISSING(text)) 
		text = QSTRING_ARG(text);
	
	if (VARGOPT(backward, false)) flags |= QWebPage::FindBackward;
	if (VARGOPT(case_sensitive, false)) flags |= QWebPage::FindCaseSensitively;
	if (VARGOPT(wrap, false)) flags |= QWebPage::FindWrapsAroundDocument;
	//if (VARGOPT(highlight, false)) flags |= QWebPage::HighlightAllOccurrences;
	
	GB.ReturnBoolean(!WIDGET->findText(text, flags));

END_METHOD

GB_DESC CWebViewAuthDesc[] =
{
  GB_DECLARE(".WebView.Auth", sizeof(CWEBVIEW)), GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Url", "s", WebViewAuth_Url),
	GB_PROPERTY_READ("Realm", "s", WebViewAuth_Realm),
	GB_PROPERTY("User", "s", WebViewAuth_User),
	GB_PROPERTY("Password", "s", WebViewAuth_Password),
	
	GB_END_DECLARE
};

GB_DESC CWebViewDesc[] =
{
  GB_DECLARE("WebView", sizeof(CWEBVIEW)), GB_INHERITS("Control"),
	
  GB_METHOD("_new", NULL, WebView_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, WebView_free, NULL),
  GB_METHOD("_exit", NULL, WebView_exit, NULL),
	
	GB_PROPERTY("Url", "s", WebView_Url),
	GB_PROPERTY("Status", "s", WebView_Status),

	GB_PROPERTY("HTML", "s", WebView_HTML),
	GB_PROPERTY_READ("Text", "s", WebView_Text),
	GB_PROPERTY_READ("Icon", "Picture", WebView_Icon),
	GB_PROPERTY_READ("SelectedText", "s", WebView_SelectedText),
	GB_PROPERTY_READ("Progress", "f", WebView_Progress),
	#if QT_VERSION >= QT_VERSION_CHECK(4, 5, 0)
	GB_PROPERTY("Zoom", "f", WebView_Zoom),
	#else
	GB_PROPERTY("Zoom", "f", WebView_TextZoom),
	#endif
	GB_PROPERTY("TextZoom", "f", WebView_TextZoom),
	GB_PROPERTY_READ("Title", "s", WebView_Title),
	
	GB_PROPERTY_READ("Frame", "WebFrame", WebView_Frame),
	GB_PROPERTY_READ("Current", "WebFrame", WebView_Current),
	
	GB_PROPERTY_SELF("Settings", ".WebView.Settings"),
	GB_PROPERTY_SELF("Auth", ".WebView.Auth"),

	GB_METHOD("Back", NULL, WebView_Back, NULL),
	GB_METHOD("Forward", NULL, WebView_Forward, NULL),
	GB_METHOD("Reload", NULL, WebView_Reload, NULL),
	GB_METHOD("Stop", NULL, WebView_Stop, NULL),
	
	GB_PROPERTY("NewView", "WebView", WebView_NewView),

	GB_PROPERTY("Cached", "b", WebView_Cached),
	
	GB_PROPERTY("Cookies", "Cookie[]", WebView_Cookies),
	
	GB_METHOD("HitTest", "WebHitTest", WebView_HitTest, "(X)i(Y)i"),
	GB_METHOD("FindText", "b", WebView_FindText, "[(Text)s(Backward)b(CaseSensitive)b(Wrap)b]"),

	GB_CONSTANT("_Properties", "s", "*,Url,Cached"),
	
	GB_EVENT("Click", NULL, "(Frame)WebFrame", &EVENT_CLICK),
	GB_EVENT("Link", NULL, "(Url)s", &EVENT_LINK),
	GB_EVENT("Progress", NULL, NULL, &EVENT_PROGRESS),
	GB_EVENT("Load", NULL, NULL, &EVENT_LOAD),
	GB_EVENT("Error", NULL, NULL, &EVENT_ERROR),
	GB_EVENT("Icon", NULL, NULL, &EVENT_ICON),
	GB_EVENT("Title", NULL, NULL, &EVENT_TITLE),
	GB_EVENT("Select", NULL, NULL, &EVENT_SELECT),
	GB_EVENT("Status", NULL, NULL, &EVENT_STATUS),
	GB_EVENT("NewWindow", NULL, "(Modal)b", &EVENT_NEW_WINDOW),
	GB_EVENT("NewFrame", NULL, "(Frame)WebFrame", &EVENT_NEW_FRAME),
	GB_EVENT("Auth", NULL, NULL, &EVENT_AUTH),
	GB_EVENT("Download", NULL, "(Download)WebDownload", &EVENT_DOWNLOAD),

	GB_END_DECLARE
};

/***************************************************************************/

MyWebView::MyWebView(QWidget *parent) : QWebView(parent)
{
	//settings()->setFontFamily(QWebSettings::FixedFont, "monospace");
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

// void CWebView::linkClicked(const QUrl &url)
// {
// 	GET_SENDER();
// 	WIDGET->page()->currentFrame()->setUrl(url);
// 	//WIDGET->setUrl(url);
// 	GB.Raise(THIS, EVENT_CLICK, 0);
// }

void CWebView::loadFinished(bool ok)
{
	GET_SENDER();
	THIS->progress = 1;
	if (ok)
		GB.Raise(THIS, EVENT_LOAD, 0);
	else
		GB.Raise(THIS, EVENT_ERROR, 0);
}

void CWebView::loadProgress(int progress)
{
	GET_SENDER();
	THIS->progress = progress / 100.0;
	GB.Raise(THIS, EVENT_PROGRESS, 0);
}

void CWebView::loadStarted()
{
	GET_SENDER();
	THIS->progress = 0;
	GB.Raise(THIS, EVENT_PROGRESS, 0);
}

void CWebView::selectionChanged()
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_SELECT, 0);
}

void CWebView::statusBarMessage(const QString &text)
{
	GET_SENDER();
	GB.FreeString(&THIS->status);
	THIS->status = GB.NewZeroString(TO_UTF8(text));
	GB.Raise(THIS, EVENT_STATUS, 0);
}
	
void CWebView::titleChanged(const QString &title)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_TITLE, 0);
}

void CWebView::linkHovered(const QString &link, const QString &title, const QString &textContent)
{
	void *_object = QT.GetObject(((QWebPage*)sender())->view());
	const char *str = TO_UTF8(link);
	GB.Raise(THIS, EVENT_LINK, 1, GB_T_STRING, str, strlen(str));
}

void CWebView::frameCreated(QWebFrame *frame)
{
	QObject::connect(frame, SIGNAL(urlChanged(const QUrl &)), &CWebView::manager, SLOT(urlChanged(const QUrl &)));
	void *_object = QT.GetObject(((QWebPage*)sender())->view());
	GB.Raise(THIS, EVENT_NEW_FRAME, 1, GB_T_OBJECT, CWEBFRAME_get(frame));
}

void CWebView::authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
	void *_object = QT.GetObject((QWidget *)((QNetworkAccessManager*)sender())->parent());
	if (!THIS)
		return;
	
	THIS->reply = reply;
	THIS->authenticator = authenticator;
	
	GB.Raise(THIS, EVENT_AUTH, 0);
	
	THIS->reply = 0;
	THIS->authenticator = 0;
}

void CWebView::iconChanged()
{
	void *_object = QT.GetObject(((QWebFrame *)sender())->page()->view());
	GB.Unref(POINTER(&THIS->icon));
	THIS->icon = NULL;
	GB.Raise(THIS, EVENT_ICON, 0);
}

void CWebView::urlChanged(const QUrl &)
{
	QWebFrame *frame = (QWebFrame *)sender();
	void *_object = QT.GetObject(frame->page()->view());
	GB.Raise(THIS, EVENT_CLICK, 1, GB_T_OBJECT, CWEBFRAME_get(frame));
}

void CWebView::downloadRequested(const QNetworkRequest &request)
{
	void *_object = QT.GetObject(((QWebPage*)sender())->view());
	CWEBDOWNLOAD *download;
	
	download = WEB_create_download(_network_access_manager->get(request));
	
	if (GB.Raise(THIS, EVENT_DOWNLOAD, 1, GB_T_OBJECT, download) || !download->path || !*download->path)
		WEB_remove_download(download);
}

void CWebView::handleUnsupportedContent(QNetworkReply *reply)
{
	void *_object = QT.GetObject(((QWebPage*)sender())->view());
	CWEBDOWNLOAD *download;
	
	if (reply->error() == QNetworkReply::NoError) 
	{
		download = WEB_create_download(reply);
		
		if (GB.Raise(THIS, EVENT_DOWNLOAD, 1, GB_T_OBJECT, download) || !download->path || !*download->path)
			WEB_remove_download(download);
	}
}
