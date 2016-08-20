/***************************************************************************

  CSpinBox.h

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

#ifndef __CSPINBOX_H
#define __CSPINBOX_H

#include "gambas.h"
#include "gb.qt.h"
#include "CWidget.h"

//#include <QSpinBox>

#ifndef __CSPINBOX_CPP

extern GB_DESC CSpinBoxDesc[];

#else

#define THIS    ((CSPINBOX *)_object)
#define WIDGET  ((QSpinBox *)((QT_WIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CSPINBOX;

/*class MySpinBox : public QSpinBox
{
public:
  MySpinBox(QWidget *parent = 0);
  ~MySpinBox();
};*/
	
class CSpinBox : public QObject
{
  Q_OBJECT

public:

  static CSpinBox manager;

public slots:

  void event_change(void);

};

#endif
