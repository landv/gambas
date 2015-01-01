/***************************************************************************

  cwebdownload.cpp

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

#define __CWEBDOWNLOAD_CPP

#include <QNetworkRequest>

#include "cwebdownload.h"

/***************************************************************************/

static CWEBDOWNLOAD *get_download(QNetworkReply *reply)
{
	CWEBDOWNLOAD *download = 0;
	sscanf(TO_UTF8(reply->objectName()), "gb-download-%p", &download);
	return download;
}

static void set_error(CWEBDOWNLOAD *_object, const char *error)
{
	GB.FreeString(&THIS->error);
	if (error) THIS->error = GB.NewZeroString(error);
}

static void abort_download(CWEBDOWNLOAD *_object, const char *error)
{
	REPLY->abort();
	if (error)
	{
		set_error(THIS, error);
		THIS->status = STATUS_ERROR;
	}
}

/***************************************************************************/

static CWEBDOWNLOAD **_downloads = NULL;

int WEB_get_downloads_count()
{
	if (_downloads)
		return GB.Count(_downloads);
	else
		return 0;
}

#define get_downloads_count WEB_get_downloads_count

static int find_download(CWEBDOWNLOAD *download)
{
	int i;
	
	for (i = 0; i < get_downloads_count(); i++)
	{
		if (_downloads[i] == download)
			return i;
	}
	
	return (-1);
}

void WEB_remove_download(CWEBDOWNLOAD *_object)
{
	int index;
	
	abort_download(THIS, NULL);
	
	index = find_download(THIS);
	
	if (index >= 0)
	{
		GB.Unref(POINTER(&_downloads[index]));
		GB.Remove(&_downloads, index, 1);
	}
}

/***************************************************************************/

BEGIN_PROPERTY(WebDownload_Status)

	GB.ReturnInteger(THIS->status);

END_PROPERTY

BEGIN_PROPERTY(WebDownload_Url)

	GB.ReturnNewZeroString(TO_UTF8(REPLY->url().toString()));

END_PROPERTY

BEGIN_PROPERTY(WebDownload_Path)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->path);
	else
		GB.StoreString(PROP(GB_STRING), &THIS->path);

END_PROPERTY

BEGIN_PROPERTY(WebDownload_Size)

	qulonglong size = 0;
	
	size = REPLY->header(QNetworkRequest::ContentLengthHeader).toULongLong();
	
	GB.ReturnLong(size);

END_PROPERTY

BEGIN_PROPERTY(WebDownload_ErrorText)

	GB.ReturnString(THIS->error);

END_PROPERTY

BEGIN_PROPERTY(WebDownload_Progress)

	GB.ReturnFloat(THIS->progress);

END_PROPERTY

BEGIN_METHOD_VOID(WebDownload_Cancel)

	abort_download(THIS, NULL);

END_METHOD

BEGIN_METHOD_VOID(WebDownload_Delete)

	WEB_remove_download(THIS);

END_METHOD

BEGIN_METHOD_VOID(WebDownload_free)

	if (THIS->reply)
		THIS->reply->abort();
	
	delete THIS->output;
	THIS->reply->deleteLater();
	set_error(THIS, NULL);
	GB.FreeString(&THIS->path);

END_METHOD

GB_DESC WebDownloadDesc[] =
{
  GB_DECLARE("WebDownload", sizeof(CWEBDOWNLOAD)),
  
  GB_METHOD("_free", NULL, WebDownload_free, NULL),
  
  GB_CONSTANT("Created", "i", STATUS_CREATED),
  GB_CONSTANT("Downloading", "i", STATUS_DOWNLOADING),
  GB_CONSTANT("Finished", "i", STATUS_FINISHED),
  GB_CONSTANT("Cancelled", "i", STATUS_CANCELLED),
  GB_CONSTANT("Error", "i", STATUS_ERROR),
  
	GB_PROPERTY("Path", "s", WebDownload_Path),
	GB_PROPERTY_READ("Url", "s", WebDownload_Url),
	GB_PROPERTY_READ("Size", "l", WebDownload_Size),
	GB_PROPERTY_READ("Progress", "f", WebDownload_Progress),
	GB_PROPERTY_READ("Status", "i", WebDownload_Status),
	GB_PROPERTY_READ("ErrorText", "s", WebDownload_ErrorText),
	
	GB_METHOD("Cancel", NULL, WebDownload_Cancel, NULL),
	GB_METHOD("Delete", NULL, WebDownload_Delete, NULL),

	GB_END_DECLARE
};

/***************************************************************************/

BEGIN_PROPERTY(WebDownloads_Count)

	GB.ReturnInteger(get_downloads_count());

