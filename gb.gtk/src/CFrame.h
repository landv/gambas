/***************************************************************************

  CFrame.h

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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

#ifndef __CFRAME_H
#define __CFRAME_H

#include "gambas.h"
#include "widgets.h"
#include "CWidget.h"
#include "CContainer.h"

#ifndef __CFRAME_CPP
extern GB_DESC CUserControlDesc[];
extern GB_DESC CUserContainerDesc[];
extern GB_DESC CFrameDesc[];
extern GB_DESC CPanelDesc[];
extern GB_DESC CHBoxDesc[];
extern GB_DESC CVBoxDesc[];
extern GB_DESC CHPanelDesc[];
extern GB_DESC CVPanelDesc[];
#else

#define THIS ((CFRAME *)_object)

#define WIDGET ((gFrame *)((CWIDGET *)_object)->widget)

#define CFRAME_PROPERTIES CWIDGET_PROPERTIES \
  ",Text"
  
#define CUSERCONTAINER_PROPERTIES CWIDGET_PROPERTIES \
  ",Arrangement,AutoResize,Spacing,Padding"

#define CPANEL_PROPERTIES CWIDGET_PROPERTIES \
  "," CARRANGEMENT_PROPERTIES ",Border"

#define CHBOX_PROPERTIES CWIDGET_PROPERTIES \
  ",Spacing,Padding"

#define CVBOX_PROPERTIES CHBOX_PROPERTIES

#endif

typedef  struct
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;

}  CFRAME;

typedef  struct
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;
	CCONTAINER *ct;

}  CUSERCONTROL;

#endif
