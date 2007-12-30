/***************************************************************************

  CButton.h

  The Button and ToggleButton class

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

#ifndef __CBUTTON_H
#define __CBUTTON_H

#include <qsize.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qevent.h>

#include "gambas.h"
#include "CWidget.h"
#include "CPicture.h"
#include "CWindow.h"

#ifndef __CBUTTON_CPP
extern GB_DESC CButtonDesc[];
extern GB_DESC CToggleButtonDesc[];
extern GB_DESC CToolButtonDesc[];
#else

#define QBUTTON(object) ((QButton *)((CWIDGET *)object)->widget)
#define QPUSHBUTTON(object) ((MyPushButton *)((CWIDGET *)object)->widget)
#define QTOOLBUTTON(object) ((MyToolButton *)((CWIDGET *)object)->widget)

#define THIS OBJECT(CBUTTON)
#define WIDGET QPUSHBUTTON(_object)
#define WIDGET_TOOL QTOOLBUTTON(_object)

#endif

typedef
  struct {
    CWIDGET widget;
    CPICTURE *picture;
	  int last_size;
    unsigned radio : 1;
    unsigned stretch : 1;
    }
  CBUTTON;

class MyPushButton : public QPushButton
{
  Q_OBJECT

public:

  MyPushButton(QWidget *parent);
  ~MyPushButton();
  //QSize sizeHint(void) const;
  virtual void fontChange(const QFont &);
  void calcMinimumHeight();
  virtual void resizeEvent(QResizeEvent *e);

  CWINDOW *top;
};


class MyToolButton : public QToolButton
{
  Q_OBJECT

public:

  MyToolButton(QWidget *parent);
  ~MyToolButton();
  //QSize sizeHint(void) const;
  virtual void fontChange(const QFont &);
  void calcMinimumHeight();
  virtual void resizeEvent(QResizeEvent *e);
};


class CButton : public QObject
{
  Q_OBJECT

public:

  static CButton manager;

  static void onlyMe(CBUTTON *);

public slots:

  void clicked(void);
  void clickedToggle(void);
  void clickedTool(void);

};


#endif