END_PROPERTY

BEGIN_METHOD(WebDownloads_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= get_downloads_count())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	GB.ReturnObject(_downloads[index]);

END_METHOD

BEGIN_METHOD(WebDownloads_Remove, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= get_downloads_count())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	WEB_remove_download(_downloads[index]);
	
END_METHOD

BEGIN_METHOD(WebDownloads_Find, GB_OBJECT download)

	CWEBDOWNLOAD *download = (CWEBDOWNLOAD *)VARG(download);
	
	GB.ReturnInteger(find_download(download));

END_METHOD

BEGIN_METHOD_VOID(WebDownloads_exit)

	while (get_downloads_count())
		WEB_remove_download(_downloads[0]);
	
	GB.FreeArray(&_downloads);

END_METHOD

GB_DESC WebDownloadsDesc[] =
{
  GB_DECLARE("WebDownloads", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_METHOD("_exit", NULL, WebDownloads_exit, NULL),
	GB_STATIC_PROPERTY_READ("Count", "i", WebDownloads_Count),
	GB_STATIC_METHOD("_get", "WebDownload", WebDownloads_get, "(Index)i"),
	GB_STATIC_METHOD("Remove", NULL, WebDownloads_Remove, "(Index)i"),
	GB_STATIC_METHOD("Find", "i", WebDownloads_Find, "(Download)WebDownload"),
	
	GB_END_DECLARE
};


/***************************************************************************/

CWEBDOWNLOAD *WEB_create_download(QNetworkReply *reply)
{
	CWEBDOWNLOAD *_object;
	char name[32];
	int index;
	
  _object = (CWEBDOWNLOAD *)GB.New(GB.FindClass("WebDownload"), NULL, NULL);
	THIS->reply = reply;
	reply->setParent(0);
	sprintf(name, "gb-download-%p", THIS);
	reply->setObjectName(name);
	
	QObject::connect(reply, SIGNAL(readyRead()), &CWebDownload::manager, SLOT(readyRead()));
	QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &CWebDownload::manager, SLOT(error(QNetworkReply::NetworkError)));
	QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), &CWebDownload::manager, SLOT(downloadProgress(qint64,qint64)));
	QObject::connect(reply, SIGNAL(finished()), &CWebDownload::manager, SLOT(finished()));
	
	index = get_downloads_count();
	
	if (!_downloads)
		GB.NewArray(&_downloads, sizeof(*_downloads), 1);
	else
		GB.Add(&_downloads);
	
	_downloads[index] = THIS;
	GB.Ref(THIS);
	
	return THIS;
}

/***************************************************************************/

CWebDownload CWebDownload::manager;

#define GET_DOWNLOAD() \
	QNetworkReply *reply = (QNetworkReply *)sender(); \
	CWEBDOWNLOAD *_object = get_download(reply);

void CWebDownload::readyRead()
{
	GET_DOWNLOAD();
	
	if (!THIS->path)
	{
		//abort_download(THIS, "No file name specified");
		return;
	}
	
	if (!THIS->output)
	{
		THIS->output = new QFile(TO_QSTRING(THIS->path));

		if (!THIS->output->open(QIODevice::WriteOnly)) 
		{
			char *error = NULL;
			error = GB.AddString(error, "Unable to save file: ", 0);
			error = GB.AddString(error, TO_UTF8(THIS->output->errorString()), 0);
			abort_download(THIS, error);
			return;
		}
	}
	
	if (THIS->output->write(reply->readAll()) < 0) 
	{
		abort_download(THIS, TO_UTF8(THIS->output->errorString()));
		return;
	}
	
	THIS->status = STATUS_DOWNLOADING;
}

void CWebDownload::error(QNetworkReply::NetworkError code)
{
	GET_DOWNLOAD();
	
	if (code == QNetworkReply::OperationCanceledError)
	{
		THIS->status = STATUS_CANCELLED;
		GB.FreeString(&THIS->error);
	}
	else
	{
		THIS->status = STATUS_ERROR;
		if (!THIS->error)
			THIS->error = GB.NewZeroString(TO_UTF8(reply->errorString()));
	}
}

void CWebDownload::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	GET_DOWNLOAD();
	
	if (bytesTotal < 0)
		THIS->progress = 0;
	else
		THIS->progress = (double)bytesReceived / bytesTotal;
}

void CWebDownload::finished()
{
	GET_DOWNLOAD();
	
	if (THIS->status == STATUS_DOWNLOADING)
	{
		THIS->output->close();
		THIS->status = STATUS_FINISHED;
	}
	
	THIS->progress = 1;
}
