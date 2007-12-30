/***************************************************************************

  CRadioButton.h

  The RadioButton class

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

#ifndef __CRADIOBUTTON_H
#define __CRADIOBUTTON_H

#include "gambas.h"

#include "CWidget.h"

#ifndef __CRADIOBUTTON_CPP
extern GB_DESC CRadioButtonDesc[];
#else

#define QRADIOBUTTON(object) ((QRadioButton *)((CWIDGET *)object)->widget)
#define THIS ((CRADIOBUTTN *)_object)
#define WIDGET QRADIOBUTTON(_object)

#define CRADIOBUTTON_PROPERTIES "*,Text,Value"

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CRADIOBUTTON;

class CRadioButton : public QObject
{
  Q_OBJECT

public:

  static CRadioButton manager;

public slots:

  void clicked(bool on);

};

#endif
