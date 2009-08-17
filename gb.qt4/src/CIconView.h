/***************************************************************************

  CIconView.h

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

#ifndef __CICONVIEW_H
#define __CICONVIEW_H

#include "gambas.h"

#include <q3asciidict.h>
#include <q3iconview.h>
#include <qevent.h>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CICONVIEW_CPP
extern GB_DESC CIconViewItemDesc[];
extern GB_DESC CIconViewDesc[];
#else

#define WIDGET ((MyIconView *)((CWIDGET *)_object)->widget)
#define THIS ((CICONVIEW *)_object)

#define ARRANGEMENT_FREE (-1)

#endif

class MyIconViewItem;

typedef
  struct {
    CWIDGET widget;
    Q3AsciiDict<MyIconViewItem> *dict;
    MyIconViewItem *item;
    MyIconViewItem *save;
    char compare;
    unsigned sorted : 1;
    unsigned asc : 1;
    unsigned editable : 1;
    }
  CICONVIEW;


class MyIconViewItem : public Q3IconViewItem
{
public:

  MyIconViewItem(Q3IconView *parent);
  MyIconViewItem(Q3IconView *parent, MyIconViewItem *after);
  ~MyIconViewItem();

  CPICTURE *picture;
  char *key;
  CICONVIEW *container;

  void setPicture(GB_OBJECT *pict);
  virtual int compare(Q3IconViewItem *i) const;

private:

  void initData(void);
};


class MyIconView : public Q3IconView
{
public:

  MyIconView(QWidget* parent);
  ~MyIconView() { };
  
  void setArrangementMode(int arr);
  int getArrangementMode();
  void setGridWidth(int w);
  int gridWidth() { return _grid_width; }

protected:

  virtual void startDrag();
  
private:

	int _grid_width;
};


class CIconView : public QObject
{
  Q_OBJECT

public:

  static CIconView manager;
  static MyIconViewItem *getItem(CICONVIEW *view, char *key);

public slots:

  void selected(void);
  void activated(Q3IconViewItem *);
  void clicked(Q3IconViewItem *);
  void renamed(Q3IconViewItem *);

private:

  void raiseEvent(int, Q3IconViewItem *);
};

#endif
