/***************************************************************************

  CTabStrip.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

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

#define __CTABSTRIP_CPP

#include "CPicture.h"
#include "CContainer.h"
#include "CTabStrip.h"

DECLARE_EVENT(EVENT_Click);

void gb_tabstrip_raise_click(CTABSTRIP *_object)
{
	GB.Raise(THIS, EVENT_Click, 0);
	GB.Unref(POINTER(&_object));
}

void gb_tabstrip_post_click(gTabStrip *sender)
{
	CWIDGET *_object = GetObject(sender);
	
	GB.Ref(THIS);
	GB.Post((GB_POST_FUNC)gb_tabstrip_raise_click, (long)THIS);
}

/***************************************************************************

  TabStrip

***************************************************************************/

BEGIN_METHOD(CTABSTRIP_new, GB_OBJECT parent)

	InitControl(new gTabStrip(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	TABSTRIP->onClick = gb_tabstrip_post_click;
	gb_tabstrip_post_click(TABSTRIP);

END_METHOD




BEGIN_PROPERTY(CTABSTRIP_tabs)

	if (READ_PROPERTY) { GB.ReturnInteger(TABSTRIP->count()); return; }
	
	if (VPROP(GB_INTEGER) < 1 || VPROP(GB_INTEGER) > 255)
	{
		GB.Error("Bad argument");
		return;
	}
	
	if (TABSTRIP->setCount(VPROP(GB_INTEGER))) 
		GB.Error("Tab is not empty"); 

END_PROPERTY



BEGIN_PROPERTY(CTABSTRIP_index)

	if (READ_PROPERTY) { GB.ReturnInteger(TABSTRIP->index()); return; }
	if ( (VPROP(GB_INTEGER)<0) || (VPROP(GB_INTEGER)>=TABSTRIP->count()) )
	{
		GB.Error("Bad index");
		return;
	}
	TABSTRIP->setIndex(VPROP(GB_INTEGER));
	

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_current)

	RETURN_SELF();

END_PROPERTY


BEGIN_METHOD(CTABSTRIP_get, GB_INTEGER index)

	if ( (VARG(index)<0) || (VARG(index)>=TABSTRIP->count()) )
	{
		GB.Error("Bad index");
		return;
	}
	
	THIS->index=VARG(index);
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTABSTRIP_orientation)

	if (READ_PROPERTY) { GB.ReturnBoolean(TABSTRIP->orientation()); return; }
	TABSTRIP->setOrientation(VPROP(GB_BOOLEAN));

END_PROPERTY



/***************************************************************************

  .Tab

***************************************************************************/

BEGIN_PROPERTY(CTAB_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TABSTRIP->tabText(THIS->index));
	else
		TABSTRIP->setTabText(THIS->index, GB.ToZeroString(PROP(GB_STRING)));
		
END_PROPERTY


BEGIN_PROPERTY(CTAB_picture)

	if (READ_PROPERTY)
	{
		gPicture *pic = TABSTRIP->tabPicture(THIS->index);;
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		TABSTRIP->setTabPicture(THIS->index, pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CTAB_enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(TABSTRIP->tabEnabled(THIS->index));
	else
		TABSTRIP->setTabEnabled(THIS->index,VPROP(GB_BOOLEAN));		

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_next)

	int *ct = (int *)GB.GetEnum();
	
	if (*ct >= TABSTRIP->tabCount(THIS->index)) 
	{ 
		GB.StopEnum(); 
		return; 
	}
	
	GB.ReturnObject(GetObject(TABSTRIP->tabChild(THIS->index, *ct)));
	(*ct)++;

END_METHOD


BEGIN_PROPERTY(CTAB_count)

	GB.ReturnInteger(TABSTRIP->tabCount(THIS->index));

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TABSTRIP->tabText(TABSTRIP->index()));
	else
		TABSTRIP->setTabText(TABSTRIP->index(), GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_picture)

	int index = TABSTRIP->index();

	if (READ_PROPERTY)
	{
		gPicture *pic = TABSTRIP->tabPicture(index);;
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		TABSTRIP->setTabPicture(index, pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_delete)

	if (TABSTRIP->removeTab(THIS->index))
		GB.Error("Tab is not empty");

END_METHOD

BEGIN_PROPERTY(CTAB_visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(TABSTRIP->tabVisible(THIS->index));
	else
		TABSTRIP->setTabVisible(THIS->index, VPROP(GB_BOOLEAN));		

END_PROPERTY


/***************************************************************************

  Descriptions

***************************************************************************/

GB_DESC CTabChildrenDesc[] =
{
  GB_DECLARE(".TabChildren", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CTAB_next, NULL),
  GB_PROPERTY_READ("Count", "i", CTAB_count),

  GB_END_DECLARE
};


GB_DESC CTabDesc[] =
{
  GB_DECLARE(".Tab", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTAB_text),
  GB_PROPERTY("Picture", "Picture", CTAB_picture),
  GB_PROPERTY("Caption", "s", CTAB_text),
  GB_PROPERTY("Enabled", "b", CTAB_enabled),
  GB_PROPERTY("Visible", "b", CTAB_visible),
  GB_PROPERTY_SELF("Children", ".TabChildren"),
  GB_METHOD("Delete", NULL, CTAB_delete, NULL),

  GB_END_DECLARE
};


GB_DESC CTabStripDesc[] =
{
  GB_DECLARE("TabStrip", sizeof(CTABSTRIP)), GB_INHERITS("Container"),

  GB_CONSTANT("Top", "i", 0),
  GB_CONSTANT("Bottom", "i", 1),

  GB_METHOD("_new", NULL, CTABSTRIP_new, "(Parent)Container;"),

  GB_PROPERTY("Count", "i", CTABSTRIP_tabs),
  GB_PROPERTY("Text", "s", CTABSTRIP_text),
  GB_PROPERTY("Picture", "Picture", CTABSTRIP_picture),
  GB_PROPERTY("Caption", "s", CTABSTRIP_text),
  GB_PROPERTY_READ("Current", ".Tab", CTABSTRIP_current),
  GB_PROPERTY("Index", "i", CTABSTRIP_index),
  GB_PROPERTY("Orientation", "i", CTABSTRIP_orientation),
  
  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

  GB_METHOD("_get", ".Tab", CTABSTRIP_get, "(Index)i"),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  
  TABSTRIP_DESCRIPTION,

  GB_END_DECLARE
};





