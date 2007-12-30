/***************************************************************************

  CTextView.h

  The TextView class

  Hacked together by Fabien Bodard ;-) using code provided by

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

#ifndef __CTEXTVIEW_H
#define __CTEXTVIEW_H

#include "gambas.h"
#include "../gb.qt.h"

#ifndef __CTEXTVIEW_CPP

extern GB_DESC CTextViewDesc[];

#else

#define THIS    ((CTEXTVIEW *)_object)
#define WIDGET  ((QTextBrowser *)((QT_WIDGET *)_object)->widget)

#endif

typedef
  struct {
    QT_WIDGET widget;
    bool change;
    }
  CTEXTVIEW;

class CTextView : public QObject
{
  Q_OBJECT

public:

  static CTextView manager;

public slots:

  void event_link(const QString &);

};

#endif
