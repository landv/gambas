/***************************************************************************

  cwebsettings.cpp

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

#define __CWEBSETTINGS_CPP

#include <errno.h>
#include <unistd.h>

#include <QNetworkDiskCache>
#include <QNetworkProxy>

#include "cwebview.h"
#include "cwebsettings.h"

//static QNetworkDiskCache *_cache = 0;
static bool _cache_enabled = false;
static char *_cache_path = 0;

static void set_cache(bool on)
{
	QNetworkDiskCache *cache;
	
	if (!_cache_path)
		return;
	
	_cache_enabled = on;
	
	if (on)
	{
		cache = new QNetworkDiskCache(0);
		cache->setCacheDirectory(TO_QSTRING(_cache_path));
		WEBVIEW_get_network_manager()->setCache(cache);
	}
	else
		WEBVIEW_get_network_manager()->setCache(0);
}


static QWebSettings *get_settings(void *_object)
{
	if (!_object)
		return QWebSettings::globalSettings();
	else
		return WEBVIEW->settings();
}

/***************************************************************************/

static void handle_font_family(QWebSettings::FontFamily font, void *_object, void *_param)
{
	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(get_settings(_object)->fontFamily(font)));
	else
		get_settings(_object)->setFontFamily(font, QSTRING_PROP());
}

BEGIN_PROPERTY(WebSettingsFonts_CursiveFont)

	handle_font_family(QWebSettings::CursiveFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_FantasyFont)

	handle_font_family(QWebSettings::FantasyFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_FixedFont)

	handle_font_family(QWebSettings::FixedFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_SansSerifFont)

	handle_font_family(QWebSettings::SansSerifFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_SerifFont)

	handle_font_family(QWebSettings::SerifFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_StandardFont)

	handle_font_family(QWebSettings::StandardFont, _object, _param);

END_METHOD

static void handle_font_size(QWebSettings::FontSize size, void *_object, void *_param)
{
	if (READ_PROPERTY)
		GB.ReturnInteger(get_settings(_object)->fontSize(size));
	else
		get_settings(_object)->setFontSize(size, VPROP(GB_INTEGER));
}

BEGIN_PROPERTY(WebSettingsFonts_DefaultFixedFontSize)

	handle_font_size(QWebSettings::DefaultFixedFontSize, _object, _param);

END_PROPERTY

BEGIN_PROPERTY(WebSettingsFonts_DefaultFontSize)

	handle_font_size(QWebSettings::DefaultFontSize, _object, _param);

END_PROPERTY

BEGIN_PROPERTY(WebSettingsFonts_MinimumFontSize)

	handle_font_size(QWebSettings::MinimumFontSize, _object, _param);

END_PROPERTY

BEGIN_PROPERTY(WebSettingsFonts_MinimumLogicalFontSize)

	handle_font_size(QWebSettings::MinimumLogicalFontSize, _object, _param);

END_PROPERTY


/***************************************************************************/

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


/***************************************************************************/

BEGIN_PROPERTY(WebSettingsCache_Path)

	if (READ_PROPERTY)
		GB.ReturnString(_cache_path);
	else
	{
		QString path = QSTRING_PROP();
		QString root = QString(GB.System.Home());

		if (root.at(root.length() - 1) != '/')
			root += '/';
		root += ".cache/";
		if (!path.startsWith(root))
		{
			GB.Error("Cache directory must be located inside ~/.cache");
			return;
		}

		GB.StoreString(PROP(GB_STRING), &_cache_path);
		set_cache(_cache_enabled);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsCache_Enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_cache_enabled);
	else
		set_cache(VPROP(GB_BOOLEAN));

END_PROPERTY

static int _clear_error;
static char *_clear_path;

static void remove_file(const char *path)
{
	if (rmdir(path) == 0)
		return;

	if (errno == ENOTDIR)
	{
		if (unlink(path) == 0)
			return;
	}

	if (_clear_error == 0)
	{
		_clear_error = errno;
		_clear_path = GB.NewZeroString(path);
	}
}

BEGIN_METHOD_VOID(WebSettingsCache_Clear)

	if (!_cache_path || !*_cache_path)
		return;

	_clear_error = 0;
	GB.BrowseDirectory(_cache_path, NULL, remove_file);

	if (_clear_error)
	{
		GB.Error("Unable to remove '&1': &2", _clear_path, strerror(_clear_error));
		GB.FreeString(&_clear_path);
	}

	set_cache(_cache_enabled);

END_METHOD


/***************************************************************************/

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


