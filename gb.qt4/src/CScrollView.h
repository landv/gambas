/***************************************************************************

  CScrollView.h

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

#ifndef __CSCROLLVIEW_H
#define __CSCROLLVIEW_H

#include "gambas.h"

#include <qevent.h>
#include <q3scrollview.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QShowEvent>
#include <QChildEvent>

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CSCROLLVIEW_CPP
extern GB_DESC CScrollViewDesc[];
#else

#define THIS    ((CSCROLLVIEW *)_object)
#define WIDGET  ((MyScrollView *)((CWIDGET *)_object)->widget)

#endif

class MyContents;

typedef
  struct {
    CWIDGET widget;
    MyContents *container;
    CARRANGEMENT arrangement;
    //unsigned locked : 1;
    }
  CSCROLLVIEW;


class MyScrollView : public Q3ScrollView
{
  Q_OBJECT

public:

  MyScrollView(QWidget *);

protected:

  void frameChanged();
  void resizeEvent(QResizeEvent *);
  void showEvent(QShowEvent *);
};


class MyContents : public MyContainer
{
  Q_OBJECT

public:

  MyContents(QWidget *parent, MyScrollView *scrollview);
  
public slots:

  void autoResize(void);

protected:

  void childEvent(QChildEvent *);

  bool eventFilter(QObject *, QEvent *);

private:

  void findRightBottom(void);
  void checkWidget(QWidget *);

  QWidget *right;
  QWidget *bottom;
  MyScrollView *sw;
  bool timer;
};

class CScrollView : public QObject
{
  Q_OBJECT

public:

  static CScrollView manager;

protected:

  bool eventFilter(QObject *, QEvent *);

public slots:

  void scrolled(void);
};

#endif
