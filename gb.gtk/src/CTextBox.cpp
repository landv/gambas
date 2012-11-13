/***************************************************************************

  CTextBox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CTEXTBOX_CPP

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CTextBox.h"
#include "CWidget.h"
#include "CContainer.h"


DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Click);

/*static void txt_post_change(void *_object)
{
	GB.Raise(THIS, EVENT_Change, 0);
	GB.Unref(POINTER(&_object));
}*/

static void txt_raise_change(gTextBox *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	GB.Raise(THIS, EVENT_Change, 0);
}

static void txt_raise_activate(gTextBox *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	GB.Raise(THIS, EVENT_Activate, 0);
}

/***************************************************************************

  TextBox

***************************************************************************/

#define CHECK_COMBOBOX() \
	if (!TEXTBOX->hasEntry()) \
	{ \
		GB.Error("ComboBox is read-only"); \
		return; \
	}

BEGIN_METHOD(CTEXTBOX_new, GB_OBJECT parent)

	InitControl(new gTextBox(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	TEXTBOX->onChange = txt_raise_change;
	TEXTBOX->onActivate = txt_raise_activate;
	
END_METHOD


BEGIN_METHOD_VOID(CTEXTBOX_clear)

	TEXTBOX->clear();

END_METHOD


BEGIN_METHOD(CTEXTBOX_insert, GB_STRING text)

	CHECK_COMBOBOX();
	TEXTBOX->insert(STRING(text),LENGTH(text));

END_METHOD


BEGIN_PROPERTY(CTEXTBOX_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TEXTBOX->text());
	else
		TEXTBOX->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_length)

	GB.ReturnInteger(TEXTBOX->length());

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_alignment)

	if (READ_PROPERTY) { GB.ReturnInteger(TEXTBOX->alignment()); return; }
	TEXTBOX->setAlignment(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_pos)

	CHECK_COMBOBOX();
	if (READ_PROPERTY) { GB.ReturnInteger(TEXTBOX->position()); return; }
	TEXTBOX->setPosition(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_read_only)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTBOX->isReadOnly()); return; }
	TEXTBOX->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_border)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTBOX->hasBorder()); return; }
	TEXTBOX->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_password)

	CHECK_COMBOBOX();
	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTBOX->password()); return; }
	TEXTBOX->setPassword(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_max_length)

	CHECK_COMBOBOX();
	if (READ_PROPERTY) { GB.ReturnInteger(TEXTBOX->maxLength()); return; }
	TEXTBOX->setMaxLength(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTBOX_selected)

	CHECK_COMBOBOX();
  GB.ReturnBoolean(TEXTBOX->isSelected());

END_METHOD

/***************************************************************************

  .TextBox.Selection

***************************************************************************/

BEGIN_PROPERTY(CTEXTBOX_sel_text)

	char *buf;
	
	CHECK_COMBOBOX();
	
	if (READ_PROPERTY)
	{
		buf=TEXTBOX->selText();
		GB.ReturnNewZeroString(buf);
		g_free(buf);
		return;
	}
	
	buf=GB.ToZeroString(PROP(GB_STRING));
	TEXTBOX->setSelText(buf,strlen(buf));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_sel_length)

	CHECK_COMBOBOX();
	GB.ReturnInteger(TEXTBOX->selLength());

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_sel_start)

	CHECK_COMBOBOX();
	GB.ReturnInteger(TEXTBOX->selStart());

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTBOX_sel_clear)

	CHECK_COMBOBOX();
	TEXTBOX->selClear();

END_METHOD

BEGIN_METHOD_VOID(CTEXTBOX_sel_all)

	CHECK_COMBOBOX();
	TEXTBOX->selectAll();

END_METHOD

BEGIN_METHOD(CTEXTBOX_sel_select, GB_INTEGER start; GB_INTEGER length)

	CHECK_COMBOBOX();
	TEXTBOX->select(VARG(start),VARG(length));

END_METHOD



/***************************************************************************

  ComboBox

***************************************************************************/

#undef THIS
#define THIS ((CCOMBOBOX *)_object)

