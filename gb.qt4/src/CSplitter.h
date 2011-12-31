/***************************************************************************

  CSplitter.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CSPLITTER_H
#define __CSPLITTER_H

#include "gambas.h"
#include "gb.qt.h"
#include "CWidget.h"

#include <QSplitter>
#include <QEvent>
#include <QResizeEvent>

#ifndef __CSPLITTER_CPP
extern GB_DESC CHSplitDesc[];
extern GB_DESC CVSplitDesc[];
#else

#define QFRAME(object)
#define WIDGET ((MySplitter *)((QT_WIDGET *)_object)->widget)
#define THIS ((CSPLITTER *)_object)

#endif

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    }
  CSPLITTER;

class MySplitter : public QSplitter
{
  Q_OBJECT

public:

  MySplitter(QWidget *parent);

  bool _event;
  
  int handleCount();
	
public slots:
	
	void resizeSlot();

protected:

	void resizeEvent(QResizeEvent *);
  bool eventFilter(QObject *, QEvent *);

};


#endif
