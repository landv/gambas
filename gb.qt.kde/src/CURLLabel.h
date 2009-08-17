/***************************************************************************

  CURLLabel.h

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

#ifndef __CURLLABEL_H
#define __CURLLABEL_H

#include "gambas.h"
#include "../gb.qt.h"

#ifndef __CURLLABEL_CPP

extern GB_DESC CURLLabelDesc[];

#else

#define THIS    ((CURLLABEL *)_object)
#define WIDGET  ((KURLLabel *)((QT_WIDGET *)_object)->widget)

#define CURLLABEL_PROPERTIES QT_WIDGET_PROPERTIES ",Text,URL"

#endif

typedef
  struct {
    QT_WIDGET widget;
    }
  CURLLABEL;

class CUrlLabel : public QObject
{
  Q_OBJECT

public:

  static CUrlLabel manager;

public slots:

  void clicked();

};

#endif
