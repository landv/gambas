/***************************************************************************

  cwebsettings.cpp

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/

#define __CWEBSETTINGS_CPP

#include <QNetworkDiskCache>
#include <QNetworkProxy>

#include "cwebview.h"
#include "cwebsettings.h"

//static QNetworkDiskCache *_cache = 0;
static char *_cache_path = 0;

void WEBSETTINGS_set_cache(QWebView *view, bool on)
{
	QNetworkDiskCache *cache;
	
	if (!_cache_path)
		return;
	
	if (on)
	{
		cache = new QNetworkDiskCache(0);
		cache->setCacheDirectory(TO_QSTRING(_cache_path));
		view->page()->networkAccessManager()->setCache(cache);
	}
	else
		view->page()->networkAccessManager()->setCache(0);
}


static QWebSettings *get_settings(void *_object)
{
	if (!_object)
		return QWebSettings::globalSettings();
	else
		return WEBVIEW->settings();
}


BEGIN_METHOD(WebSettingsFont_get, GB_INTEGER font)

	QWebSettings *settings = get_settings(_object);
	QWebSettings::FontFamily font(QWebSettings::FontFamily(VARG(font)));

	GB.ReturnNewZeroString(TO_UTF8(settings->fontFamily(font)));

END_METHOD

BEGIN_METHOD(WebSettingsFont_put, GB_STRING family; GB_INTEGER font)

	QWebSettings *settings = get_settings(_object);
	QWebSettings::FontFamily font(QWebSettings::FontFamily(VARG(font)));
	
	settings->setFontFamily(font, QSTRING_ARG(family));

END_METHOD


BEGIN_METHOD_VOID(WebSettingsIconDatabase_Clear)

	QWebSettings::clearIconDatabase();

END_METHOD

BEGIN_PROPERTY(WebSettingsIconDatabase_Path)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(QWebSettings::iconDatabasePath()));
	else
		QWebSettings::setIconDatabasePath(QSTRING_PROP());

END_PROPERTY

BEGIN_METHOD(WebSettingsIconDatabase_get, GB_STRING url)

	QIcon icon;
	QSize size;
	QSize max_size;
	
	icon = QWebSettings::iconForUrl(QSTRING_ARG(url));
	if (icon.isNull())
	{
		GB.ReturnNull();
		return;
	}
	
	foreach(size, icon.availableSizes())
	{
		if ((size.width() * size.height()) > (max_size.width() * max_size.height()))
			max_size = size;
	}
	
	GB.ReturnObject(QT.CreatePicture(icon.pixmap(max_size)));

END_METHOD

BEGIN_PROPERTY(WebSettingsCache_Path)

	if (READ_PROPERTY)
		GB.ReturnString(_cache_path);
	else
		GB.StoreString(PROP(GB_STRING), &_cache_path);

END_PROPERTY


BEGIN_METHOD(WebSettings_get, GB_INTEGER flag)

	QWebSettings *settings = get_settings(_object);
	QWebSettings::WebAttribute flag(QWebSettings::WebAttribute(VARG(flag)));
	
	GB.ReturnBoolean(settings->testAttribute(flag));

END_METHOD

BEGIN_METHOD(WebSettings_put, GB_BOOLEAN value; GB_INTEGER flag)

	QWebSettings *settings = get_settings(_object);
	QWebSettings::WebAttribute flag(QWebSettings::WebAttribute(VARG(flag)));
	
	settings->setAttribute(flag, VARG(value));

END_METHOD

BEGIN_METHOD(WebSettings_Reset, GB_INTEGER flag)

	QWebSettings *settings = get_settings(_object);
	QWebSettings::WebAttribute flag(QWebSettings::WebAttribute(VARG(flag)));
	
	settings->resetAttribute(flag);

END_METHOD

BEGIN_METHOD_VOID(WebSettings_exit)

	GB.FreeString(&_cache_path);

END_METHOD

/***************************************************************************/

BEGIN_PROPERTY(WebSettingsProxy_Host)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(proxy.hostName()));
	else
	{
		proxy.setHostName(QSTRING_PROP());
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_User)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(proxy.user()));
	else
	{
		proxy.setUser(QSTRING_PROP());
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_Password)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(proxy.password()));
	else
	{
		proxy.setPassword(QSTRING_PROP());
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_Port)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(proxy.port());
	else
	{
		proxy.setPort(VPROP(GB_INTEGER));
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_Type)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(proxy.type());
	else
	{
		int type = VPROP(GB_INTEGER);
		if (type == QNetworkProxy::NoProxy || type == QNetworkProxy::Socks5Proxy || type == QNetworkProxy::HttpProxy)
		{
			proxy.setType((QNetworkProxy::ProxyType)type);
			nam->setProxy(proxy);
		}
	}

END_PROPERTY

/***************************************************************************/

GB_DESC CWebViewSettingsDesc[] =
{
  GB_DECLARE(".WebView.Settings", 0),
	
	GB_METHOD("_get", "b", WebSettings_get, "(Flag)i"),
	GB_METHOD("_put", NULL, WebSettings_put, "(Value)b(Flag)i"),
	GB_METHOD("Reset", NULL, WebSettings_Reset, "(Flag)i"),
	
	GB_END_DECLARE
};

