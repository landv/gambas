/***************************************************************************

  CDial.h

  The Dial class

  Hacked together by Nigel Gerrard using code provided by

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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
#ifndef __CDIAL_H
#define __CDIAL_H

#include "gambas.h"
#include "../gb.qt.h"

#ifndef __CDIAL_CPP

extern GB_DESC CDialDesc[];

#else

#define THIS    ((CDIAL *)_object)
#define WIDGET  ((QDial *)((QT_WIDGET *)_object)->widget)

#endif

typedef
  struct {
    QT_WIDGET widget;
    }
  CDIAL;

class CDial : public QObject
{
  Q_OBJECT

public:

  static CDial manager;

public slots:

  void event_change(void);
};

#endif
