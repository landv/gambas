/***************************************************************************

  CTabStrip.h

  The Tab Strip class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#ifndef __CTABSTRIP_H
#define __CTABSTRIP_H

#include "gambas.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"

#include <qframe.h>
#include <qptrlist.h>
#include <qtabwidget.h>
#include <qtabbar.h>

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
    long index;
    long child;
    bool init;
    }
  CTABSTRIP_ENUM;

class CTab;

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    CARRANGEMENT arrangement;
    QPtrList<CTab> *stack;
    long index;
    long id;
    unsigned geom : 1;
    unsigned lock : 1;
    }
  CTABSTRIP;

#endif

class MyTabWidget : public QTabWidget
{
Q_OBJECT

public:

	MyTabWidget(QWidget *parent);
	virtual void setEnabled(bool e);
	void forceLayout();
	
protected:

  virtual bool eventFilter(QObject *, QEvent *);
};

class CTabStrip : public QObject
{
Q_OBJECT

public:

  static CTabStrip manager;

public slots:

  void currentChanged(QWidget *);
};

#endif