GB_DESC CWebSettingsFontDesc[] =
{
  GB_DECLARE(".WebSettings.Font", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_METHOD("_get", "s", WebSettingsFont_get, "(Font)i"),
	GB_STATIC_METHOD("_put", NULL, WebSettingsFont_get, "(Family)s(Font)i"),
	
	GB_END_DECLARE
};

GB_DESC CWebSettingsIconDatabaseDesc[] =
{
  GB_DECLARE(".WebSettings.IconDatabase", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_METHOD("Clear", NULL, WebSettingsIconDatabase_Clear, NULL),
	GB_STATIC_PROPERTY("Path", "s", WebSettingsIconDatabase_Path),
	GB_STATIC_METHOD("_get", "Picture", WebSettingsIconDatabase_get, "(Url)s"),
	
	GB_END_DECLARE
};

GB_DESC CWebSettingsCacheDesc[] =
{
  GB_DECLARE(".WebSettings.Cache", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Path", "s", WebSettingsCache_Path),
	
	GB_END_DECLARE
};

GB_DESC CWebSettingsProxyDesc[] =
{
  GB_DECLARE(".WebSettings.Proxy", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Type", "i", WebSettingsProxy_Type),
	GB_STATIC_PROPERTY("Host", "s", WebSettingsProxy_Host),
	GB_STATIC_PROPERTY("Port", "i", WebSettingsProxy_Port),
	GB_STATIC_PROPERTY("User", "s", WebSettingsProxy_User),
	GB_STATIC_PROPERTY("Password", "s", WebSettingsProxy_Password),
	
	GB_END_DECLARE
};

GB_DESC CWebSettingsDesc[] =
{
  GB_DECLARE("WebSettings", 0),
	
	GB_CONSTANT("StandardFont", "i", QWebSettings::StandardFont),
	GB_CONSTANT("FixedFont", "i", QWebSettings::FixedFont),
	GB_CONSTANT("SerifFont", "i", QWebSettings::SerifFont),
	GB_CONSTANT("SansSerifFont", "i", QWebSettings::SansSerifFont),
	GB_CONSTANT("CursiveFont", "i", QWebSettings::CursiveFont),
	GB_CONSTANT("FantasyFont", "i", QWebSettings::FantasyFont),
	
	GB_CONSTANT("AutoLoadImages", "i", QWebSettings::AutoLoadImages),
	GB_CONSTANT("JavascriptEnabled", "i", QWebSettings::JavascriptEnabled),
	GB_CONSTANT("JavaEnabled", "i", QWebSettings::JavaEnabled),
	GB_CONSTANT("PluginsEnabled", "i", QWebSettings::PluginsEnabled),
	GB_CONSTANT("PrivateBrowsingEnabled", "i", QWebSettings::PrivateBrowsingEnabled),
	GB_CONSTANT("JavascriptCanOpenWindows", "i", QWebSettings::JavascriptCanOpenWindows),
	GB_CONSTANT("JavascriptCanAccessClipboard", "i", QWebSettings::JavascriptCanAccessClipboard),
	GB_CONSTANT("DeveloperExtrasEnabled", "i", QWebSettings::DeveloperExtrasEnabled),
	GB_CONSTANT("LinksIncludedInFocusChain", "i", QWebSettings::LinksIncludedInFocusChain),
	GB_CONSTANT("ZoomTextOnly", "i", QWebSettings::ZoomTextOnly),
	GB_CONSTANT("PrintElementBackgrounds", "i", QWebSettings::PrintElementBackgrounds),
	GB_CONSTANT("OfflineStorageDatabaseEnabled", "i", QWebSettings::OfflineStorageDatabaseEnabled),
	GB_CONSTANT("OfflineWebApplicationCacheEnabled", "i", QWebSettings::OfflineWebApplicationCacheEnabled),
	GB_CONSTANT("LocalStorageDatabaseEnabled", "i", QWebSettings::LocalStorageDatabaseEnabled),
	
	GB_CONSTANT("NoProxy", "i", QNetworkProxy::NoProxy),
	GB_CONSTANT("Socks5Proxy", "i", QNetworkProxy::Socks5Proxy),
	GB_CONSTANT("HttpProxy", "i", QNetworkProxy::HttpProxy),
	
	GB_STATIC_PROPERTY_SELF("Font", ".WebSettings.Font"),
	GB_STATIC_PROPERTY_SELF("IconDatabase", ".WebSettings.IconDatabase"),
	GB_STATIC_PROPERTY_SELF("Cache", ".WebSettings.Cache"),
	GB_STATIC_PROPERTY_SELF("Proxy", ".WebSettings.Proxy"),
	
	GB_STATIC_METHOD("_get", "b", WebSettings_get, "(Flag)i"),
	GB_STATIC_METHOD("_put", NULL, WebSettings_put, "(Value)b(Flag)i"),
	
	GB_STATIC_METHOD("_exit", NULL, WebSettings_exit, NULL),

GB_END_DECLARE
};

/***************************************************************************/


