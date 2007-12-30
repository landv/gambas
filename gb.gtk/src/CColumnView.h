/***************************************************************************

  CColumnView.h

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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

#ifndef __CCOLUMNVIEW_H
#define __CCOLUMNVIEW_H

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#ifndef __CCOLUMNVIEW_CPP
extern GB_DESC CColumnViewItemDesc[];
extern GB_DESC CColumnViewColumnDesc[];
extern GB_DESC CColumnViewColumnsDesc[];
extern GB_DESC CColumnViewDesc[];
#else

#define THIS ((CCOLUMNVIEW *)_object)
#define COLUMNVIEW ((gColumnView*)THIS->widget)
#define CCOLUMNVIEW_PROPERTIES CWIDGET_PROPERTIES \
  ",Mode,Sorted,Editable,Root,Resizable,AutoResize,Header,Border,ScrollBar"

#endif

typedef struct 
{
	GB_BASE ob;
	gControl *widget;
	GB_VARIANT_VALUE tag;
	
	long index;
	long cursor;
	long colIndex;

}  CCOLUMNVIEW;


#endif
