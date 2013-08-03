/***************************************************************************

  CProgress.h

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

#ifndef __CPROGRESS_H
#define __CPROGRESS_H

#include <QWindowsStyle>

#include "gambas.h"

#include "CWidget.h"

#ifndef __CPROGRESS_CPP
extern GB_DESC CProgressDesc[];
#else

#define THIS    ((CPROGRESS *)_object)
#define WIDGET  ((QProgressBar *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CPROGRESS;

class MyWindowsStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    //MyCleanlooksStyle();
    //~MyCleanlooksStyle();

protected:
    void timerEvent(QTimerEvent *event);
};

void CPROGRESS_style_hack(void *_object);

#endif
