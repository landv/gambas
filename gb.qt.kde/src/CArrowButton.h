/***************************************************************************

  CArrowButton.h

  The KDE Arrow Button

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CARROWBUTTON_H
#define __CARROWBUTTON_H

#include "gambas.h"
#include "../gb.qt.h"

#ifndef __CARROWBUTTON_CPP

extern GB_DESC CArrowButtonDesc[];

#else

#define THIS    ((CARROWBUTTON *)_object)
#define WIDGET  ((KArrowButton *)((QT_WIDGET *)_object)->widget)

#define CARROWBUTTON_PROPERTIES QT_WIDGET_PROPERTIES ",Text,Orientation,Border"

#endif

typedef
  struct {
    QT_WIDGET widget;
    int orient;
    }
  CARROWBUTTON;

class CArrowButton : public QObject
{
  Q_OBJECT

public:

  static CArrowButton manager;

public slots:

  void clicked();

};

#endif
