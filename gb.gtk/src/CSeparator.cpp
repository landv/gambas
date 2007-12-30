/***************************************************************************

  CSeparator.cpp

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

#define __CSEPARATOR_CPP

#include <stdio.h>

#include "gambas.h"
#include "widgets.h"
#include "CSeparator.h"
#include "CContainer.h"


BEGIN_METHOD(CSEPARATOR_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSeparator(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
END_METHOD



GB_DESC CSeparatorDesc[] =
{
  GB_DECLARE("Separator", sizeof(CSEPARATOR)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CSEPARATOR_new, "(Parent)Container;"),

  
  GB_CONSTANT("_Properties", "s", CSEPARATOR_PROPERTIES), 

  GB_END_DECLARE
};




