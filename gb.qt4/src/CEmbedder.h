/***************************************************************************

  CEmbedder.h

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

#ifndef __CEMBEDDER_H
#define __CEMBEDDER_H

#include "gambas.h"
#include "CWidget.h"

#ifndef __CEMBEDDER_CPP
extern GB_DESC CEmbedderDesc[];
#else

typedef
  struct {
    CWIDGET widget;
    }
  CEMBEDDER;

#define THIS  ((CEMBEDDER *)_object)
#endif

#ifndef NO_X_WINDOW

#include <QX11EmbedWidget>
#include <QX11EmbedContainer>

#define WIDGET ((QX11EmbedContainer *)((CWIDGET *)_object)->widget)

class CEmbedder : public QObject
{
  Q_OBJECT

public:

  static CEmbedder manager;

public slots:

  void error(void);
  void embedded(void);
  void closed(void);

};
#endif

#endif
