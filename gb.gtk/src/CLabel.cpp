/***************************************************************************

  CLabel.cpp

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

#define __CLABEL_CPP

#include <stdio.h>

#include "gambas.h"
#include "widgets.h"
#include "CLabel.h"
#include "CContainer.h"


BEGIN_METHOD(CLABEL_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSimpleLabel(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
END_METHOD

BEGIN_METHOD(CTEXTLABEL_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSimpleLabel(Parent->widget);
	SLABEL->enableMarkup(true);
	SLABEL->setAlignment(alignTopNormal);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
END_METHOD

BEGIN_PROPERTY(CLABEL_auto_resize)

  if (READ_PROPERTY) { GB.ReturnBoolean(SLABEL->autoResize()); return; }
  
  SLABEL->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLABEL_text)

	char *vl="";
	
	if (READ_PROPERTY) { GB.ReturnNewString(SLABEL->text(),0); return; }
	if ( PROP(GB_STRING)->value.addr ) vl=PROP(GB_STRING)->value.addr;
	SLABEL->setText(vl);

END_PROPERTY

BEGIN_PROPERTY(CLABEL_border)

	if (READ_PROPERTY) { GB.ReturnInteger(SLABEL->getBorder()); return; }
	SLABEL->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CLABEL_alignment)

	if (READ_PROPERTY) { GB.ReturnInteger(SLABEL->alignment()); return; }
	SLABEL->setAlignment(VPROP(GB_INTEGER));


END_PROPERTY


BEGIN_PROPERTY(CLABEL_transparent)

	if (READ_PROPERTY) { GB.ReturnBoolean(SLABEL->transparent()); return; }
	SLABEL->setTransparent(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CLabelDesc[] =
{
  GB_DECLARE("Label", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CLABEL_text),
  GB_PROPERTY("Caption", "s", CLABEL_text),
  GB_PROPERTY("Alignment", "i<Align>", CLABEL_alignment),
  GB_PROPERTY("Border", "i<Border>", CLABEL_border),
  GB_PROPERTY("AutoResize", "b", CLABEL_auto_resize),
  GB_PROPERTY("Transparent","b",CLABEL_transparent),
  
  GB_CONSTANT("_Properties", "s", CLABEL_PROPERTIES), 

  GB_END_DECLARE
};


GB_DESC CTextViewDesc[] =
{
  GB_DECLARE("TextLabel", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTEXTLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CLABEL_text),
  GB_PROPERTY("Caption", "s", CLABEL_text),
  GB_PROPERTY("Alignment", "i<Align>", CLABEL_alignment),
  GB_PROPERTY("Border", "i<Border>", CLABEL_border), 
  GB_PROPERTY("Transparent","b",CLABEL_transparent),

  GB_CONSTANT("_Properties", "s", CTEXTLABEL_PROPERTIES), 

  GB_END_DECLARE
};



