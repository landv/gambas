/***************************************************************************

  CContainer.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CCONTAINER_CPP

#include "gambas.h"
#include "CContainer.h"
#include "gframe.h"
#include "gmainwindow.h"

/***************************************************************************

  Container

***************************************************************************/

DECLARE_EVENT(EVENT_BeforeArrange);
DECLARE_EVENT(EVENT_Arrange);
DECLARE_EVENT(EVENT_Insert);


void CCONTAINER_cb_before_arrange(gContainer *sender)
{
	GB.Raise(sender->hFree, EVENT_BeforeArrange, 0);
}

void CCONTAINER_cb_arrange(gContainer *sender)
{
	GB.Raise(sender->hFree, EVENT_Arrange, 0);
}

void CCONTAINER_raise_insert(CCONTAINER *_object, CWIDGET *child)
{
	GB.Raise(THIS, EVENT_Insert, 1, GB_T_OBJECT, child);
}

static int get_child_count(gContainer *ct)
{
	int i;
	int n = 0;
	
	for (i = 0; i < ct->childCount(); i++)
	{
		if (ct->child(i)->hFree)
			n++;
	}
	
	return n;
}

static CWIDGET *get_child(gContainer *ct, int index)
{
	int i;
	int n = 0;
	gControl *ch;
	
	for (i = 0; i < ct->childCount(); i++)
	{
		ch = ct->child(i);
		if (!ch->hFree)
			continue;

		if (n == index)
			return (CWIDGET *)(ch->hFree);
		n++;
	}
	
	return NULL;
}

BEGIN_METHOD_VOID(ContainerChildren_next)

	gContainer *cont = WIDGET->proxyContainer();
	int *ct;
	CWIDGET *child;
	
	ct = (int *)GB.GetEnum();
	
	if (*ct >= get_child_count(cont)) 
	{ 
		GB.StopEnum(); 
		return; 
	}

	child = get_child(cont, *ct);
	(*ct)++;
	
	GB.ReturnObject(child);
	
END_METHOD


BEGIN_METHOD(ContainerChildren_get, GB_INTEGER index)

	gContainer *cont = WIDGET->proxyContainer();	
	int ct = VARG(index);
	
	if (ct < 0 || ct >= get_child_count(cont)) 
	{ 
		GB.Error(GB_ERR_BOUND);
		return; 
	}
	
	GB.ReturnObject(get_child(cont, ct));
	
END_METHOD


BEGIN_PROPERTY(ContainerChildren_Count)

	gContainer *cont = WIDGET->proxyContainer();
	GB.ReturnInteger(get_child_count(cont));

END_PROPERTY


BEGIN_METHOD_VOID(ContainerChildren_Clear)

	gContainer *cont = WIDGET->proxyContainer();
	gControl *child;
	
	for(;;)
	{
		child = cont->child(0);
		if (!child)
			break;
		child->destroy();
	}
	
END_METHOD

static void get_client_area(gContainer *cont, int *x, int *y, int *w, int *h)
{
	gContainer *proxy = cont->proxyContainer();
	
	if (x) *x = proxy->clientX();
	if (y) *y = proxy->clientY();
	if (w) *w = proxy->clientWidth();
	if (h) *h = proxy->clientHeight();
	
	if (x || y)
	{
		while (proxy && proxy != cont)
		{
			if (x) *x += proxy->x();
			if (y) *y += proxy->y();
			proxy = proxy->parent();
		}
	}
}

BEGIN_PROPERTY(Container_X)

	int x;
	get_client_area(WIDGET, &x, NULL, NULL, NULL);
	GB.ReturnInteger(x);

END_PROPERTY


BEGIN_PROPERTY(Container_Y)

	int y;
	get_client_area(WIDGET, NULL, &y, NULL, NULL);
	GB.ReturnInteger(y);

END_PROPERTY


BEGIN_PROPERTY(Container_ClientWidth)

	int w;
	get_client_area(WIDGET, NULL, NULL, &w, NULL);
	GB.ReturnInteger(w);


END_PROPERTY


BEGIN_PROPERTY(Container_ClientHeight)

	int h;
	get_client_area(WIDGET, NULL, NULL, NULL, &h);
	GB.ReturnInteger(h);
	
END_PROPERTY


