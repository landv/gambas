/***************************************************************************

  CEditor.h

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

#ifndef __CEDITOR_H
#define __CEDITOR_H

#include "gambas.h"
#include "../gb.qt.h"

#include "gdocument.h"
#include "gview.h"

#ifndef __CEDITOR_CPP

extern GB_DESC CHighlightDesc[];
extern GB_DESC CEditorLineDesc[];
extern GB_DESC CEditorLinesDesc[];
extern GB_DESC CEditorSelectionDesc[];
extern GB_DESC CEditorStyleDesc[];
extern GB_DESC CEditorStylesDesc[];
extern GB_DESC CEditorFlagsDesc[];
extern GB_DESC CEditorDesc[];

#else

#define THIS      ((CEDITOR *)_object)
#define WIDGET    ((GEditor *)((QT_WIDGET *)_object)->widget)
#define DOC       (WIDGET->getDocument())
#define MANAGER   &CEditor::manager

#endif

typedef
  struct {
    QT_WIDGET widget;
    void *view;
    int line;
		GB_FUNCTION highlight;
    }
  CEDITOR;

class CEditor : public QObject
{
  Q_OBJECT

public:

  static CEditor manager;

public slots:

  void changed(void);
  void moved(void);
  void scrolled(int, int);
  //void marginClicked(int);
  void marginDoubleClicked(int);
};

#endif
