/***************************************************************************

  CColorBox.h

  The KDE Color Combo Box

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

#ifndef __CCOLORBOX_H
#define __CCOLORBOX_H

#include "gambas.h"
#include "../gb.qt.h"

#ifndef __CCOLORBOX_CPP

extern GB_DESC CColorBoxDesc[];

#else

#define THIS    ((CCOLORBOX *)_object)
#define WIDGET  ((KColorCombo *)((QT_WIDGET *)_object)->widget)

#define CCOLORBOX_PROPERTIES QT_WIDGET_PROPERTIES ",Color"

#endif

typedef
  struct {
    QT_WIDGET widget;
    }
  CCOLORBOX;

class CColorBox : public QObject
{
  Q_OBJECT

public:

  static CColorBox manager;

public slots:

  void clicked();

};

#endif