static void cmb_raise_click(gComboBox *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	if (THIS->click)
		return;
	THIS->click = true;
	GB.Raise(THIS, EVENT_Click, 0);
	THIS->click = false;
}


BEGIN_METHOD(CCOMBOBOX_new, GB_OBJECT parent)

	InitControl(new gComboBox(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	
	COMBOBOX->onClick = cmb_raise_click;
	COMBOBOX->onChange = txt_raise_change;
	COMBOBOX->onActivate = txt_raise_activate;

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(COMBOBOX->text());
	else
		COMBOBOX->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(CCOMBOBOX_popup)

	COMBOBOX->popup();

END_METHOD


BEGIN_METHOD(CCOMBOBOX_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= COMBOBOX->count())
	{
		GB.Error("Bad index");
		return;
	}

	THIS->index = index;
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_item_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(COMBOBOX->itemText(THIS->index));
	else	
		COMBOBOX->setItemText(THIS->index,GB.ToZeroString(PROP(GB_STRING)));


END_PROPERTY


BEGIN_METHOD(CCOMBOBOX_add, GB_STRING item; GB_INTEGER pos)

	int Pos;
	char *Item=GB.ToZeroString(ARG(item));

	if (MISSING(pos)) Pos=COMBOBOX->count();
	else Pos=VARG(pos);
	
	COMBOBOX->add(Item,Pos);

END_METHOD


BEGIN_METHOD(CCOMBOBOX_remove, GB_INTEGER pos)

	COMBOBOX->remove(VARG(pos));

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_sorted)

	if (READ_PROPERTY) { GB.ReturnBoolean(COMBOBOX->isSorted()); return; }
	COMBOBOX->setSorted(VPROP(GB_BOOLEAN));

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_count)

	GB.ReturnInteger(COMBOBOX->count());

END_PROPERTY


BEGIN_PROPERTY(CCOMBOBOX_index)

	if (READ_PROPERTY) { GB.ReturnInteger(COMBOBOX->index()); return; }
	COMBOBOX->setIndex(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CCOMBOBOX_current)

	if (!COMBOBOX->count()) { GB.ReturnNull(); return; }
	THIS->index = COMBOBOX->index();
	RETURN_SELF();

END_PROPERTY

BEGIN_METHOD(CCOMBOBOX_find, GB_STRING item)

	GB.ReturnInteger(COMBOBOX->find(GB.ToZeroString(ARG(item))));

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_list)

	GB_ARRAY array;
	int i;
	
	if (READ_PROPERTY)
	{
		GB.Array.New(&array, GB_T_STRING, COMBOBOX->count());
		for (i = 0; i < COMBOBOX->count(); i++)
		{
			*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(COMBOBOX->itemText(i));
		}
		
		GB.ReturnObject(array);
	}
	else
	{
		char *text = GB.NewZeroString(COMBOBOX->text());
		
		array = VPROP(GB_OBJECT);
		COMBOBOX->lock();
		COMBOBOX->clear();
		if (array)
		{
			for (i = 0; i < GB.Array.Count(array); i++)
				COMBOBOX->add(*((char **)GB.Array.Get(array, i)));
		}
		COMBOBOX->setText(text);
		COMBOBOX->unlock();

		GB.FreeString(&text);
		
		if (COMBOBOX->isReadOnly())
		{
			if (COMBOBOX->index() < 0 && COMBOBOX->count() > 0)
				COMBOBOX->setIndex(0);
		}
	}

END_PROPERTY

BEGIN_PROPERTY(ComboBox_Border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COMBOBOX->hasBorder());
	else
		COMBOBOX->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

/***************************************************************************

  Descriptions

***************************************************************************/

