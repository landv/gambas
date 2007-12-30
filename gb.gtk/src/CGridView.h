/***************************************************************************

  CGridView.h

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

#ifndef __CGRIDVIEW_H
#define __CGRIDVIEW_H

#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "tablerender.h"
#include "CPicture.h"

#ifndef __CGRIDVIEW_CPP
extern GB_DESC CGridViewItemDesc[];
extern GB_DESC CGridViewDataDesc[];
extern GB_DESC CGridViewColumnDesc[];
extern GB_DESC CGridViewRowDesc[];
extern GB_DESC CGridViewColumnsDesc[];
extern GB_DESC CGridViewRowsDesc[];
extern GB_DESC CGridViewDesc[];
#else

#define THIS ((CGRIDVIEW *)_object)
#define GRIDVIEW ((gGridView*)THIS->widget)
#define CGRIDVIEW_PROPERTIES CWIDGET_PROPERTIES \
  ",Mode,Sorted,Editable,Root,Resizable,AutoResize,Header,Border,ScrollBar"

#endif


typedef struct 
{
	GB_BASE ob;
	gControl *widget;
	GB_VARIANT_VALUE tag;

	long row;
	long col;
	long curr_row;
	long curr_col;
	gTableData *data;
	

}  CGRIDVIEW;


#endif
