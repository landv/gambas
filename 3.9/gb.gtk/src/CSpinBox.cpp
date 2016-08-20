/***************************************************************************

  CSpinBox.cpp

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

#define __CSPINBOX_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"

#include "CSpinBox.h"
#include "CWidget.h"
#include "CContainer.h"

DECLARE_EVENT(EVENT_Change);

/*static void raise_change(CWIDGET *_object)
{
	GB.Raise(THIS, EVENT_Change, 0);
	GB.Unref(POINTER(&_object));
}*/

static void cb_change(gSpinBox *sender)
{
	CWIDGET *_object = GetObject(sender);
	GB.Raise(THIS, EVENT_Change, 0);
	//GB.Ref(THIS);
	//GB.Post((GB_POST_FUNC)raise_change, (long)THIS);
}

BEGIN_METHOD(CSPINBOX_new, GB_OBJECT parent)
	
	InitControl(new gSpinBox(CONTAINER(VARG(parent))),(CWIDGET*)THIS);
	
	SPINBOX->onChange = cb_change;

END_METHOD


BEGIN_PROPERTY(CSPINBOX_wrapping)

	if (READ_PROPERTY) { GB.ReturnBoolean(SPINBOX->wrap()); return; }
	SPINBOX->setWrap(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_value)

	if (READ_PROPERTY) { GB.ReturnInteger(SPINBOX->value()); return; }
	SPINBOX->setValue(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSPINBOX_minval)

	if (READ_PROPERTY) { GB.ReturnInteger(SPINBOX->minValue()); return; }
	SPINBOX->setMinValue(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSPINBOX_maxval)

	if (READ_PROPERTY) { GB.ReturnInteger(SPINBOX->maxValue()); return; }
	SPINBOX->setMaxValue(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSPINBOX_linestep)

	if (READ_PROPERTY) { GB.ReturnInteger(SPINBOX->step()); return; }
	SPINBOX->setStep(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_text)

	GB.ReturnInteger(SPINBOX->value());

END_PROPERTY

BEGIN_METHOD_VOID(CSPINBOX_select_all)

	SPINBOX->selectAll();	

END_METHOD

BEGIN_PROPERTY(CSPINBOX_border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(SPINBOX->hasBorder());
	else
		SPINBOX->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

GB_DESC CSpinBoxDesc[] =
{
  GB_DECLARE("SpinBox", sizeof(CSPINBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CSPINBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Value", "i", CSPINBOX_value),
  GB_PROPERTY_READ("Text", "s", CSPINBOX_text),

  GB_PROPERTY("MinValue", "i", CSPINBOX_minval),
  GB_PROPERTY("MaxValue", "i", CSPINBOX_maxval),
  GB_PROPERTY("Step", "i", CSPINBOX_linestep),

  GB_PROPERTY("Wrap", "b", CSPINBOX_wrapping),
  GB_PROPERTY("Border", "b", CSPINBOX_border),

  GB_METHOD("SelectAll", 0, CSPINBOX_select_all, 0),

  GB_EVENT("Change", 0, 0, &EVENT_Change),

	SPINBOX_DESCRIPTION,

  GB_END_DECLARE
};

