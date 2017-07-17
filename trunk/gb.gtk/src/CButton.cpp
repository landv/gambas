/***************************************************************************

  CButton.cpp

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

#define __CBUTTON_CPP

#include "CButton.h"
#include "CContainer.h"
#include "CPicture.h"

DECLARE_EVENT(EVENT_Click);

void gb_raise_button_Click(gControl *sender)
{
	CWIDGET *ob = GetObject(sender);
	
	if (!ob) return;
	GB.Ref(ob);
	GB.Raise((void*)ob,EVENT_Click,0);
	CACTION_raise(ob);
	GB.Unref(POINTER(&ob));
}

/***************************************************************

CONSTRUCTORS

****************************************************************/

BEGIN_METHOD(CBUTTON_new, GB_OBJECT parent)

	InitControl(new gButton(CONTAINER(VARG(parent)), gButton::Button), (CWIDGET*)THIS);
	BUTTON->onClick=gb_raise_button_Click;

END_METHOD


BEGIN_METHOD(CTOGGLEBUTTON_new, GB_OBJECT parent)

	InitControl(new gButton(CONTAINER(VARG(parent)), gButton::Toggle), (CWIDGET*)THIS);
	BUTTON->onClick=gb_raise_button_Click;

END_METHOD

BEGIN_METHOD(CCHECKBOX_new, GB_OBJECT parent)

	InitControl(new gButton(CONTAINER(VARG(parent)), gButton::Check), (CWIDGET*)THIS);
	BUTTON->onClick = gb_raise_button_Click;

END_METHOD

BEGIN_METHOD(CRADIOBUTTON_new, GB_OBJECT parent)
	
	InitControl(new gButton(CONTAINER(VARG(parent)), gButton::Radio), (CWIDGET*)THIS);
	BUTTON->onClick=gb_raise_button_Click;

END_METHOD

BEGIN_METHOD(CTOOLBUTTON_new, GB_OBJECT parent)

	InitControl(new gButton(CONTAINER(VARG(parent)), gButton::Tool), (CWIDGET*)THIS);
	BUTTON->onClick=gb_raise_button_Click;

END_METHOD


BEGIN_PROPERTY(CBUTTON_text)

	if (READ_PROPERTY) { GB.ReturnNewZeroString(BUTTON->text()); return; }
	BUTTON->setText((const char*)GB.ToZeroString(PROP(GB_STRING)));
	
END_PROPERTY


BEGIN_PROPERTY(CBUTTON_picture)

	if (READ_PROPERTY)
	{
		gPicture *pic = BUTTON->picture();
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		BUTTON->setPicture(pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_border)

	if (READ_PROPERTY) { GB.ReturnBoolean(BUTTON->getBorder()); return; }
	BUTTON->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_value)

	if (READ_PROPERTY) { GB.ReturnBoolean(BUTTON->value()); return; }
	BUTTON->setValue(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CBUTTON_default)

	if (READ_PROPERTY) { GB.ReturnBoolean(BUTTON->isDefault()); return; }
	BUTTON->setDefault(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_cancel)

	if (READ_PROPERTY) { GB.ReturnBoolean(BUTTON->isCancel()); return; }
	BUTTON->setCancel(VPROP(GB_BOOLEAN));	

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(BUTTON->isToggle());
	else
		BUTTON->setToggle(VPROP(GB_INTEGER));	

END_PROPERTY

BEGIN_PROPERTY(CCHECKBOX_tristate)

	if (READ_PROPERTY) 
		GB.ReturnBoolean(BUTTON->isTristate());
	else
		BUTTON->setTristate(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCHECKBOX_value)

	if (READ_PROPERTY) 
	{ 
		if (BUTTON->isTristate() && BUTTON->inconsistent())
			GB.ReturnInteger(1);
		else
			GB.ReturnInteger(BUTTON->value() ? -1 : 0);
	}
	else
	{
		if (BUTTON->isTristate() && VPROP(GB_INTEGER) == 1) 
			BUTTON->setInconsistent(true);
		else
		{
			BUTTON->setInconsistent(false);
			BUTTON->setValue(VPROP(GB_INTEGER));
		}
	}

END_PROPERTY

BEGIN_PROPERTY(CBUTTON_radio)

	if (READ_PROPERTY)
		GB.ReturnBoolean(BUTTON->isRadio());
	else
		BUTTON->setRadio(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CBUTTON_autoresize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(BUTTON->isAutoResize());
	else
		BUTTON->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

GB_DESC CButtonDesc[] =
{
  GB_DECLARE("Button", sizeof(CBUTTON)), GB_INHERITS("Control"),
  
  GB_METHOD("_new", 0, CBUTTON_new, "(Parent)Container;"),
  
  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CBUTTON_picture),

  GB_PROPERTY("Border", "b", CBUTTON_border),
  GB_PROPERTY("Default", "b", CBUTTON_default),
  GB_PROPERTY("Cancel", "b", CBUTTON_cancel),
  GB_PROPERTY("Value", "b", CBUTTON_value),
	GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

  GB_EVENT("Click", 0, 0, &EVENT_Click),

	BUTTON_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CToggleButtonDesc[] =
{
  GB_DECLARE("ToggleButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CTOGGLEBUTTON_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CBUTTON_picture),
  GB_PROPERTY("Value", "b", CBUTTON_value),
  GB_PROPERTY("Border", "b", CBUTTON_border),
  GB_PROPERTY("Radio", "b", CBUTTON_radio),
	GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

  GB_EVENT("Click", 0, 0, &EVENT_Click),

	TOGGLEBUTTON_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CRadioButtonDesc[] =
{
  GB_DECLARE("RadioButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CRADIOBUTTON_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Value", "b", CBUTTON_value),
	GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

  GB_EVENT("Click", 0, 0, &EVENT_Click),

	RADIOBUTTON_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CCheckBoxDesc[] =
{
  GB_DECLARE("CheckBox", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_CONSTANT("False", "i", 0),
  GB_CONSTANT("True", "i", -1),
  GB_CONSTANT("None", "i", 1),

  GB_METHOD("_new", 0, CCHECKBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Tristate", "b", CCHECKBOX_tristate),
	GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

  GB_PROPERTY("Value", "i", CCHECKBOX_value),

  GB_EVENT("Click", 0, 0, &EVENT_Click),

	CHECKBOX_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CToolButtonDesc[] =
{
  GB_DECLARE("ToolButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CTOOLBUTTON_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CBUTTON_picture),
  GB_PROPERTY("Value", "b", CBUTTON_value),
  GB_PROPERTY("Toggle", "b", CTOOLBUTTON_toggle),
  GB_PROPERTY("Border", "b", CBUTTON_border),
  GB_PROPERTY("Radio", "b", CBUTTON_radio),
	GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

  GB_EVENT("Click", 0, 0, &EVENT_Click),

	TOOLBUTTON_DESCRIPTION,

  GB_END_DECLARE
};



