/***************************************************************************

  CSplitter.cpp

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

#define __CSPLITTER_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CSplitter.h"


DECLARE_EVENT(EVENT_Resize);

void gb_raise_splitter_Resize(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Resize,0);
}

BEGIN_METHOD(CHSPLIT_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSplitter(Parent->widget,0);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	WIDGET->onResize=gb_raise_splitter_Resize;

END_METHOD


BEGIN_METHOD(CVSPLIT_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSplitter(Parent->widget,1);
	InitControl(THIS->widget,(CWIDGET*)THIS);

END_METHOD


BEGIN_PROPERTY(CSPLITTER_layout)

	char *vl;
	
	if (READ_PROPERTY)
	{
		vl=WIDGET->layout();
		GB.ReturnNewString(vl,0);
		return;
	}
	
	WIDGET->setLayout( GB.ToZeroString(PROP(GB_STRING)) );


END_PROPERTY


GB_DESC CHSplitDesc[] =
{
  GB_DECLARE("HSplit", sizeof(CSPLITTER)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHSPLIT_new, "(Parent)Container;"),

  GB_PROPERTY("Layout", "s", CSPLITTER_layout),

  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

  GB_CONSTANT("_Properties", "s", CSPLITTER_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Resize"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_HORIZONTAL),

  GB_END_DECLARE
};


GB_DESC CVSplitDesc[] =
{
  GB_DECLARE("VSplit", sizeof(CSPLITTER)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVSPLIT_new, "(Parent)Container;"),

  GB_PROPERTY("Layout", "s", CSPLITTER_layout),

  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

  GB_CONSTANT("_Properties", "s", CSPLITTER_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Resize"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_VERTICAL),

  GB_END_DECLARE
};





