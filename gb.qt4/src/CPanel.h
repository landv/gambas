/***************************************************************************

  CPanel.h

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

#ifndef __CPANEL_H
#define __CPANEL_H

#include "gambas.h"

#include <QFrame>

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CPANEL_CPP
extern GB_DESC CPanelDesc[];
extern GB_DESC CHBoxDesc[];
extern GB_DESC CVBoxDesc[];
extern GB_DESC CHPanelDesc[];
extern GB_DESC CVPanelDesc[];
#else

#define THIS  ((CPANEL *)_object)
#define WIDGET ((MyPanel *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    CARRANGEMENT arrangement;
    }
  CPANEL;

// class MyPanel: public QFrame
// {
// 	Q_OBJECT
// 
// public:
// 	MyPanel(QWidget *parent);
// 	void configure();
// 	
// protected:
// 	virtual void resizeEvent(QResizeEvent *);
// };

#endif
