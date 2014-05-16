/***************************************************************************

  CFrame.h

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

#ifndef __CFRAME_H
#define __CFRAME_H

#include "gambas.h"

#include "CWidget.h"
#include "CContainer.h"

#include <QGroupBox>

#ifndef __CFRAME_CPP
extern GB_DESC CFrameDesc[];
#else

#define THIS ((CFRAME *)_object)
#define WIDGET ((MyGroupBox *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    CARRANGEMENT arrangement;
    }
  CFRAME;

class MyGroupBox : public QGroupBox
{
  Q_OBJECT

public:

	MyGroupBox(QWidget *parent);
	void updateInside();

protected:

  virtual void changeEvent(QEvent *e);
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);
};


#endif
