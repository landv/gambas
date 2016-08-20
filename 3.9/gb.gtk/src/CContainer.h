/***************************************************************************

  CContainer.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CCONTAINER_H
#define __CCONTAINER_H


#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CWidget.h"

#ifndef __CCONTAINER_CPP

extern GB_DESC CChildrenDesc[];
extern GB_DESC CContainerDesc[];
extern GB_DESC CUserControlDesc[];
extern GB_DESC CUserContainerDesc[];

#else

#define THIS ((CCONTAINER *)_object)
#define THIS_UC ((CUSERCONTROL *)_object)
#define WIDGET ((gContainer*)THIS->ob.widget)
#define PANEL ((gPanel *)(THIS->ob.widget))

#define THIS_CONT (THIS_UC->container)
#define WIDGET_CONT ((gContainer *)THIS_UC->container->ob.widget)

#endif


typedef 
  struct
  {
  	CWIDGET ob;
  }  
  CCONTAINER;

typedef  
	struct
  {
    CWIDGET widget;
    CCONTAINER *container;
    gContainerArrangement save;
  }
  CUSERCONTROL;


DECLARE_PROPERTY(Container_Arrangement);
DECLARE_PROPERTY(Container_AutoResize);
DECLARE_PROPERTY(Container_Padding);
DECLARE_PROPERTY(Container_Spacing);
DECLARE_PROPERTY(Container_Margin);
DECLARE_PROPERTY(Container_Indent);
DECLARE_PROPERTY(Container_Invert);

void CCONTAINER_cb_arrange(gContainer *sender);
void CCONTAINER_cb_before_arrange(gContainer *sender);
void CCONTAINER_raise_insert(CCONTAINER *_object, CWIDGET *child);

#endif
