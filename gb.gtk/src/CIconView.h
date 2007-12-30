/***************************************************************************

  CIconView.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

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

#ifndef __CICONVIEW_H
#define __CICONVIEW_H

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#ifndef __CICONVIEW_CPP

extern GB_DESC CIconViewItemDesc[];
extern GB_DESC CIconViewDesc[];

#else

#define THIS ((CICONVIEW *)_object)
#define ICONVIEW ((gIconView*)THIS->widget)
#define CICONVIEW_PROPERTIES CWIDGET_PROPERTIES \
  ",Mode,Sorted,Arrangement,GridX,GridY,WordWrap,Border,ScrollBar"


#endif



typedef struct 
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;

	char *icursor;
	char *item;
	
}  CICONVIEW;



#endif
