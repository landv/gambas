/***************************************************************************

  cwebview.h

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

#ifndef __CWEBVIEW_H
#define __CWEBVIEW_H

#include "main.h"

#include <QUrl>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QWebView>

#ifndef __CWEBVIEW_CPP

extern GB_DESC CWebViewAuthDesc[];
extern GB_DESC CWebViewDesc[];

#else

#define THIS      ((CWEBVIEW *)_object)
#define WIDGET    ((MyWebView *)((QT_WIDGET *)_object)->widget)

#endif

class MyWebView : public QWebView
{
	Q_OBJECT

public:
	
	MyWebView(QWidget *parent);

protected:
		
	virtual QWebView *createWindow(QWebPage::WebWindowType type);
};

typedef
	struct 
	{
		QT_WIDGET widget;
		void *new_view;
		double progress;
		char *status;
		QT_PICTURE icon;
		QNetworkReply *reply;
		QAuthenticator *authenticator;
	}
	CWEBVIEW;

class CWebView : public QObject
{
  Q_OBJECT

public:

  static CWebView manager;

public slots:

	void iconChanged();
	void linkClicked(const QUrl &url);
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void selectionChanged();
	void statusBarMessage(const QString &text);
	void titleChanged(const QString &title);
	void linkHovered(const QString &link, const QString &title, const QString &textContent);
	void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
};

#endif
