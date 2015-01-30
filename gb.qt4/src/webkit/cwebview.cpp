/***************************************************************************

  cwebview.cpp

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

#define __CWEBVIEW_CPP

#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QWebPage>
#include <QWebFrame>
#include <QWebHistory>

#include "ccookiejar.h"
#include "cwebsettings.h"
#include "cwebelement.h"
#include "cwebframe.h"
#include "cwebhittest.h"
#include "cwebview.h"

/*typedef
	struct {
		const char *name;
		QWebPage::WebAction action;
	}
	WEBVIEW_ACTION;*/
	
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

#define HISTORY (WIDGET->history())


static QNetworkAccessManager *_network_access_manager = 0;
static CWEBVIEW *_network_access_manager_view = 0;
static QT_COLOR_FUNC _old_after_set_color;

/*
static WEBVIEW_ACTION _actions[] = 
{
	{ "OpenLink", QWebPage::OpenLink },
	{ "OpenLinkInNewWindow", QWebPage::OpenLinkInNewWindow },
	{ "OpenFrameInNewWindow", QWebPage::OpenFrameInNewWindow },
	{ "DownloadLinkToDisk", QWebPage::DownloadLinkToDisk },
	{ "CopyLinkToClipboard", QWebPage::CopyLinkToClipboard },
	{ "OpenImageInNewWindow", QWebPage::OpenImageInNewWindow },
	{ "DownloadImageToDisk", QWebPage::DownloadImageToDisk },
	{ "CopyImageToClipboard", QWebPage::CopyImageToClipboard },
	{ "Back", QWebPage::Back },
	{ "Forward", QWebPage::Forward },
	{ "Stop", QWebPage::Stop },
	{ "StopScheduledPageRefresh", QWebPage::StopScheduledPageRefresh },
	{ "Reload", QWebPage::Reload },
	{ "ReloadAndBypassCache", QWebPage::ReloadAndBypassCache },
	{ "Cut", QWebPage::Cut },
	{ "Copy", QWebPage::Copy },
	{ "Paste", QWebPage::Paste },
	{ "Undo", QWebPage::Undo },
	{ "Redo", QWebPage::Redo },
	{ "MoveToNextChar", QWebPage::MoveToNextChar },
	{ "MoveToPreviousChar", QWebPage::MoveToPreviousChar },
	{ "MoveToNextWord", QWebPage::MoveToNextWord },
	{ "MoveToPreviousWord", QWebPage::MoveToPreviousWord },
	{ "MoveToNextLine", QWebPage::MoveToNextLine },
	{ "MoveToPreviousLine", QWebPage::MoveToPreviousLine },
	{ "MoveToStartOfLine", QWebPage::MoveToStartOfLine },
	{ "MoveToEndOfLine", QWebPage::MoveToEndOfLine },
	{ "MoveToStartOfBlock", QWebPage::MoveToStartOfBlock },
	{ "MoveToEndOfBlock", QWebPage::MoveToEndOfBlock },
	{ "MoveToStartOfDocument", QWebPage::MoveToStartOfDocument },
	{ "MoveToEndOfDocument", QWebPage::MoveToEndOfDocument },
	{ "SelectNextChar", QWebPage::SelectNextChar },
	{ "SelectPreviousChar", QWebPage::SelectPreviousChar },
	{ "SelectNextWord", QWebPage::SelectNextWord },
	{ "SelectPreviousWord", QWebPage::SelectPreviousWord },
	{ "SelectNextLine", QWebPage::SelectNextLine },
	{ "SelectPreviousLine", QWebPage::SelectPreviousLine },
	{ "SelectStartOfLine", QWebPage::SelectStartOfLine },
	{ "SelectEndOfLine", QWebPage::SelectEndOfLine },
	{ "SelectStartOfBlock", QWebPage::SelectStartOfBlock },
	{ "SelectEndOfBlock", QWebPage::SelectEndOfBlock },
	{ "SelectStartOfDocument", QWebPage::SelectStartOfDocument },
	{ "SelectEndOfDocument", QWebPage::SelectEndOfDocument },
	{ "DeleteStartOfWord", QWebPage::DeleteStartOfWord },
	{ "DeleteEndOfWord", QWebPage::DeleteEndOfWord },
	{ "SetTextDirectionDefault", QWebPage::SetTextDirectionDefault },
	{ "SetTextDirectionLeftToRight", QWebPage::SetTextDirectionLeftToRight },
	{ "SetTextDirectionRightToLeft", QWebPage::SetTextDirectionRightToLeft },
	{ "ToggleBold", QWebPage::ToggleBold },
	{ "ToggleItalic", QWebPage::ToggleItalic },
	{ "ToggleUnderline", QWebPage::ToggleUnderline },
	{ "InspectElement", QWebPage::InspectElement },
	{ "InsertParagraphSeparator", QWebPage::InsertParagraphSeparator },
	{ "InsertLineSeparator", QWebPage::InsertLineSeparator },
	{ "SelectAll", QWebPage::SelectAll },
	{ "PasteAndMatchStyle", QWebPage::PasteAndMatchStyle },
	{ "RemoveFormat", QWebPage::RemoveFormat },
	{ "ToggleStrikethrough", QWebPage::ToggleStrikethrough },
	{ "ToggleSubscript", QWebPage::ToggleSubscript },
	{ "ToggleSuperscript", QWebPage::ToggleSuperscript },
	{ "InsertUnorderedList", QWebPage::InsertUnorderedList },
	{ "InsertOrderedList", QWebPage::InsertOrderedList },
	{ "Indent", QWebPage::Indent },
	{ "Unindent", QWebPage::Outdent },
	{ "AlignCenter", QWebPage::AlignCenter },
	{ "AlignJustified", QWebPage::AlignJustified },
	{ "AlignLeft", QWebPage::AlignLeft },
	{ "AlignRight", QWebPage::AlignRight },
	{ NULL, QWebPage::NoWebAction }
};
*/

