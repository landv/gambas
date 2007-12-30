/***************************************************************************

  CMovieBox.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CMOVIEBOX_CPP

#include "gambas.h"
#include "main.h"

#include <qmovie.h>

#include "CWidget.h"
#include "CMovieBox.h"


static void free_movie(void *_object)
{
  if (!THIS->movie)
    return;
  
  delete THIS->movie;
  THIS->movie = 0;
  THIS->ba->resetRawData((const char *)THIS->addr, THIS->len);  
  delete THIS->ba;
  
  GB.ReleaseFile(&THIS->addr, THIS->len);
  
  GB.StoreString(NULL, &THIS->path);
}

static bool load_movie(void *_object, char *path, long len)
{
  free_movie(THIS);
  
  //qDebug("load_movie: %.*s", (int)len, path);
  if (GB.LoadFile(path, len, &THIS->addr, &THIS->len))
    return true;

  THIS->ba = new QByteArray();    
  THIS->ba->setRawData((const char *)THIS->addr, THIS->len);
  THIS->movie = new QMovie(*(THIS->ba));
  
  GB.NewString(&THIS->path, path, len);
  
  //qDebug("setMovie");
  WIDGET->setMovie(*THIS->movie);
  
  return false;
}


BEGIN_METHOD(CMOVIEBOX_new, GB_OBJECT parent)

  QLabel *wid = new QLabel(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, _object);

  wid->show();

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
      playing = THIS->movie->running();
    else
      playing = FALSE;

    if (load_movie(THIS, PSTRING(), PLENGTH()))
      return;
          
    if (!playing)
      THIS->movie->pause();
  }

END_PROPERTY


BEGIN_PROPERTY(CMOVIEBOX_playing)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->movie ? THIS->movie->running() : FALSE);
  else if (THIS->movie)
  {
    if (VPROP(GB_BOOLEAN))
      THIS->movie->unpause();
    else
      THIS->movie->pause();
  }

END_PROPERTY


BEGIN_METHOD_VOID(CMOVIEBOX_rewind)

  if (!THIS->movie)
    return;
    
  THIS->movie->restart();

END_METHOD


GB_DESC CMovieBoxDesc[] =
{
  GB_DECLARE("MovieBox", sizeof(CMOVIEBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CMOVIEBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CMOVIEBOX_free, NULL),

  GB_PROPERTY("Path", "s", CMOVIEBOX_path),
  GB_PROPERTY("Playing", "b", CMOVIEBOX_playing),
  //GB_PROPERTY("Alignment", "i<Align>", CMOVIEBOX_alignment),
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  
  GB_METHOD("Rewind", NULL, CMOVIEBOX_rewind, NULL),

  GB_CONSTANT("_Properties", "s", "*,Path,Playing,Border{Border.*}"),

  GB_END_DECLARE
};

