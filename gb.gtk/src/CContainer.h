/***************************************************************************

  CContainer.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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


DECLARE_PROPERTY(CCONTAINER_arrangement);
DECLARE_PROPERTY(CCONTAINER_auto_resize);
DECLARE_PROPERTY(CCONTAINER_padding);
DECLARE_PROPERTY(CCONTAINER_spacing);
DECLARE_PROPERTY(CCONTAINER_margin);

void CCONTAINER_cb_arrange(gContainer *sender);
void CCONTAINER_raise_insert(CCONTAINER *_object, CWIDGET *child);

#endif
