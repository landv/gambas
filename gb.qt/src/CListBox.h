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

#include <qlistbox.h>

#include "gambas.h"

#include "CWidget.h"

#ifndef __CLISTBOX_CPP
extern GB_DESC CListBoxDesc[];
extern GB_DESC CListBoxItemDesc[];
#else

#define QLISTBOX(object) ((QListBox *)((CWIDGET *)object)->widget)

#define THIS   ((CLISTBOX *)_object)
#define WIDGET ((QListBox *)((CWIDGET *)_object)->widget)

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
class MyListBoxItem : public QListBoxItem
{
public:
  MyListBoxItem(QListBox* listbox, QString& text, int width, int height);
  ~MyListBoxItem();

  int height(const QListBox *) const;
  int width(const QListBox *)  const;

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

class MyListBox : public QListBox
{
  Q_OBJECT

public:

  MyListBox(QWidget *parent);
  virtual void setCurrentItem(QListBoxItem *);

protected:

  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent *);

};

class CListBox : public QObject
{
  Q_OBJECT

public:

  static CListBox manager;

  static void getAll(QListBox *list, GB_ARRAY array);
  static void setAll(QListBox *list, GB_ARRAY array);
  static int find(QListBox *list, const QString& elt);
  static int currentItem(QListBox *list);

public slots:

  void selected(void);
  void activated(int);
  void highlighted(int);
  void clicked(QListBoxItem *);

};

#endif
