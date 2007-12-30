/***************************************************************************

  CPanel.h

  The Panel class

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

#ifndef __CPANEL_H
#define __CPANEL_H

#include "gambas.h"

#include <qframe.h>

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CPANEL_CPP
extern GB_DESC CPanelDesc[];
extern GB_DESC CHBoxDesc[];
extern GB_DESC CVBoxDesc[];
extern GB_DESC CHPanelDesc[];
extern GB_DESC CVPanelDesc[];
#else

#define THIS  ((CPANEL *)_object)
#define WIDGET ((MyPanel *)((CWIDGET *)_object)->widget)

#define CPANEL_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Border"
#define CHBOX_PROPERTIES "*,AutoResize," CPADDING_PROPERTIES
#define CVBOX_PROPERTIES CHBOX_PROPERTIES

#endif

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    CARRANGEMENT arrangement;
    }
  CPANEL;

#endif
