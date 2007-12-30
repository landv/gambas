/***************************************************************************

  CFrame.cpp

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

#define __CFRAME_CPP

#include "CFrame.h"


BEGIN_METHOD(CFRAME_new, GB_OBJECT parent)

	InitControl(new gFrame(CONTAINER(VARG(parent))), (CWIDGET*)THIS);

END_METHOD


BEGIN_METHOD(CPANEL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);

END_METHOD


BEGIN_METHOD(CHBOX_new, GB_OBJECT parent)
	
	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_HORIZONTAL);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CVBOX_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_VERTICAL);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CHPANEL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_LEFT_RIGHT);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CVPANEL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_TOP_BOTTOM);
	//WIDGET->setAutoSize(true);

END_METHOD

BEGIN_PROPERTY(CPANEL_border)

	if (READ_PROPERTY) { GB.ReturnInteger(PANEL->getBorder()); return; }
	PANEL->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CFRAME_text)

	if (READ_PROPERTY) { GB.ReturnNewString( FRAME->text(),0 ); return; }
	FRAME->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

GB_DESC CFrameDesc[] =
{
  GB_DECLARE("Frame", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CFRAME_new, "(Parent)Container;"),

  GB_PROPERTY("Caption", "s", CFRAME_text),
  GB_PROPERTY("Text", "s", CFRAME_text),
  GB_PROPERTY("Title", "s", CFRAME_text),

  FRAME_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CPanelDesc[] =
{
  GB_DECLARE("Panel", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Border", "i", CPANEL_border),
  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

	PANEL_DESCRIPTION,
	
  GB_END_DECLARE
};


GB_DESC CHBoxDesc[] =
{
  GB_DECLARE("HBox", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),

	HBOX_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CVBoxDesc[] =
{
  GB_DECLARE("VBox", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),

	VBOX_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CHPanelDesc[] =
{
  GB_DECLARE("HPanel", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),

	HPANEL_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CVPanelDesc[] =
{
  GB_DECLARE("VPanel", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),

	VPANEL_DESCRIPTION,

  GB_END_DECLARE
};



