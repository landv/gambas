/***************************************************************************

  CSpinBox.h

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

#ifndef __CSPINBOX_H
#define __CSPINBOX_H

#include "gambas.h"
#include "widgets.h"

#ifndef __CSPINBOX_CPP

extern GB_DESC CSpinBoxDesc[];

#else

#define THIS    ((CSPINBOX *)_object)
#define SPINBOX  ((gSpinBox*)THIS->widget)

#define CSPINBOX_PROPERTIES CWIDGET_PROPERTIES \
  ",Value,MinValue,MaxValue,Step,Prefix,Suffix,Wrap"

#endif

typedef  struct
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;

}  CSPINBOX;



#endif
