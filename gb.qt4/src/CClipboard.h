/***************************************************************************

  CClipboard.h

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

#ifndef __CCLIPBOARD_H
#define __CCLIPBOARD_H

#include "gambas.h"
#include "CWidget.h"
#include "CPicture.h"
#include <QMimeData>
#include <QDropEvent>

typedef
  struct {
    const QDropEvent *event;
    int x;
    int y;
    unsigned valid : 1;
    }
  CDRAG_INFO;

#ifndef __CCLIPBOARD_CPP
extern GB_DESC CClipboardDesc[];
extern GB_DESC CDragDesc[];
extern CDRAG_INFO CDRAG_info;
extern bool CDRAG_dragging;
#endif

class MyDragFrame: public QWidget
{
  Q_OBJECT

public:

  MyDragFrame(QWidget *);

protected:

	//virtual paintEvent(QPaintEvent *e);
};

void CDRAG_clear(bool valid);
void *CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, GB_STRING *fmt);
bool CDRAG_drag_enter(QWidget *w, CWIDGET *control, QDropEvent *e);
void CDRAG_drag_leave(CWIDGET *control);
bool CDRAG_drag_move(QWidget *w, CWIDGET *control, QDropEvent *e);
bool CDRAG_drag_drop(QWidget *w, CWIDGET *control, QDropEvent *e);
void CDRAG_hide_frame(CWIDGET *control);

void CLIPBOARD_has_changed();

#endif
