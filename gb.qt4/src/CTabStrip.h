/***************************************************************************

  CTabStrip.h

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

#ifndef __CTABSTRIP_H
#define __CTABSTRIP_H

#include "gambas.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"

#include <QTabWidget>
#include <QTabBar>
#include <QEvent>

#ifndef __CTABSTRIP_CPP
extern GB_DESC CTabStripDesc[];
extern GB_DESC CTabDesc[];
extern GB_DESC CTabChildrenDesc[];
#else

#define QTABWIDGET(object) ((MyTabWidget *)((CWIDGET *)object)->widget)

#define WIDGET QTABWIDGET(_object)
#define THIS OBJECT(CTABSTRIP)

typedef
  struct {
    int index;
    int child;
    bool init;
    }
  CTABSTRIP_ENUM;

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    CARRANGEMENT arrangement;
    int index;
    int id;
    unsigned geom : 1;
    unsigned lock : 1;
    }
  CTABSTRIP;

#endif

class CTab;

class MyTabWidget : public QTabWidget
{
Q_OBJECT

public:

  QList<CTab *> stack;

	MyTabWidget(QWidget *parent);
	virtual ~MyTabWidget();
	virtual void setEnabled(bool e);
	void layoutContainer();
};

class CTabStrip : public QObject
{
Q_OBJECT

public:

  static CTabStrip manager;

public slots:

  void currentChanged(int);
};

void CTABSTRIP_arrange(void *_object);

#endif
