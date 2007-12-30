/***************************************************************************

  CListBox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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
  along with WIDGET program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/


#define __CLISTBOX_CPP


#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CListBox.h"
#include "CWidget.h"
#include "CContainer.h"

#include <stdlib.h>

#include <stdio.h>

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */

void gb_raise_listbox_Select(gListBox *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Select,0);
}

void gb_raise_listbox_Activate(gListBox *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Activate,0);
}


BEGIN_METHOD(CLISTBOX_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gListBox(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	LISTBOX->onSelect=gb_raise_listbox_Select;
	LISTBOX->onActivate=gb_raise_listbox_Activate;

END_METHOD


BEGIN_METHOD_VOID(CLISTBOX_clear)

	LISTBOX->clear();

END_METHOD


BEGIN_METHOD(CLISTBOX_get, GB_INTEGER index)

	long index = VARG(index);

	if (index < 0 || index >= LISTBOX->count())
	{
		GB.Error("Bad index");
		return;
	}

	THIS->index = index;
	RETURN_SELF();

END_METHOD



BEGIN_METHOD(CLISTBOX_add, GB_STRING item; GB_INTEGER pos)

	LISTBOX->add( GB.ToZeroString(PROP(GB_STRING)),0 );	

END_METHOD



BEGIN_METHOD(CLISTBOX_remove, GB_INTEGER pos)

	LISTBOX->remove(VARG(pos));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_sorted)

	if (READ_PROPERTY) { GB.ReturnBoolean(LISTBOX->sorted()); return; }
	LISTBOX->setSorted(VPROP(GB_BOOLEAN));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_count)

	GB.ReturnInteger(LISTBOX->count());

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_index)

	if (READ_PROPERTY) { GB.ReturnInteger(LISTBOX->index()); return; }
	LISTBOX->setIndex(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_current)

	THIS->index=LISTBOX->index();
	if (THIS->index>=0) RETURN_SELF();
	else GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_text)

	char *buf;
	
	buf=LISTBOX->text();
	GB.ReturnNewString(buf,0);
	if (buf) g_free(buf);

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_mode)

	if (READ_PROPERTY) { GB.ReturnInteger(LISTBOX->mode()); return; }
	LISTBOX->setMode(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_selected)

	if (READ_PROPERTY) { GB.ReturnBoolean(LISTBOX->itemSelected(THIS->index)); return; }
	LISTBOX->setItemSelected(THIS->index,VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_text)

	char *buf;

	if (READ_PROPERTY)
	{
		buf=LISTBOX->itemText(THIS->index);
		GB.ReturnNewString(buf,0);
		if (buf) g_free(buf);
		return;
	}
	
	LISTBOX->setItemText(THIS->index,GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD(CLISTBOX_find, GB_STRING item)

	GB.ReturnInteger(LISTBOX->find(GB.ToZeroString(ARG(item))));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_list)

	GB_ARRAY Array=NULL;
	char **buf=NULL;
	char *ctmp;
	long b=0,ct=0;
	
	if (READ_PROPERTY)
	{
		buf=LISTBOX->list();
		if (!buf) 
		{
			GB.Array.New(&Array,GB_T_STRING,0);
			GB.ReturnObject(Array);
			return;
		}

		while (buf[b++]) ct++;
		GB.Array.New(&Array,GB_T_STRING,ct);
		b=0;
		while (buf[b])
		{
			ctmp=NULL;
			GB.NewString(&ctmp,buf[b],strlen(buf[b]));
			*((char **)GB.Array.Get(Array,b++)) = ctmp;
		} 
		b=0;
		while(buf[b]) g_free(buf[b++]);
		g_free(buf);
		GB.ReturnObject(Array);
		return;
	}

	
	
	Array=VPROP(GB_OBJECT);
	ct=GB.Array.Count(Array);
	if (!ct) LISTBOX->setList(NULL);
	buf=(char**)g_malloc(sizeof(char*)*(ct+1));
	buf[ct]=NULL;
	if (buf)
	{
		for (b=0;b<ct;b++)
		{	
			buf[b]=*((char **)GB.Array.Get(Array,b)); 
			if (!buf[b]) buf[b]="";
		}
	}
	LISTBOX->setList(buf);
	if (buf) g_free(buf);


END_PROPERTY

/***************************************************************************

  ListBoxItem

***************************************************************************/

GB_DESC CListBoxItemDesc[] =
{
  GB_DECLARE(".ListBoxItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CLISTBOX_item_text),
  GB_PROPERTY("Selected", "b", CLISTBOX_item_selected),

  GB_END_DECLARE
};

/***************************************************************************

  ListBox

***************************************************************************/

GB_DESC CListBoxDesc[] =
{
  GB_DECLARE("ListBox", sizeof(CLISTBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CLISTBOX_new, "(Parent)Container;"),
  GB_METHOD("_get", ".ListBoxItem", CLISTBOX_get, "(Index)i"),
  GB_METHOD("Clear", NULL, CLISTBOX_clear, NULL),
  GB_METHOD("Add", NULL, CLISTBOX_add, "(Text)s[(After)i]"),
  GB_METHOD("Remove", NULL, CLISTBOX_remove, "(Index)i"),
  GB_METHOD("Find", "i", CLISTBOX_find, "(Item)s"),

  GB_PROPERTY("Sorted", "b", CLISTBOX_sorted),
  GB_PROPERTY("List[]", "String[]", CLISTBOX_list),
  GB_PROPERTY_READ("Count", "i", CLISTBOX_count),
  GB_PROPERTY_READ("Current", ".ListBoxItem", CLISTBOX_current),
  GB_PROPERTY_READ("Text", "s", CLISTBOX_text),
  GB_PROPERTY("Index", "i", CLISTBOX_index),
  GB_PROPERTY("Mode", "i", CLISTBOX_mode),

  GB_CONSTANT("_Properties", "s", CLISTBOX_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};



