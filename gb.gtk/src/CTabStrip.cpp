/***************************************************************************

  CTabStrip.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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


#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CPicture.h"
#include "CContainer.h"
#include "CTabStrip.h"

#include <stdlib.h>

DECLARE_EVENT(EVENT_Click);

void gb_tabstrip_raise_click(gTabStrip *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Click,0);
}

void gb_tabstrip_post_click(gTabStrip *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Post((void (*)())gb_tabstrip_raise_click,(long)sender);
}

/***************************************************************************

  TabStrip

***************************************************************************/

BEGIN_METHOD(CTABSTRIP_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gTabStrip(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	TABSTRIP->onClick=gb_tabstrip_post_click;
	GB.Post((void (*)())gb_tabstrip_raise_click,(long)TABSTRIP);

END_METHOD




BEGIN_PROPERTY(CTABSTRIP_tabs)

	if (READ_PROPERTY) { GB.ReturnInteger(TABSTRIP->count()); return; }
	
	if (VPROP(GB_INTEGER)<1)
	{
		GB.Error("Bad argument");
		return;
	}
	
	if (TABSTRIP->setCount(VPROP(GB_INTEGER))) GB.Error("Tab is not empty"); 

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

	char *buf;

	if (READ_PROPERTY)
	{
		buf=TABSTRIP->text(THIS->index);
		GB.ReturnNewString( buf,0 );
		if (buf) free(buf);
		return;
	}
	TABSTRIP->setText(THIS->index,GB.ToZeroString(PROP(GB_STRING)));
		
END_PROPERTY


BEGIN_PROPERTY(CTAB_picture)

	CPICTURE *pic=NULL;
	gPicture *hPic=NULL;

	if (READ_PROPERTY)
	{
		hPic=TABSTRIP->picture(THIS->index);
		if (hPic)
		{
			GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
			if (pic->picture) pic->picture->unref();
			pic->picture=hPic;
			GB.ReturnObject((void*)pic);
		}
		GB.ReturnObject(pic);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (!pic)
		TABSTRIP->setPicture(THIS->index,NULL);
	else
		TABSTRIP->setPicture(THIS->index,pic->picture);

END_PROPERTY


BEGIN_PROPERTY(CTAB_enabled)

	if (READ_PROPERTY) { GB.ReturnBoolean(TABSTRIP->tabEnabled(THIS->index)); return;}
	TABSTRIP->setTabEnabled(THIS->index,VPROP(GB_BOOLEAN));		

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_next)

	stub ("CTAB_next");

END_METHOD


BEGIN_PROPERTY(CTAB_count)

	stub ("CTAB_count");

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_text)

	char *buf;
	
	if (READ_PROPERTY)
	{
		buf=TABSTRIP->text(TABSTRIP->index());
		GB.ReturnNewString( buf,0 );
		if (buf) free(buf);
		return;
	}
	TABSTRIP->setText(TABSTRIP->index(),GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_picture)

	CPICTURE *pic=NULL;
	gPicture *hPic=NULL;

	if (READ_PROPERTY)
	{
		hPic=TABSTRIP->picture(TABSTRIP->index());
		if (hPic)
		{
			GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
			if (pic->picture) pic->picture->unref();
			pic->picture=hPic;
			GB.ReturnObject((void*)pic);
		}
		GB.ReturnObject(pic);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (!pic)
		TABSTRIP->setPicture(TABSTRIP->index(),NULL);
	else
		TABSTRIP->setPicture(TABSTRIP->index(),pic->picture);

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
  GB_PROPERTY_SELF("Children", ".TabChildren"),

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
  GB_PROPERTY("Orientation", "i<TabStrip>", CTABSTRIP_orientation),
  
  GB_PROPERTY("Arrangement", "i<Arrange>", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

  GB_METHOD("_get", ".Tab", CTABSTRIP_get, "(Index)i"),

  GB_CONSTANT("_Properties", "s", CTABSTRIP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};





