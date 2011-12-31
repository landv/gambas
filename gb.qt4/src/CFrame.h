/***************************************************************************

  CFrame.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
#define THIS ((CFRAME *)_object)

#endif

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    int arrangement;
    }
  CFRAME;

#endif
