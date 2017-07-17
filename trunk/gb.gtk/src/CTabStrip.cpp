/***************************************************************************

	CTabStrip.cpp

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

#define __CTABSTRIP_CPP

#include "CPicture.h"
#include "CContainer.h"
#include "CFont.h"
#include "CTabStrip.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Close);

static void gb_tabstrip_raise_click(CTABSTRIP *_object)
{
	GB.Raise(THIS, EVENT_Click, 0);
	GB.Unref(POINTER(&_object));
}

static void gb_tabstrip_post_click(gTabStrip *sender)
{
	CWIDGET *_object = GetObject(sender);
	
	GB.Ref(THIS);
	GB.Post((GB_CALLBACK)gb_tabstrip_raise_click, (long)THIS);
}

static void handle_close(gTabStrip *sender, int index)
{
	CWIDGET *_object = GetObject(sender);
	GB.Raise(THIS, EVENT_Close, 1, GB_T_INTEGER, index);
}

/***************************************************************************

	TabStrip

***************************************************************************/

BEGIN_METHOD(CTABSTRIP_new, GB_OBJECT parent)

	InitControl(new gTabStrip(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	TABSTRIP->onClick = gb_tabstrip_post_click;
	TABSTRIP->onClose = handle_close;
	gb_tabstrip_post_click(TABSTRIP);

END_METHOD


BEGIN_METHOD_VOID(TabStrip_free)

	GB.Unref(POINTER(&THIS->textFont));

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

	THIS->index = TABSTRIP->index();
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

	if (READ_PROPERTY)
		switch (TABSTRIP->orientation())
		{ 
			case GTK_POS_TOP: GB.ReturnInteger(ALIGN_TOP); break;
			case GTK_POS_BOTTOM: GB.ReturnInteger(ALIGN_BOTTOM); break;
			case GTK_POS_LEFT: GB.ReturnInteger(ALIGN_LEFT); break;
			case GTK_POS_RIGHT: GB.ReturnInteger(ALIGN_RIGHT); break;
			default: GB.ReturnInteger(ALIGN_NORMAL); break;
		}
		
	else 
				switch (VPROP(GB_INTEGER))
				{
		case ALIGN_TOP: TABSTRIP->setOrientation(GTK_POS_TOP); break;
		case ALIGN_BOTTOM: TABSTRIP->setOrientation(GTK_POS_BOTTOM); break;
		case ALIGN_LEFT: TABSTRIP->setOrientation(GTK_POS_LEFT); break;
		case ALIGN_RIGHT: TABSTRIP->setOrientation(GTK_POS_RIGHT); break;
		
				}
				
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


BEGIN_PROPERTY(TabStrip_TextFont)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->textFont);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->textFont));
		CFONT *font = (CFONT *)THIS->textFont;
		if (font)
			TABSTRIP->setTextFont(font->font);
		else
			TABSTRIP->setTextFont(0);
	}

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

BEGIN_METHOD(CTAB_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= TABSTRIP->tabCount(THIS->index))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	GB.ReturnObject(GetObject(TABSTRIP->tabChild(THIS->index, index)));

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
		gPicture *pic = TABSTRIP->tabPicture(index);
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		TABSTRIP->setTabPicture(index, pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_PROPERTY(TabStrip_Closable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(TABSTRIP->isClosable());
	else
		TABSTRIP->setClosable(VPROP(GB_BOOLEAN));
	
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

BEGIN_METHOD(TabStrip_FindIndex, GB_OBJECT child)

	CWIDGET *child = (CWIDGET *)VARG(child);
	
	if (GB.CheckObject(child))
		return;
	
	GB.ReturnInteger(TABSTRIP->findIndex(child->widget));
	
END_METHOD


/***************************************************************************

	Descriptions

***************************************************************************/

GB_DESC CTabStripContainerChildrenDesc[] =
{
	GB_DECLARE_VIRTUAL(".TabStripContainer.Children"),

	GB_METHOD("_next", "Control", CTAB_next, NULL),
	GB_PROPERTY_READ("Count", "i", CTAB_count),
	GB_METHOD("_get", "Control", CTAB_next, "(Index)i"),

	GB_END_DECLARE
};


GB_DESC CTabStripContainerDesc[] =
{
	GB_DECLARE_VIRTUAL(".TabStripContainer"),

	GB_PROPERTY("Text", "s", CTAB_text),
	GB_PROPERTY("Picture", "Picture", CTAB_picture),
	GB_PROPERTY("Caption", "s", CTAB_text),
	GB_PROPERTY("Enabled", "b", CTAB_enabled),
	GB_PROPERTY("Visible", "b", CTAB_visible),
	GB_PROPERTY_SELF("Children", ".TabStripContainer.Children"),
	GB_METHOD("Delete", 0, CTAB_delete, 0),

	GB_END_DECLARE
};


GB_DESC CTabStripDesc[] =
{
	GB_DECLARE("TabStrip", sizeof(CTABSTRIP)), GB_INHERITS("Container"),

	GB_METHOD("_new", NULL, CTABSTRIP_new, "(Parent)Container;"),
	GB_METHOD("_free", NULL, TabStrip_free, NULL),

	GB_PROPERTY("Count", "i", CTABSTRIP_tabs),
	GB_PROPERTY("Text", "s", CTABSTRIP_text),
	GB_PROPERTY("TextFont", "Font", TabStrip_TextFont),
	GB_PROPERTY("Picture", "Picture", CTABSTRIP_picture),
	GB_PROPERTY("Closable", "b", TabStrip_Closable),
	GB_PROPERTY("Caption", "s", CTABSTRIP_text),
	GB_PROPERTY_READ("Current", ".TabStripContainer", CTABSTRIP_current),
	GB_PROPERTY("Index", "i", CTABSTRIP_index),
	GB_PROPERTY("Orientation", "i", CTABSTRIP_orientation),
	
	GB_PROPERTY("Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("Padding", "i", Container_Padding),
	GB_PROPERTY("Spacing", "b", Container_Spacing),
	GB_PROPERTY("Margin", "b", Container_Margin),
	GB_PROPERTY("Indent", "b", Container_Indent),
	GB_PROPERTY("Invert", "b", Container_Invert),

	GB_METHOD("_get", ".TabStripContainer", CTABSTRIP_get, "(Index)i"),
	GB_METHOD("FindIndex", "i", TabStrip_FindIndex, "(Child)Control;"),

	GB_EVENT("Click", NULL, NULL, &EVENT_Click),
	GB_EVENT("Close", NULL, "(Index)i", &EVENT_Close),
	
	TABSTRIP_DESCRIPTION,

	GB_END_DECLARE
};

