/***************************************************************************

  CCheckBox.h

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

#ifndef __CCHECKBOX_H
#define __CCHECKBOX_H

#include "gambas.h"

#include "CWidget.h"

#ifndef __CCHECKBOX_CPP
extern GB_DESC CCheckBoxDesc[];
#else

#define QCHECKBOX(object) ((QCheckBox *)((CWIDGET *)object)->widget)

#define THIS ((CCHECKBOX *)_object)
#define WIDGET QCHECKBOX(_object)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CCHECKBOX;

class CCheckBox : public QObject
{
  Q_OBJECT

public:

  static CCheckBox manager;

public slots:

  void clicked(void);

};

#endif
