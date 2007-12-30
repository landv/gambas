/***************************************************************************

  CTextArea.h

  The TextArea class

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

#ifndef __CTEXTAREA_H
#define __CTEXTAREA_H

#include "gambas.h"

#include "CWidget.h"

#include <qtextedit.h>

#ifndef __CTEXTAREA_CPP
extern GB_DESC CTextAreaDesc[];
extern GB_DESC CTextAreaSelectionDesc[];
extern GB_DESC CTextEditSelectionDesc[];
extern GB_DESC CTextEditFormatDesc[];
extern GB_DESC CTextEditDesc[];
#else

#define WIDGET       ((QTextEdit *)((CWIDGET *)_object)->widget)
#define MYTEXTEDIT   ((MyTextEdit *)((CWIDGET *)_object)->widget)
#define THIS_AREA    ((CTEXTAREA *)_object)
#define THIS_EDIT    ((CTEXTEDIT *)_object)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CTEXTAREA;

typedef
  struct {
    CWIDGET widget;
    bool change;
    }
  CTEXTEDIT;

class MyTextEdit: public QTextEdit
{
  Q_OBJECT
  
public:
  MyTextEdit(QWidget *parent = 0, const char *name = 0);
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
  void link(const QString &);
};

#endif
