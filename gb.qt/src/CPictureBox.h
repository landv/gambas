/***************************************************************************

  CPictureBox.h

  The PictureBox class

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

#ifndef __CPICTUREBOX_H
#define __CPICTUREBOX_H

#include "gambas.h"

#include <qlabel.h>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CPICTUREBOX_CPP
extern GB_DESC CPictureBoxDesc[];
#else

#define QLABEL(object) ((QLabel *)((CWIDGET *)object)->widget)

#define WIDGET ((QLabel *)((CWIDGET *)_object)->widget)
#define THIS ((CPICTUREBOX *)_object)

#endif

typedef
  struct {
    CWIDGET widget;
    CPICTURE *picture;
    }
  CPICTUREBOX;

/*
class MyLabel : public QLabel
{
  Q_OBJECT

public:

  MyLabel(QWidget *parent);
  ~MyLabel();

protected:

  void resizeEvent( QResizeEvent* e );
};
*/


#endif
