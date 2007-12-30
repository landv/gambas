/***************************************************************************

  CContainer.cpp

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

#define __CCONTAINER_CPP



#include "gambas.h"
#include "CContainer.h"

/***************************************************************************

  CContainer

***************************************************************************/

BEGIN_METHOD_VOID(CCONTAINER_next)

	long *ct;
	
	ct=(long*)GB.GetEnum();
	
	
	if (ct[0]>=ChildrenCount(CONTAINER)) { GB.StopEnum(); return; }
	GB.ReturnObject(GetChild(CONTAINER,ct[0]));
	ct[0]++;
	
END_METHOD


BEGIN_PROPERTY(CCONTAINER_count)

	GB.ReturnInteger(ChildrenCount(CONTAINER));

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_x)

	GB.ReturnInteger(CONTAINER->clientX());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_y)

	GB.ReturnInteger(CONTAINER->clientY());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_width)

	GB.ReturnInteger(CONTAINER->clientWidth());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_height)

	GB.ReturnInteger(CONTAINER->clientHeight());
	
END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_arrangement)

	if (READ_PROPERTY) { GB.ReturnInteger(CONTAINER->arrange()); return; }
	CONTAINER->setArrange(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCONTAINER_auto_resize)

	stub("CCOLUMNVIEW_column_auto_resize");
	if (READ_PROPERTY) GB.ReturnBoolean(false);

END_PROPERTY

BEGIN_PROPERTY(CCONTAINER_padding)

	if (READ_PROPERTY) { GB.ReturnInteger(CONTAINER->padding()); return; }
	CONTAINER->setPadding(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_spacing)

	if (READ_PROPERTY) { GB.ReturnInteger(CONTAINER->spacing()); return; }
	CONTAINER->setSpacing(VPROP(GB_INTEGER));

END_PROPERTY


GB_DESC CChildrenDesc[] =
{
  GB_DECLARE(".ContainerChildren", sizeof(CCONTAINER)), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CCONTAINER_next, NULL),
  GB_PROPERTY_READ("Count", "i", CCONTAINER_count),

  GB_END_DECLARE
};


GB_DESC CContainerDesc[] =
{
  GB_DECLARE("Container", sizeof(CCONTAINER)), GB_INHERITS("Control"),

  GB_NOT_CREATABLE(),

  GB_PROPERTY_SELF("Children", ".ContainerChildren"),

  GB_PROPERTY_READ("ClientX", "i", CCONTAINER_x),
  GB_PROPERTY_READ("ClientY", "i", CCONTAINER_y),
  GB_PROPERTY_READ("ClientW", "i", CCONTAINER_width),
  GB_PROPERTY_READ("ClientWidth", "i", CCONTAINER_width),
  GB_PROPERTY_READ("ClientH", "i", CCONTAINER_height),
  GB_PROPERTY_READ("ClientHeight", "i", CCONTAINER_height),

  GB_END_DECLARE
};