QNetworkAccessManager *WEBVIEW_get_network_manager()
{
	if (!_network_access_manager)
	{
		_network_access_manager = new QNetworkAccessManager();
		_network_access_manager->setCookieJar(new MyCookieJar);
	}
	
	return _network_access_manager;
}

/*
static QWebPage::WebAction get_action(const char *name)
{
	WEBVIEW_ACTION *p;
	
	for (p = _actions; p->name; p++)
	{
		if (!strcasecmp(p->name, name))
			return p->action;
	}
	
	return QWebPage::NoWebAction;
}
*/

static void after_set_color(void *_object)
{
	if (!GB.Is(THIS, CLASS_WebView))
	{
		if (_old_after_set_color)
			(*_old_after_set_color)(THIS);
		return;
	}

	if (QT.GetBackgroundColor(THIS) == GB_COLOR_DEFAULT)
	{
		QPalette palette = WIDGET->palette();
		WIDGET->page()->setPalette(palette);
		WIDGET->setAttribute(Qt::WA_OpaquePaintEvent, true);
	}
	else
	{
		qDebug("after_set_color");
		QPalette palette = WIDGET->palette();
		palette.setBrush(QPalette::Base, Qt::transparent);
		WIDGET->page()->setPalette(palette);
		WIDGET->setAttribute(Qt::WA_OpaquePaintEvent, false);
	}
}

static void stop_view(void *_object)
{
	//fprintf(stderr, "stop_view\n");
	THIS->stopping = TRUE;
	WIDGET->stop();
	THIS->stopping = FALSE;
}

//-------------------------------------------------------------------------

BEGIN_METHOD(WebView_new, GB_OBJECT parent)

  MyWebView *wid = new MyWebView(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object, false);
	
	WEBVIEW_get_network_manager();
	
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
	QObject::connect(wid->page(), SIGNAL(downloadRequested(QNetworkRequest)), &CWebView::manager, SLOT(downloadRequested(QNetworkRequest)));
  QObject::connect(wid->page(), SIGNAL(unsupportedContent(QNetworkReply*)), &CWebView::manager, SLOT(handleUnsupportedContent(QNetworkReply*)));
	
  QObject::connect(wid, SIGNAL(iconChanged()), &CWebView::manager, SLOT(iconChanged()));
	QObject::connect(wid->page()->mainFrame(), SIGNAL(urlChanged(const QUrl &)), &CWebView::manager, SLOT(urlChanged(const QUrl &)));

	QObject::connect(wid->page()->networkAccessManager(), SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), &CWebView::manager,
										SLOT(authenticationRequired(QNetworkReply *, QAuthenticator *)));
END_METHOD

BEGIN_METHOD_VOID(WebView_free)

	if (_network_access_manager_view == THIS)
		_network_access_manager_view = 0;
	
	GB.FreeString(&THIS->status);
	GB.FreeString(&THIS->userAgent);
	GB.Unref(POINTER(&THIS->icon));

END_METHOD

BEGIN_METHOD_VOID(WebView_init)

	_old_after_set_color = QT.AfterSetColor(after_set_color);

END_METHOD

BEGIN_METHOD_VOID(WebView_exit)

	delete _network_access_manager;

END_METHOD

BEGIN_PROPERTY(WebView_Url)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->url().toString()));
	else
	{
		QUrl url(QSTRING_PROP());
		stop_view(THIS);
		//fprintf(stderr, "setUrl: %s\n", GB.ToZeroString(PROP(GB_STRING)));
		WIDGET->setUrl(url);
	}

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
		
		if (icon.isNull()) 
			icon = QWebSettings::iconForUrl(WIDGET->url());
		
		if (!icon.isNull())
		{
			THIS->icon = QT.CreatePicture(icon.pixmap(16, 16));
			GB.Ref(THIS->icon);
		}
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