GB_DESC CTextBoxSelectionDesc[] =
{
  GB_DECLARE(".TextBox.Selection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTEXTBOX_sel_text),
  GB_PROPERTY_READ("Length", "i", CTEXTBOX_sel_length),
  GB_PROPERTY_READ("Start", "i", CTEXTBOX_sel_start),
  GB_PROPERTY_READ("Pos", "i", CTEXTBOX_sel_start),

  GB_METHOD("Hide", 0, CTEXTBOX_sel_clear, 0),

  GB_END_DECLARE
};

GB_DESC CTextBoxDesc[] =
{
  GB_DECLARE("TextBox", sizeof(CTEXTBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CTEXTBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CTEXTBOX_text),
  GB_PROPERTY("Alignment", "i", CTEXTBOX_alignment),
  GB_PROPERTY_READ("Length", "i", CTEXTBOX_length),
  GB_PROPERTY("Pos", "i", CTEXTBOX_pos),
  GB_PROPERTY("ReadOnly", "b", CTEXTBOX_read_only),
  GB_PROPERTY("Border", "b", CTEXTBOX_border),
  GB_PROPERTY("Password", "b", CTEXTBOX_password),
  GB_PROPERTY("MaxLength", "i", CTEXTBOX_max_length),

  GB_PROPERTY_SELF("Selection", ".TextBox.Selection"),
  GB_METHOD("Select", 0, CTEXTBOX_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", 0, CTEXTBOX_sel_all, 0),
  GB_METHOD("Unselect", 0, CTEXTBOX_sel_clear, 0),
  GB_PROPERTY_READ("Selected", "b", CTEXTBOX_selected),

  GB_METHOD("Clear", 0, CTEXTBOX_clear, 0),
  GB_METHOD("Insert", 0, CTEXTBOX_insert, "(Text)s"),

  GB_EVENT("Change", 0, 0, &EVENT_Change),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  
  TEXTBOX_DESCRIPTION,

  GB_END_DECLARE
};



GB_DESC CComboBoxItemDesc[] =
{
  GB_DECLARE(".ComboBox.Item", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CCOMBOBOX_item_text),

  GB_END_DECLARE
};


GB_DESC CComboBoxDesc[] =
{
  GB_DECLARE("ComboBox", sizeof(CCOMBOBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CCOMBOBOX_new, "(Parent)Container;"),
  GB_METHOD("_get", ".ComboBox.Item", CCOMBOBOX_get, "(Index)i"),
  GB_METHOD("Popup", 0, CCOMBOBOX_popup, 0),
  GB_METHOD("Clear", 0, CTEXTBOX_clear, 0),
  GB_METHOD("Insert", 0, CTEXTBOX_insert, "(Text)s"),
  GB_METHOD("Add", 0, CCOMBOBOX_add, "(Item)s[(Index)i]"),
  GB_METHOD("Remove", 0, CCOMBOBOX_remove, "(Index)i"),
  GB_METHOD("Find", "i", CCOMBOBOX_find, "(Item)s"),

  GB_PROPERTY("Text", "s", CCOMBOBOX_text),
  GB_PROPERTY_READ("Length", "i", CTEXTBOX_length),
  GB_PROPERTY("Pos", "i", CTEXTBOX_pos),
  GB_PROPERTY("ReadOnly", "b", CTEXTBOX_read_only),
  GB_PROPERTY("Password", "b", CTEXTBOX_password),
  GB_PROPERTY("MaxLength", "i", CTEXTBOX_max_length),
  GB_PROPERTY("Border", "b", ComboBox_Border),
  
  GB_PROPERTY_SELF("Selection", ".TextBox.Selection"),
  GB_METHOD("Select", 0, CTEXTBOX_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", 0, CTEXTBOX_sel_all, 0),
  GB_METHOD("Unselect", 0, CTEXTBOX_sel_clear, 0),
  GB_PROPERTY_READ("Selected", "b", CTEXTBOX_selected),

  GB_PROPERTY("Sorted", "b", CCOMBOBOX_sorted),
  GB_PROPERTY("List", "String[]", CCOMBOBOX_list),
  GB_PROPERTY_READ("Count", "i", CCOMBOBOX_count),
  GB_PROPERTY_READ("Current", ".ComboBox.Item", CCOMBOBOX_current),
  GB_PROPERTY("Index", "i", CCOMBOBOX_index),

  GB_EVENT("Change", 0, 0, &EVENT_Change),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Click", 0, 0, &EVENT_Click),

  COMBOBOX_DESCRIPTION,

  GB_END_DECLARE
};


