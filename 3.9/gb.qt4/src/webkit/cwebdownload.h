/***************************************************************************

  cwebdownload.h

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

#ifndef __CWEBDOWNLOAD_H
#define __CWEBDOWNLOAD_H

#include <QNetworkReply>
#include <QFile>

#include "main.h"

#ifndef __CWEBDOWNLOAD_CPP

extern GB_DESC WebDownloadDesc[];
extern GB_DESC WebDownloadsDesc[];

#else

#define THIS ((CWEBDOWNLOAD *)_object)
#define REPLY (THIS->reply)

#endif

enum {
	STATUS_CREATED,
	STATUS_DOWNLOADING,
	STATUS_ERROR,
	STATUS_CANCELLED,
	STATUS_FINISHED
};

typedef
	struct 
	{
		GB_BASE ob;
		QNetworkReply *reply;
		int status;
		char *path;
		char *error;
		double progress;
		QFile *output;
	}
	CWEBDOWNLOAD;

class CWebDownload : public QObject
{
	Q_OBJECT

public:

	static CWebDownload manager;

public slots:

	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void error(QNetworkReply::NetworkError code);
	void finished();
	void readyRead();
	//void 	metaDataChanged ()
	//void 	sslErrors ( const QList<QSslError> & errors )
	//void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
};


CWEBDOWNLOAD *WEB_create_download(QNetworkReply *reply);
int WEB_get_downloads_count();
void WEB_remove_download(CWEBDOWNLOAD *_object);

#endif
