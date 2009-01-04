/***************************************************************************

  CPanel.cpp

  The Panel class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CPANEL_CPP



#include <qapplication.h>
#include <qframe.h>
#include <qobjectlist.h>

#include "gambas.h"

#include "CConst.h"
#include "CPanel.h"


BEGIN_METHOD(CPANEL_new, GB_OBJECT parent)

  MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));
  THIS->container = wid;

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(CHBOX_new, GB_OBJECT parent)

  MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

  THIS->container = wid;
  THIS->arrangement.mode = ARRANGE_HORIZONTAL;
  //THIS->arrangement.autoresize = true;

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(CVBOX_new, GB_OBJECT parent)

  MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

  THIS->container = wid;
  THIS->arrangement.mode = ARRANGE_VERTICAL;
  //THIS->arrangement.autoresize = true;

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(CHPANEL_new, GB_OBJECT parent)

  MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

  THIS->container = wid;
  THIS->arrangement.mode = ARRANGE_ROW;
  //THIS->arrangement.autoresize = true;

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(CVPANEL_new, GB_OBJECT parent)

  MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

  THIS->container = wid;
  THIS->arrangement.mode = ARRANGE_COLUMN;
  //THIS->arrangement.autoresize = true;

  CWIDGET_new(wid, (void *)_object);

END_METHOD



GB_DESC CPanelDesc[] =
{
  GB_DECLARE("Panel", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

	PANEL_DESCRIPTION,
	
  GB_END_DECLARE
};


GB_DESC CHBoxDesc[] =
{
  GB_DECLARE("HBox", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHBOX_new, "(Parent)Container;"),

  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

	HBOX_DESCRIPTION,
	
  GB_END_DECLARE
};


GB_DESC CVBoxDesc[] =
{
  GB_DECLARE("VBox", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVBOX_new, "(Parent)Container;"),

  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
	GB_PROPERTY("Padding", "i", CCONTAINER_padding),

	VBOX_DESCRIPTION,
	
  GB_END_DECLARE
};


GB_DESC CHPanelDesc[] =
{
  GB_DECLARE("HPanel", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

	HPANEL_DESCRIPTION,
	
  GB_END_DECLARE
};


GB_DESC CVPanelDesc[] =
{
  GB_DECLARE("VPanel", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),

	VPANEL_DESCRIPTION,
	
  GB_END_DECLARE
};


