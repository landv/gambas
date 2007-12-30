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
#include "gframe.h"

/***************************************************************************

  Container

***************************************************************************/

DECLARE_EVENT(EVENT_Arrange);
DECLARE_EVENT(EVENT_Insert);


void CCONTAINER_cb_arrange(gContainer *sender)
{
	GB.Raise(sender->hFree, EVENT_Arrange, 0);
}

void CCONTAINER_raise_insert(CCONTAINER *_object, CWIDGET *child)
{
	GB.Raise(THIS, EVENT_Insert, 1, GB_T_OBJECT, child);
}

static CWIDGET *get_child(gContainer *container, int index)
{
	gControl *child = container->child(index);
	
	if (child)
		return (CWIDGET *)child->hFree;
	else
		return NULL;
}

BEGIN_METHOD_VOID(CCONTAINER_next)

	gContainer *cont = WIDGET->proxy() ? WIDGET->proxy() : WIDGET;
	int *ct;
	
	ct = (int *)GB.GetEnum();
	
	if (*ct >= cont->childCount()) 
	{ 
		GB.StopEnum(); 
		return; 
	}
	
	GB.ReturnObject(get_child(cont, *ct));
	(*ct)++;
	
END_METHOD


BEGIN_PROPERTY(CCONTAINER_count)

	gContainer *cont = WIDGET->proxy() ? WIDGET->proxy() : WIDGET;
	GB.ReturnInteger(cont->childCount());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_x)

	GB.ReturnInteger(WIDGET->clientX());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_y)

	GB.ReturnInteger(WIDGET->clientY());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_width)

	GB.ReturnInteger(WIDGET->clientWidth());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_height)

	GB.ReturnInteger(WIDGET->clientHeight());
	
END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_arrangement)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->arrange()); return; }
	WIDGET->setArrange(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCONTAINER_auto_resize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->autoResize());
	else
		WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCONTAINER_padding)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->padding()); return; }
	WIDGET->setPadding(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_spacing)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->spacing()); return; }
	WIDGET->setSpacing(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CCONTAINER_find, GB_INTEGER x; GB_INTEGER y)

	gControl *child = WIDGET->find(VARG(x), VARG(y));
	
	if (child)
		GB.ReturnObject(child->hFree);
	else
		GB.ReturnNull();

END_METHOD


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

  GB_METHOD("Find", "Control", CCONTAINER_find, "(X)i(Y)i"),

  GB_EVENT("Arrange", NULL, NULL, &EVENT_Arrange),
  GB_EVENT("Insert", NULL, "(Control)Control", &EVENT_Insert),
  
  GB_END_DECLARE
};

/****************************************************************************

	UserControl & UserContainer

****************************************************************************/

BEGIN_METHOD(CUSERCONTROL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	
	PANEL->setArrange(ARRANGE_FILL);
	PANEL->setUser(true);

	THIS_UC->container = THIS;

END_METHOD

	
BEGIN_PROPERTY(CUSERCONTROL_container)

	CCONTAINER *ct;
	gControl *test;
	int count;
	int bucle;
	bool ok=false;
	
	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS_UC->container);
		return;
	}
	
	ct = (CCONTAINER*)VPROP(GB_OBJECT);
	
	if (!ct)
	{
		THIS_UC->container = THIS;
		WIDGET->setProxy(0);
		return;
	}
	
	if (GB.CheckObject(ct)) 
		return;
	
	count = PANEL->childCount();
	
	for (bucle=0;bucle<count;bucle++)
	{
		test=PANEL->child(bucle);
		do 
		{
			if (test->parent()==WIDGET )
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
	
	THIS_UC->container = ct;
	WIDGET->setProxy(WIDGET_CONT);
	WIDGET_CONT->performArrange();

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_container)

	if (READ_PROPERTY)
		CUSERCONTROL_container(_object, _param);
	else
	{
		CUSERCONTROL_container(_object, _param);

		WIDGET_CONT->setFullArrangement(THIS_UC->save);
		//qDebug("(%s %p): arrangement = %08X", GB.GetClassName(THIS), THIS, after->arrangement);
	}

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_arrangement)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET_CONT->arrange());
	else
	{
		WIDGET_CONT->setArrange(VPROP(GB_INTEGER));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_auto_resize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET_CONT->autoResize());
	else
	{
		WIDGET_CONT->setAutoResize(VPROP(GB_BOOLEAN));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_padding)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET_CONT->padding());
	else
	{
		WIDGET_CONT->setPadding(VPROP(GB_INTEGER));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_spacing)
	
	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET_CONT->spacing());
	else
	{
		WIDGET_CONT->setSpacing(VPROP(GB_INTEGER));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


GB_DESC CUserControlDesc[] =
{
  GB_DECLARE("UserControl", sizeof(CUSERCONTROL)), GB_INHERITS("Container"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),
  GB_PROPERTY("_Container", "Container", CUSERCONTROL_container),
  GB_PROPERTY("_AutoResize", "b", CCONTAINER_auto_resize),
  
	USERCONTROL_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CUserContainerDesc[] =
{
  GB_DECLARE("UserContainer", sizeof(CUSERCONTROL)), GB_INHERITS("Container"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),

  GB_PROPERTY("_Container", "Container", CUSERCONTAINER_container),

  GB_PROPERTY("Arrangement", "i", CUSERCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CUSERCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CUSERCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CUSERCONTAINER_spacing),

	USERCONTAINER_DESCRIPTION,

  GB_END_DECLARE
};

