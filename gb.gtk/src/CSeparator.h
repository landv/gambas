/***************************************************************************

  CSeparator.h

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

#ifndef __CSEPARATOR_H
#define __CSEPARATOR_H

#include "widgets.h"
#include "gambas.h"
#include "CWidget.h"


#ifndef __CSEPARATOR_CPP
extern GB_DESC CSeparatorDesc[];
#else

#define THIS ((CSEPARATOR *)_object)
#define SEPARATOR ((gSeparator*)THIS->widget)

#define CSEPARATOR_PROPERTIES CWIDGET_PROPERTIES 



#endif 

typedef  struct 
{
    GB_BASE ob;
    gControl *widget;
	GB_VARIANT_VALUE tag;

}  CSEPARATOR;

#endif
