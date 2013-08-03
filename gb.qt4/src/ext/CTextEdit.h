/***************************************************************************

  CTextEdit.h

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

#ifndef __CTEXTEDIT_H
#define __CTEXTEDIT_H

#include "gambas.h"
#include "../gb.qt.h"

#include <QTextEdit>

#ifndef __CTEXTEDIT_CPP
extern GB_DESC CTextEditSelectionDesc[];
extern GB_DESC CTextEditFormatDesc[];
extern GB_DESC CTextEditDesc[];
#else

#define WIDGET       ((QTextEdit *)((QT_WIDGET *)_object)->widget)
#define MYTEXTEDIT   ((MyTextEdit *)((QT_WIDGET *)_object)->widget)
#define THIS         ((CTEXTEDIT *)_object)

#endif

typedef
  struct {
    QT_WIDGET widget;
		int length;
    bool change;
    }
  CTEXTEDIT;

class MyTextEdit: public QTextEdit
{
  Q_OBJECT
  
public:
  MyTextEdit(QWidget *parent = 0);
  ~MyTextEdit();
  
signals:
  //void highlighted(const QString&);
  void linkClicked(const QString&);

private:
  //void popupDetail( const QString& contents, const QPoint& pos );
  bool linksEnabled() const { return true; }
  //void emitHighlighted( const QString &s );
  void emitLinkClicked( const QString &s );
};

class CTextArea : public QObject
{
  Q_OBJECT

public:

  static CTextArea manager;

public slots:

  void changed(void);
  void cursor(void);
  //void link(const QString &);
};

#endif