BEGIN_PROPERTY(Container_Arrangement)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->arrange()); return; }
	WIDGET->setArrange(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Container_AutoResize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->autoResize());
	else
		WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Container_Padding)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->padding()); return; }
	WIDGET->setPadding(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(Container_Spacing)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->spacing()); return; }
	WIDGET->setSpacing(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Container_Margin)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->margin()); return; }
	WIDGET->setMargin(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Container_Indent)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->indent());
	else
		WIDGET->setMargin(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(Container_Invert)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->invert());
	else
		WIDGET->setInvert(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(Container_Find, GB_INTEGER x; GB_INTEGER y)

	gControl *child = WIDGET->find(VARG(x), VARG(y));
	
	if (child)
		GB.ReturnObject(child->hFree);
	else
		GB.ReturnNull();

END_METHOD

BEGIN_METHOD(Container_unknown, GB_VALUE x; GB_VALUE y)

	char *name = GB.GetUnknown();
	int nparam = GB.NParam();
	
	if (strcasecmp(name, "Find"))
	{
		GB.Error(GB_ERR_NSYMBOL, GB.GetClassName(NULL), name);
		return;
	}
	
	if (nparam < 2)
	{
		GB.Error("Not enough argument");
		return;
	}
	else if (nparam > 2)
	{
		GB.Error("Too many argument");
		return;
	}
	
	GB.Deprecated("gb.gtk", "Container.Find", "Container.FindChild");
	
	if (GB.Conv(ARG(x), GB_T_INTEGER) || GB.Conv(ARG(y), GB_T_INTEGER))
		return;
	
	Container_Find(_object, _param);
	
	GB.ReturnConvVariant();

END_METHOD


GB_DESC CChildrenDesc[] =
{
  GB_DECLARE(".Container.Children", sizeof(CCONTAINER)), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", ContainerChildren_next, NULL),
  GB_METHOD("_get", "Control", ContainerChildren_get, "(Index)i"),
  GB_PROPERTY_READ("Count", "i", ContainerChildren_Count),
  GB_METHOD("Clear", NULL, ContainerChildren_Clear, NULL),

  GB_END_DECLARE
};


GB_DESC CContainerDesc[] =
{
  GB_DECLARE("Container", sizeof(CCONTAINER)), GB_INHERITS("Control"),

  GB_NOT_CREATABLE(),

  GB_PROPERTY_SELF("Children", ".Container.Children"),

  GB_PROPERTY_READ("ClientX", "i", Container_X),
  GB_PROPERTY_READ("ClientY", "i", Container_Y),
  GB_PROPERTY_READ("ClientW", "i", Container_ClientWidth),
  GB_PROPERTY_READ("ClientWidth", "i", Container_ClientWidth),
  GB_PROPERTY_READ("ClientH", "i", Container_ClientHeight),
  GB_PROPERTY_READ("ClientHeight", "i", Container_ClientHeight),

  GB_METHOD("FindChild", "Control", Container_Find, "(X)i(Y)i"),
  GB_METHOD("_unknown", "v", Container_unknown, "."),

	CONTAINER_DESCRIPTION,

  GB_EVENT("BeforeArrange", NULL, NULL, &EVENT_BeforeArrange),
  GB_EVENT("Arrange", NULL, NULL, &EVENT_Arrange),
  GB_EVENT("NewChild", NULL, "(Control)Control", &EVENT_Insert),
  
  GB_END_DECLARE
};

/****************************************************************************

	UserControl & UserContainer

****************************************************************************/

BEGIN_METHOD(UserControl_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	
	PANEL->setArrange(ARRANGE_FILL);
	PANEL->setUser(true);

	THIS_UC->container = THIS;

END_METHOD

	
BEGIN_PROPERTY(UserControl_Container)

	CCONTAINER *ct;
	gControl *test;
	int count;
	int bucle;
	bool ok = false;
	
	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS_UC->container);
		return;
	}
	
	ct = (CCONTAINER*)VPROP(GB_OBJECT);
	
	if (!ct)
	{
		THIS_UC->container = THIS;
		WIDGET->setProxyContainer(NULL);
		WIDGET->setProxy(NULL);
		return;
	}
	
	if (GB.CheckObject(ct)) 
		return;
	
	count = PANEL->childCount();
	
	for (bucle = 0; bucle < count; bucle++)
	{
		test = PANEL->child(bucle);
		do 
		{
			if (test->parent() == WIDGET)
			{
				ok = true;
				break;
			}
			test = test->parent();
		} while (test);
	}
	
	if (!ok)
	{
		GB.Error("Container must be a child control");
		return;
	}

	gColor bg = THIS_UC->container->ob.widget->background();
	gColor fg = THIS_UC->container->ob.widget->foreground();

	THIS_UC->container = (CCONTAINER *)GetObject(((gContainer *)ct->ob.widget)->proxyContainer());
	WIDGET->setProxyContainer(WIDGET_CONT->proxyContainer());
	WIDGET->setProxy(THIS_UC->container->ob.widget);

	THIS_UC->container->ob.widget->setBackground(bg);
	THIS_UC->container->ob.widget->setForeground(fg);

	WIDGET_CONT->performArrange();

