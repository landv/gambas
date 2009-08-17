/***************************************************************************

  cwebsettings.h

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

#ifndef __CWEBSETTINGS_H
#define __CWEBSETTINGS_H

#include "main.h"

#include <QUrl>
#include <QWebView>
#include <QWebSettings>

#ifndef __CWEBSETTINGS_CPP

extern GB_DESC CWebViewSettingsDesc[];
extern GB_DESC CWebSettingsFontDesc[];
extern GB_DESC CWebSettingsIconDatabaseDesc[];
extern GB_DESC CWebSettingsDesc[];

#else

#define WEBVIEW ((QWebView *)((QT_WIDGET *)_object)->widget)

#endif

#endif