BEGIN_METHOD(WebView_Reload, GB_BOOLEAN bypass)

	bool bypass = VARGOPT(bypass, false);
	stop_view(THIS);
	if (bypass)
		WIDGET->page()->triggerAction(QWebPage::ReloadAndBypassCache);
	else
		WIDGET->reload();

END_METHOD

BEGIN_METHOD_VOID(WebView_Stop)

	stop_view(THIS);

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
		GB.ReturnVoidString();

END_PROPERTY

BEGIN_PROPERTY(WebViewAuth_Realm)

	if (THIS->authenticator)
		GB.ReturnNewZeroString(TO_UTF8(THIS->authenticator->realm()));
	else
		GB.ReturnVoidString();

END_PROPERTY

BEGIN_PROPERTY(WebViewAuth_User)

	if (READ_PROPERTY)
	{
		if (THIS->authenticator)
			GB.ReturnNewZeroString(TO_UTF8(THIS->authenticator->user()));
		else
			GB.ReturnVoidString();
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
			GB.ReturnVoidString();
	}
	else
	{
		if (THIS->authenticator)
			THIS->authenticator->setPassword(QSTRING_PROP());
		else
			GB.Error("No authentication required");
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
		// TODO what todo?
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

BEGIN_PROPERTY(WebView_Editable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->page()->isContentEditable());
	else
		WIDGET->page()->setContentEditable(VPROP(GB_BOOLEAN));

END_PROPERTY

#if 0
BEGIN_METHOD(WebView_Exec, GB_STRING action; GB_VARIANT arg)

	QWebPage::WebAction action = get_action(GB.ToZeroString(ARG(action)));
	
	if (action == QWebPage::NoWebAction)
	{
		GB.Error("Unknown action");
		return;
	}
	
	if (MISSING(arg))
	{
		WIDGET->page()->triggerAction(action);
	}
	else
	{
		if (GB.Conv((GB_VALUE *)ARG(arg), GB_T_BOOLEAN))
			return;
		
		WIDGET->page()->triggerAction(action, ((GB_BOOLEAN *)ARG(arg))->value);
	}

END_METHOD
#endif

/*
BEGIN_METHOD(WebView_CanExec, GB_STRING action)

	QWebPage::Action action = get_action(GB.ToZeroString(ARG(action)));
	
	if (action == QWebPage::NoWebAction)
	{
		GB.Error("Unknown action");
		return;
	}
	
END_METHOD
*/

BEGIN_METHOD(WebView_Eval, GB_STRING javascript)

	CWEBFRAME_eval(WIDGET->page()->currentFrame(), QSTRING_ARG(javascript));

END_METHOD

BEGIN_PROPERTY(WebView_UserAgent)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->userAgent);
	else
		GB.StoreString(PROP(GB_STRING), &THIS->userAgent);

END_PROPERTY

BEGIN_PROPERTY(WebView_Document)

	GB.ReturnObject(CWEBELEMENT_create(WIDGET->page()->mainFrame()->documentElement()));

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_PROPERTY(WebViewHistory_Count)

	GB.ReturnInteger(HISTORY->count());

END_PROPERTY

BEGIN_PROPERTY(WebViewHistory_Max)

	GB.ReturnInteger(HISTORY->count() - 1);

END_PROPERTY

BEGIN_PROPERTY(WebViewHistory_Index)

	if (READ_PROPERTY)
		GB.ReturnInteger(HISTORY->currentItemIndex());
	else
	{
		int index = VPROP(GB_INTEGER);
		if (index < 0 || index >= HISTORY->count())
		{
			GB.Error(GB_ERR_ARG);
			return;
		}
		HISTORY->goToItem(HISTORY->itemAt(index));
	}

END_PROPERTY

