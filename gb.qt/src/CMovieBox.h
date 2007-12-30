/***************************************************************************

  CMovieBox.h

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

#ifndef __CMOVIEBOX_H
#define __CMOVIEBOX_H

#include "gambas.h"
#include "gb.qt.h"
#include <qlabel.h>


#ifndef __CMOVIEBOX_CPP
extern GB_DESC CMovieBoxDesc[];
#else

#define THIS    ((CMOVIEBOX *)_object)
#define WIDGET  ((QLabel *)((QT_WIDGET *)_object)->widget)

#endif

typedef
  struct {
    QT_WIDGET widget;
    char *path;
    QMovie *movie;
    char *addr;
    long len;
    QByteArray *ba;
    }
  CMOVIEBOX;

/*
GAMBAS_METHOD CBUTTON_new;

GAMBAS_PROPERTY CBUTTON_set_text, CBUTTON_get_text;
*/

#endif