END_PROPERTY


BEGIN_PROPERTY(UserContainer_Container)

	if (READ_PROPERTY)
		UserControl_Container(_object, _param);
	else
	{
		UserControl_Container(_object, _param);

		WIDGET_CONT->setFullArrangement(&THIS_UC->save);
		//qDebug("(%s %p): arrangement = %08X", GB.GetClassName(THIS), THIS, after->arrangement);
	}

END_PROPERTY


BEGIN_PROPERTY(UserContainer_Arrangement)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET_CONT->arrange());
	else
	{
		WIDGET_CONT->setArrange(VPROP(GB_INTEGER));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}

END_PROPERTY


BEGIN_PROPERTY(UserContainer_AutoResize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET_CONT->autoResize());
	else
	{
		WIDGET_CONT->setAutoResize(VPROP(GB_BOOLEAN));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(UserContainer_Padding)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET_CONT->padding());
	else
	{
		WIDGET_CONT->setPadding(VPROP(GB_INTEGER));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(UserContainer_Spacing)
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET_CONT->spacing());
	else
	{
		WIDGET_CONT->setSpacing(VPROP(GB_BOOLEAN));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(UserContainer_Margin)
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET_CONT->margin());
	else
	{
		WIDGET_CONT->setMargin(VPROP(GB_BOOLEAN));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(UserContainer_Indent)
	
	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET_CONT->indent());
	else
	{
		WIDGET_CONT->setIndent(VPROP(GB_INTEGER));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY


BEGIN_PROPERTY(UserContainer_Invert)
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET_CONT->invert());
	else
	{
		WIDGET_CONT->setInvert(VPROP(GB_BOOLEAN));
		THIS_UC->save = WIDGET_CONT->fullArrangement();
	}
	
END_PROPERTY

BEGIN_PROPERTY(UserContainer_Focus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->canFocus());
	else
		WIDGET->setCanFocus(VPROP(GB_BOOLEAN));

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC CUserControlDesc[] =
{
  GB_DECLARE("UserControl", sizeof(CUSERCONTROL)), GB_INHERITS("Container"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_new", NULL, UserControl_new, "(Parent)Container;"),
  GB_PROPERTY("_Container", "Container", UserControl_Container),
  GB_PROPERTY("_AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("_Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("_Padding", "i", Container_Padding),
	GB_PROPERTY("_Spacing", "b", Container_Spacing),
	GB_PROPERTY("_Margin", "b", Container_Margin),
	GB_PROPERTY("_Indent", "b", Container_Indent),
	GB_PROPERTY("_Invert", "b", Container_Invert),

	USERCONTROL_DESCRIPTION,

	GB_END_DECLARE
};

GB_DESC CUserContainerDesc[] =
{
  GB_DECLARE("UserContainer", sizeof(CUSERCONTROL)), GB_INHERITS("Container"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_new", NULL, UserControl_new, "(Parent)Container;"),

  GB_PROPERTY("_Container", "Container", UserContainer_Container),
	GB_PROPERTY("_Arrangement", "i", Container_Arrangement),

  GB_PROPERTY("Arrangement", "i", UserContainer_Arrangement),
  GB_PROPERTY("AutoResize", "b", UserContainer_AutoResize),
  GB_PROPERTY("Padding", "i", UserContainer_Padding),
  GB_PROPERTY("Spacing", "b", UserContainer_Spacing),
  GB_PROPERTY("Margin", "b", UserContainer_Margin),
  GB_PROPERTY("Indent", "b", UserContainer_Indent),
  GB_PROPERTY("Invert", "b", UserContainer_Invert),
  
  //GB_PROPERTY("Focus", "b", UserContainer_Focus),

	USERCONTAINER_DESCRIPTION,

  GB_END_DECLARE
};