BEGIN_PROPERTY(WebViewHistory_MaxSize)

	if (READ_PROPERTY)
		GB.ReturnInteger(HISTORY->maximumItemCount());
	else
		HISTORY->setMaximumItemCount(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD_VOID(WebViewHistory_Clear)

	int max = HISTORY->maximumItemCount();
	HISTORY->setMaximumItemCount(0);
	HISTORY->setMaximumItemCount(max);

END_METHOD

//-------------------------------------------------------------------------

GB_DESC WebViewAuthDesc[] =
{
  GB_DECLARE_VIRTUAL(".WebView.Auth"),
	
	GB_PROPERTY_READ("Url", "s", WebViewAuth_Url),
	GB_PROPERTY_READ("Realm", "s", WebViewAuth_Realm),
	GB_PROPERTY("User", "s", WebViewAuth_User),
	GB_PROPERTY("Password", "s", WebViewAuth_Password),
	
	GB_END_DECLARE
};

GB_DESC WebViewHistoryDesc[] =
{
	GB_DECLARE_VIRTUAL(".WebView.History"),

	GB_PROPERTY_READ("Count", "i", WebViewHistory_Count),
	GB_PROPERTY_READ("Max", "i", WebViewHistory_Max),
	GB_PROPERTY("Index", "i", WebViewHistory_Index),
	GB_PROPERTY("MaxSize", "i", WebViewHistory_MaxSize),
	GB_METHOD("Clear", NULL, WebViewHistory_Clear, NULL),

	GB_END_DECLARE
};

GB_DESC WebViewDesc[] =
{
  GB_DECLARE("WebView", sizeof(CWEBVIEW)), GB_INHERITS("Control"),
	
  GB_METHOD("_new", NULL, WebView_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, WebView_free, NULL),
  GB_METHOD("_init", NULL, WebView_init, NULL),
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
	GB_PROPERTY_READ("Document", "WebElement", WebView_Document),
	
	GB_PROPERTY_SELF("Settings", ".WebView.Settings"),
	GB_PROPERTY_SELF("Auth", ".WebView.Auth"),
	GB_PROPERTY_SELF("History", ".WebView.History"),

	GB_METHOD("Back", NULL, WebView_Back, NULL),
	GB_METHOD("Forward", NULL, WebView_Forward, NULL),
	GB_METHOD("Reload", NULL, WebView_Reload, "[(BypassCache)b]"),
	GB_METHOD("Stop", NULL, WebView_Stop, NULL),

	GB_PROPERTY("NewView", "WebView", WebView_NewView),

	GB_PROPERTY("Cookies", "Cookie[]", WebView_Cookies),
	
	GB_METHOD("HitTest", "WebHitTest", WebView_HitTest, "(X)i(Y)i"),
	GB_METHOD("FindText", "b", WebView_FindText, "[(Text)s(Backward)b(CaseSensitive)b(Wrap)b]"),

	GB_PROPERTY("Editable", "b", WebView_Editable),
	//GB_METHOD("Exec", NULL, WebView_Exec, "(Action)s[(Argument)v]"),
	//GB_METHOD("CanExec", "b", WebView_CanExec, "(Action)s"),
	GB_METHOD("Eval", "v", WebView_Eval, "(JavaScript)s"),

	GB_PROPERTY("UserAgent", "s", WebView_UserAgent),

	GB_CONSTANT("_Properties", "s", "*,Url,Editable"),
	GB_CONSTANT("_Group", "s", "View"),
	
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

MyWebPage::MyWebPage(QObject *parent) : QWebPage(parent)
{
}

QString MyWebPage::userAgentForUrl(const QUrl& url) const 
{
	MyWebView *view = (MyWebView *)parent();
	void *_object = QT.GetObject(view);
	
	if (THIS->userAgent)
		return (const char *)THIS->userAgent;
	else
		return QWebPage::userAgentForUrl(url);
};


MyWebView::MyWebView(QWidget *parent) : QWebView(parent)
{
	//settings()->setFontFamily(QWebSettings::FixedFont, "monospace");
	setPage(new MyWebPage(this));
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

	//fprintf(stderr, "loadFinished %d (%d)\n", ok, THIS->stopping);

	THIS->progress = 1;

	if (ok)
		GB.Raise(THIS, EVENT_LOAD, 0);
	else if (!THIS->stopping)
		GB.RaiseLater(THIS, EVENT_ERROR);
}

void CWebView::loadProgress(int progress)
{
	GET_SENDER();

	double v = progress / 100.0;
	if (THIS->progress == v)
		return;

	THIS->progress = v;
	GB.Raise(THIS, EVENT_PROGRESS, 0);
}

void CWebView::loadStarted()
{
	GET_SENDER();

	//fprintf(stderr, "loadStarted\n");

	THIS->progress = 0;
	_network_access_manager_view = THIS;
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
	//qDebug("CWebView::authenticationRequired: %p", _network_access_manager_view);

	void *_object = _network_access_manager_view; //QT.GetObject((QWidget *)((QNetworkAccessManager*)sender())->parent());
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
	//void *_object = QT.GetObject(((QWebFrame *)sender())->page()->view());
	GET_SENDER();
	GB.Unref(POINTER(&THIS->icon));
	THIS->icon = NULL;
	GB.RaiseLater(THIS, EVENT_ICON);
}

void CWebView::urlChanged(const QUrl &)
{
	QWebFrame *frame = (QWebFrame *)sender();
	//fprintf(stderr, "urlChanged: %p\n", frame);
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

	delete reply;
}
