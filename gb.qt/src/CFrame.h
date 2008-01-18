/***************************************************************************

  CFrame.h

  The Frame class

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

#ifndef __CFRAME_H
#define __CFRAME_H

#include "gambas.h"

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CFRAME_CPP
extern GB_DESC CFrameDesc[];
#else

#define QGROUPBOX(object) ((QGroupBox *)((CWIDGET *)object)->widget)
#define WIDGET QGROUPBOX(_object)

#endif

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    int arrangement;
    }
  CFRAME;

#endif
