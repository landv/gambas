/***************************************************************************

  CFrame.cpp

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

#define __CFRAME_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CFrame.h"


BEGIN_METHOD(CUSERCONTROL_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
	WIDGET->setArrange(ARRANGE_FILL);
	WIDGET->setBorder(0);

END_METHOD

BEGIN_PROPERTY(CUSERCONTROL_container)

	CCONTAINER *ct;
	gControl *test;
	long count;
	long bucle;
	bool ok=false;
	
	if (READ_PROPERTY)
	{
		if ( !((CUSERCONTROL*)_object)->ct )
			GB.ReturnObject(_object);
		else
			GB.ReturnObject( ((CUSERCONTROL*)_object)->ct );
		
		return;
	}
	
	ct=(CCONTAINER*)VPROP(GB_OBJECT);
	if (GB.CheckObject(ct)) return;
	if (!ct->widget) { GB.Error("Invalid object"); return; }
	
	count=((gContainer*)WIDGET)->childCount();
	
	for (bucle=0;bucle<count;bucle++)
	{
		test=((gContainer*)WIDGET)->child(bucle);
		do 
		{
			if (test->pr==THIS->widget )
			{
				ok=true;
				break;
			}
			test=test->parent();
		} while (test);
	}
	
	if (!ok)
	{
		GB.Error("Container must be a child control");
		return;
	}
	
	((CUSERCONTROL*)_object)->ct=ct;


END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_arrangement)

	gContainer *test=(gContainer*)WIDGET->widget;

	if (((CUSERCONTROL*)_object)->ct)
	{
		test=(gContainer*) ((CUSERCONTROL*)_object)->ct->widget;
	}
	
	if (READ_PROPERTY) { GB.ReturnInteger(test->arrange()); return; }
	test->setArrange(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_auto_resize)

	stub("CUSERCONTAINER_auto_resize");
	if (READ_PROPERTY) GB.ReturnBoolean(false);

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_padding)

	gContainer *test=(gContainer*)WIDGET->widget;

	if (((CUSERCONTROL*)_object)->ct)
	{
		test=(gContainer*) ((CUSERCONTROL*)_object)->ct->widget;
	}
	
	if (READ_PROPERTY) { GB.ReturnInteger(test->padding()); return; }
	test->setPadding(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_spacing)
	
	gContainer *test=(gContainer*)WIDGET->widget;

	if (((CUSERCONTROL*)_object)->ct)
	{
		test=(gContainer*) ((CUSERCONTROL*)_object)->ct->widget;
	}
	
	if (READ_PROPERTY) { GB.ReturnInteger(test->spacing()); return; }
	test->setSpacing(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CFRAME_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
END_METHOD


BEGIN_METHOD(CPANEL_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	WIDGET->setBorder(0);
	InitControl(THIS->widget,(CWIDGET*)THIS);


END_METHOD


BEGIN_METHOD(CHBOX_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	WIDGET->setBorder(0);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
	WIDGET->setArrange(ARRANGE_HORIZONTAL);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CVBOX_new, GB_OBJECT parent)

  	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	WIDGET->setBorder(0);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
	WIDGET->setArrange(ARRANGE_VERTICAL);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CHPANEL_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	WIDGET->setBorder(0);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
	WIDGET->setArrange(ARRANGE_LEFT_RIGHT);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CVPANEL_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gFrame(Parent->widget);
	WIDGET->setBorder(0);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	
	WIDGET->setArrange(ARRANGE_TOP_BOTTOM);
	//WIDGET->setAutoSize(true);

END_METHOD

BEGIN_PROPERTY(CPANEL_border)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->getBorder()); return; }
	WIDGET->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CFRAME_text)

	if (READ_PROPERTY) { GB.ReturnNewString( ((gFrame*)THIS->widget)->text(),0 ); return; }
	((gFrame*)THIS->widget)->setText(GB.ToZeroString(PROP(GB_STRING)));
	

END_PROPERTY

GB_DESC CUserControlDesc[] =
{
  GB_DECLARE("UserControl", sizeof(CUSERCONTROL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),
  GB_PROPERTY("Container", "Container", CUSERCONTROL_container),
  
  GB_CONSTANT("_Properties", "s", CWIDGET_PROPERTIES),

  GB_END_DECLARE
};

GB_DESC CUserContainerDesc[] =
{
  GB_DECLARE("UserContainer", sizeof(CUSERCONTROL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),

  GB_PROPERTY("Container", "Container", CUSERCONTROL_container),

  GB_PROPERTY("Arrangement", "i<Arrange>", CUSERCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CUSERCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CUSERCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CUSERCONTAINER_spacing),

  GB_CONSTANT("_Properties", "s", CUSERCONTAINER_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_END_DECLARE
};

GB_DESC CFrameDesc[] =
{
  GB_DECLARE("Frame", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CFRAME_new, "(Parent)Container;"),

  GB_PROPERTY("Caption", "s", CFRAME_text),
  GB_PROPERTY("Text", "s", CFRAME_text),
  GB_PROPERTY("Title", "s", CFRAME_text),

  GB_CONSTANT("_Properties", "s", CFRAME_PROPERTIES),

  GB_END_DECLARE
};

GB_DESC CPanelDesc[] =
{
  GB_DECLARE("Panel", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Border", "i<Border>", CPANEL_border),
  GB_PROPERTY("Arrangement", "i<Arrange>", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

  GB_CONSTANT("_Properties", "s", CPANEL_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_END_DECLARE
};


GB_DESC CHBoxDesc[] =
{
  GB_DECLARE("HBox", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

  GB_CONSTANT("_Properties", "s", CHBOX_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_HORIZONTAL),

  GB_END_DECLARE
};


GB_DESC CVBoxDesc[] =
{
  GB_DECLARE("VBox", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

  GB_CONSTANT("_Properties", "s", CVBOX_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_VERTICAL),

  GB_END_DECLARE
};


GB_DESC CHPanelDesc[] =
{
  GB_DECLARE("HPanel", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

  GB_CONSTANT("_Properties", "s", CHBOX_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_LEFT_RIGHT),

  GB_END_DECLARE
};


GB_DESC CVPanelDesc[] =
{
  GB_DECLARE("VPanel", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

  GB_CONSTANT("_Properties", "s", CVBOX_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_TOP_BOTTOM),

  GB_END_DECLARE
};