/***************************************************************************/

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
		if (type == QNetworkProxy::DefaultProxy || type == QNetworkProxy::NoProxy || type == QNetworkProxy::Socks5Proxy || type == QNetworkProxy::HttpProxy)
		{
			proxy.setType((QNetworkProxy::ProxyType)type);
			nam->setProxy(proxy);
		}
	}

END_PROPERTY

/***************************************************************************/

GB_DESC WebViewSettingsDesc[] =
{
  GB_DECLARE(".WebView.Settings", 0),
	
	GB_METHOD("_get", "b", WebSettings_get, "(Flag)i"),
	GB_METHOD("_put", NULL, WebSettings_put, "(Value)b(Flag)i"),
	GB_METHOD("Reset", NULL, WebSettings_Reset, "(Flag)i"),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsFontsDesc[] =
{
	GB_DECLARE(".WebSettings.Fonts", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY("StandardFont", "s", WebSettingsFonts_StandardFont),
	GB_STATIC_PROPERTY("FixedFont", "s", WebSettingsFonts_FixedFont),
	GB_STATIC_PROPERTY("SerifFont", "s", WebSettingsFonts_SerifFont),
	GB_STATIC_PROPERTY("SansSerifFont", "s", WebSettingsFonts_SansSerifFont),
	GB_STATIC_PROPERTY("CursiveFont", "s", WebSettingsFonts_CursiveFont),
	GB_STATIC_PROPERTY("FantasyFont", "s", WebSettingsFonts_FantasyFont),

	GB_STATIC_PROPERTY("MinimumFontSize", "i", WebSettingsFonts_MinimumFontSize),
	GB_STATIC_PROPERTY("MinimumLogicalFontSize", "i", WebSettingsFonts_MinimumLogicalFontSize),
	GB_STATIC_PROPERTY("DefaultFontSize", "i", WebSettingsFonts_DefaultFontSize),
	GB_STATIC_PROPERTY("DefaultFixedFontSize", "i", WebSettingsFonts_DefaultFixedFontSize),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsIconDatabaseDesc[] =
{
  GB_DECLARE(".WebSettings.IconDatabase", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_METHOD("Clear", NULL, WebSettingsIconDatabase_Clear, NULL),
	GB_STATIC_PROPERTY("Path", "s", WebSettingsIconDatabase_Path),
	GB_STATIC_METHOD("_get", "Picture", WebSettingsIconDatabase_get, "(Url)s"),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsCacheDesc[] =
{
  GB_DECLARE(".WebSettings.Cache", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Enabled", "b", WebSettingsCache_Enabled),
	GB_STATIC_PROPERTY("Path", "s", WebSettingsCache_Path),
	GB_STATIC_METHOD("Clear", NULL, WebSettingsCache_Clear, NULL),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsProxyDesc[] =
{
  GB_DECLARE(".WebSettings.Proxy", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Type", "i", WebSettingsProxy_Type),
	GB_STATIC_PROPERTY("Host", "s", WebSettingsProxy_Host),
	GB_STATIC_PROPERTY("Port", "i", WebSettingsProxy_Port),
	GB_STATIC_PROPERTY("User", "s", WebSettingsProxy_User),
	GB_STATIC_PROPERTY("Password", "s", WebSettingsProxy_Password),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsDesc[] =
{
  GB_DECLARE("WebSettings", 0),
	
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
	
	GB_CONSTANT("DefaultProxy", "i", QNetworkProxy::DefaultProxy),
	GB_CONSTANT("NoProxy", "i", QNetworkProxy::NoProxy),
	GB_CONSTANT("Socks5Proxy", "i", QNetworkProxy::Socks5Proxy),
	GB_CONSTANT("HttpProxy", "i", QNetworkProxy::HttpProxy),
	
	GB_STATIC_PROPERTY_SELF("Fonts", ".WebSettings.Fonts"),
	GB_STATIC_PROPERTY_SELF("IconDatabase", ".WebSettings.IconDatabase"),
	GB_STATIC_PROPERTY_SELF("Cache", ".WebSettings.Cache"),
	GB_STATIC_PROPERTY_SELF("Proxy", ".WebSettings.Proxy"),
	
	GB_STATIC_METHOD("_get", "b", WebSettings_get, "(Flag)i"),
	GB_STATIC_METHOD("_put", NULL, WebSettings_put, "(Value)b(Flag)i"),
	
	GB_STATIC_METHOD("_exit", NULL, WebSettings_exit, NULL),

GB_END_DECLARE
};

/***************************************************************************/


