/***************************************************************************

  CListBox.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CLISTBOX_H
#define __CLISTBOX_H

#include <QResizeEvent>
#include <QMouseEvent>
#include <QListWidget>

#include "gambas.h"

#include "CWidget.h"

#ifndef __CLISTBOX_CPP
extern GB_DESC CListBoxDesc[];
extern GB_DESC CListBoxItemDesc[];
#else

#define THIS   ((CLISTBOX *)_object)
#define WIDGET ((QListWidget *)((CWIDGET *)_object)->widget)

#define CUSTOM_RTTI 1000

#endif

typedef
  struct {
    CWIDGET widget;
    int index;
    int last;
		bool posted;
    }
  CLISTBOX;


/*class MyListBox : public QListWidget
{
  Q_OBJECT

public:

  MyListBox(QWidget *parent);
  virtual void setCurrentItem(QListBoxItem *);

protected:

  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent *);

};*/

class CListBox : public QObject
{
  Q_OBJECT

public:

  static CListBox manager;

  static void getAll(QListWidget *list, GB_ARRAY array);
  static void setAll(QListWidget *list, GB_ARRAY array);
  static int find(QListWidget *list, const QString& elt);
  //static int currentItem(QListWidget *list);

public slots:

  void selected(void);
  void activated(QListWidgetItem *);
  //void highlighted(int);
  void clicked(QListWidgetItem *);

};

#endif
