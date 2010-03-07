/***************************************************************************

  CTextBox.cpp

  The TextBox class

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

#ifndef __CTEXTBOX_H
#define __CTEXTBOX_H

#include "gambas.h"

#include <qcombobox.h>

#include "CWidget.h"

#ifndef __CTEXTBOX_CPP

extern GB_DESC CTextBoxSelectionDesc[];
extern GB_DESC CTextBoxDesc[];
extern GB_DESC CComboBoxDesc[];
extern GB_DESC CComboBoxItemDesc[];

#else

#define QLINEEDIT(object) ((QLineEdit *)((CWIDGET *)object)->widget)

#define TEXTBOX ((QLineEdit *)((CWIDGET *)_object)->widget)
#define COMBOBOX ((MyComboBox *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CTEXTBOX;

typedef
  struct {
    CWIDGET widget;
    int index;
    bool sorted;
		//bool click;
    }
  CCOMBOBOX;


class MyComboBox : public QComboBox
{
  Q_OBJECT

public:

  MyComboBox(QWidget *parent);
  void fontChange(const QFont &);
  void calcMinimumHeight();
};


class CTextBox : public QObject
{
  Q_OBJECT

public:

  static CTextBox manager;

public slots:

  void event_change(void);
  void event_activate(void);
  void event_click(void);

};


#endif
