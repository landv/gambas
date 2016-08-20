/***************************************************************************

  CScrollBar.h

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

#ifndef __CSCROLLBAR_H
#define __CSCROLLBAR_H

#include <qscrollbar.h>
//Added by qt3to4:
#include <QResizeEvent>
#include "gb.qt.h"
#include "gambas.h"

#ifndef __CSCROLLBAR_CPP

extern GB_DESC CScrollBarDesc[];

#else

#define THIS    ((CSCROLLBAR *)_object)
#define WIDGET  ((QScrollBar *)((QT_WIDGET *)_object)->widget)

#endif


typedef
  struct {
    QT_WIDGET widget;
    }
  CSCROLLBAR;


class MyScrollBar : public QScrollBar
{
  Q_OBJECT

public:

  MyScrollBar(QWidget *);

protected:

  void resizeEvent(QResizeEvent *);
};


class CScrollBar : public QObject
{
  Q_OBJECT

public:

  static CScrollBar manager;

public slots:

  void event_change(void);

};

#endif
