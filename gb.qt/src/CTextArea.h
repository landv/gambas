/***************************************************************************

  CTextArea.h

  The TextArea class

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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
//extern GB_DESC CTextAreaLinesDesc[];
extern GB_DESC CTextAreaSelectionDesc[];
#else

#define WIDGET ((QTextEdit *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CTEXTAREA;


/*
class MyMultiLineEdit : public QtMultiLineEdit
{
  Q_OBJECT

public:

  MyMultiLineEdit(QWidget *wid) : QtMultiLineEdit(wid) {};
  long getLineLength(long line);
  void fromPos(long pos, int *row, int *col);
  long toPos(int row, int col);
  void getSelection(long *start, long *length);
  void setSelection(long start, long length);
  const char *selText(void);
};
*/

class CTextArea : public QObject
{
  Q_OBJECT

public:

  static CTextArea manager;

public slots:

  void changed(void);
  void cursor(void);

};


#endif
