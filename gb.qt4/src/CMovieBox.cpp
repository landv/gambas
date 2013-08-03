/***************************************************************************

  CMovieBox.cpp

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

#define __CMOVIEBOX_CPP

#include "gambas.h"
#include "main.h"

#include <qmovie.h>
//Added by qt3to4:
#include <QLabel>

#include "CConst.h"
#include "CMovieBox.h"


static void free_movie(void *_object)
{
  if (!THIS->movie)
    return;
  
  delete THIS->movie;
  THIS->movie = 0;

	THIS->buffer->close();
	delete THIS->buffer;
	
	THIS->data->clear();
  delete THIS->data;
  
  GB.ReleaseFile(THIS->addr, THIS->len);
  
  GB.StoreString(NULL, &THIS->path);
	
	if (WIDGET)
		WIDGET->setText("");
}

static bool load_movie(void *_object, char *path, int len)
{
  free_movie(THIS);
  
	if (len > 0)
	{
		//qDebug("load_movie: %.*s", (int)len, path);
		if (GB.LoadFile(path, len, &THIS->addr, &THIS->len))
			return true;

		THIS->data = new QByteArray();    
		*THIS->data = QByteArray::fromRawData((const char *)THIS->addr, THIS->len);
		THIS->buffer = new QBuffer(THIS->data);
		THIS->buffer->open(QIODevice::ReadOnly);
		THIS->movie = new QMovie(THIS->buffer);
		
		THIS->path = GB.NewString(path, len);
		
		//qDebug("setMovie");
		WIDGET->setMovie(THIS->movie);
	}
  
  return false;
}


BEGIN_METHOD(CMOVIEBOX_new, GB_OBJECT parent)

  QLabel *wid = new QLabel(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, _object);

  wid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

END_METHOD

BEGIN_METHOD_VOID(CMOVIEBOX_free)

  free_movie(THIS);

END_METHOD


BEGIN_PROPERTY(CMOVIEBOX_path)

  if (READ_PROPERTY)
    GB.ReturnString(THIS->path);
  else
  {
    bool playing = false;

    if (THIS->movie)
      playing = THIS->movie->state() == QMovie::Running;
    else
      playing = FALSE;

    if (load_movie(THIS, PSTRING(), PLENGTH()))
      return;
          
    if (!playing && THIS->movie)
      THIS->movie->setPaused(true);
  }

END_PROPERTY


BEGIN_PROPERTY(CMOVIEBOX_playing)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->movie ? THIS->movie->state() == QMovie::Running : FALSE);
  else if (THIS->movie)
  {
    if (VPROP(GB_BOOLEAN))
      THIS->movie->setPaused(false);
    else
      THIS->movie->setPaused(true);
  }

END_PROPERTY


BEGIN_METHOD_VOID(CMOVIEBOX_rewind)

  if (!THIS->movie)
    return;
    
  THIS->movie->stop();
  THIS->movie->start();

END_METHOD

BEGIN_PROPERTY(MovieBox_Alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true));

END_PROPERTY


GB_DESC CMovieBoxDesc[] =
{
  GB_DECLARE("MovieBox", sizeof(CMOVIEBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CMOVIEBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CMOVIEBOX_free, NULL),

  GB_PROPERTY("Path", "s", CMOVIEBOX_path),
  GB_PROPERTY("Playing", "b", CMOVIEBOX_playing),
  //GB_PROPERTY("Alignment", "i<Align>", CMOVIEBOX_alignment),
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("Alignment", "i", MovieBox_Alignment),
  
  GB_METHOD("Rewind", NULL, CMOVIEBOX_rewind, NULL),

	MOVIEBOX_DESCRIPTION,

  GB_END_DECLARE
};

