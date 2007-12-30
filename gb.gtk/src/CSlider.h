/***************************************************************************

  CSlider.h

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
#ifndef __CSLIDER_H
#define __CSLIDER_H

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#ifndef __CSLIDER_CPP

extern GB_DESC CSliderDesc[];
extern GB_DESC CScrollBarDesc[];

#else

#define THIS    ((CSLIDER *)_object)
#define SLIDER  ((gSlider*)THIS->widget)
#define SBAR ((gScrollBar *)THIS->widget)

#define CSLIDER_PROPERTIES CWIDGET_PROPERTIES \
  ",Value,MinValue,MaxValue,Step,Tracking,PageStep,Mark"

#define CSCROLLBAR_PROPERTIES CWIDGET_PROPERTIES \
  ",MinValue,MaxValue,Step,Tracking,PageStep"

#endif

typedef  struct
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;

}  CSLIDER;

typedef  struct
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;

}  CSCROLLBAR;




#endif
