/***************************************************************************

  CWebBrowser.h

  The WebBrowser class

  (c) 2000-2004 Beno� Minisini <gambas@users.sourceforge.net>

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

#ifndef __CWEBBROWSER_H
#define __CWEBBROWSER_H

#include <khtml_part.h>
#include "main.h"

#ifndef __CWEBBROWSER_CPP

extern GB_DESC CWebBrowserDesc[];

#else

#define THIS    ((CWEBBROWSER *)_object)
#define PART    (THIS->part)
#define WIDGET  ((KHTMLView *)((QT_WIDGET *)_object)->widget)

#define CWEBBROWSER_PROPERTIES QT_WIDGET_PROPERTIES \
  ",Path,JavaScript=False"

#endif

typedef
  struct {
    QT_WIDGET widget;
    KHTMLPart *part;
    char *url;
    }
  CWEBBROWSER;

/*
class MyDatePicker : public KDatePicker
{
  Q_OBJECT

public:

  MyDatePicker(QWidget *);
};
*/

class CWebBrowser : public QObject
{
  Q_OBJECT

public:

  static CWebBrowser manager;

  static void storeURL(CWEBBROWSER *_object, const QString & url);

public slots:

  void click(const KURL &, const KParts::URLArgs &);
  void link(const QString &);
  void change();
  void newFrame(KParts::Part *);
};

#endif
