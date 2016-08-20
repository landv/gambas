/***************************************************************************

  CTextArea.h

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

#ifndef __CTEXTAREA_H
#define __CTEXTAREA_H

#include "gambas.h"

#include "CWidget.h"

#include <QPlainTextEdit>

#ifndef __CTEXTAREA_CPP
extern GB_DESC CTextAreaDesc[];
extern GB_DESC CTextAreaSelectionDesc[];
#else

#define WIDGET       ((QTextEdit *)((CWIDGET *)_object)->widget)
#define MYTEXTEDIT   ((MyTextEdit *)((CWIDGET *)_object)->widget)
#define THIS         ((CTEXTAREA *)_object)

#endif

typedef
  struct {
    CWIDGET widget;
		int length;
		int align;
		unsigned no_change : 1;
    }
  CTEXTAREA;

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

void CTEXTAREA_set_foreground(void *_object);

#endif
