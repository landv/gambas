/***************************************************************************

  CListBox.h

  The ListBox class

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

#ifndef __CLISTBOX_H
#define __CLISTBOX_H

#include <q3listbox.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QMouseEvent>

#include "gambas.h"

#include "CWidget.h"

#ifndef __CLISTBOX_CPP
extern GB_DESC CListBoxDesc[];
extern GB_DESC CListBoxItemDesc[];
#else

#define QLISTBOX(object) ((Q3ListBox *)((CWIDGET *)object)->widget)

#define THIS   ((CLISTBOX *)_object)
#define WIDGET ((Q3ListBox *)((CWIDGET *)_object)->widget)

#define CUSTOM_RTTI 1000

#endif

typedef
  struct {
    CWIDGET widget;
    int index;
    bool sorted;
    int last;
    }
  CLISTBOX;

#if 0
class MyListBoxItem : public Q3ListBoxItem
{
public:
  MyListBoxItem(Q3ListBox* listbox, QString& text, int width, int height);
  ~MyListBoxItem();

  int height(const Q3ListBox *) const;
  int width(const Q3ListBox *)  const;

  int rtti() const;
  static int RTTI;

protected:
    void paint(QPainter *);

private:
  int w, h;
  //MyListBoxItem( const MyListBoxItem &);
  //MyListBoxItem &operator=(const MyListBoxItem &);
};
#endif

class MyListBox : public Q3ListBox
{
  Q_OBJECT

public:

  MyListBox(QWidget *parent);
  virtual void setCurrentItem(Q3ListBoxItem *);

protected:

  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent *);

};

class CListBox : public QObject
{
  Q_OBJECT

public:

  static CListBox manager;

  static void getAll(Q3ListBox *list, GB_ARRAY array);
  static void setAll(Q3ListBox *list, GB_ARRAY array);
  static int find(Q3ListBox *list, const QString& elt);
  static int currentItem(Q3ListBox *list);

public slots:

  void selected(void);
  void activated(int);
  void highlighted(int);
  void clicked(Q3ListBoxItem *);

};

#endif
