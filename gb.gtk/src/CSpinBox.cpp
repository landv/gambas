/***************************************************************************

  CSpinBox.cpp

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

#define __CSPINBOX_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"

#include "CSpinBox.h"
#include "CWidget.h"
#include "CContainer.h"

DECLARE_EVENT(EVENT_Change);

static void raise_change(CWIDGET *_object)
{
	GB.Raise(THIS, EVENT_Change, 0);
	GB.Unref(POINTER(&_object));
}

static void cb_change(gSpinBox *sender)
{
	CWIDGET *_object = GetObject(sender);
	GB.Ref(THIS);
	GB.Post((GB_POST_FUNC)raise_change, (long)THIS);
}

BEGIN_METHOD(CSPINBOX_new, GB_OBJECT parent)
	
	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSpinBox(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
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


GB_DESC CSpinBoxDesc[] =
{
  GB_DECLARE("SpinBox", sizeof(CSPINBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CSPINBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Value", "i", CSPINBOX_value),
  GB_PROPERTY_READ("Text", "s", CSPINBOX_text),

  GB_PROPERTY("MinValue", "i", CSPINBOX_minval),
  GB_PROPERTY("MaxValue", "i", CSPINBOX_maxval),
  GB_PROPERTY("Step", "i", CSPINBOX_linestep),

  GB_PROPERTY("Wrap", "b", CSPINBOX_wrapping),

  GB_METHOD("SelectAll", NULL, CSPINBOX_select_all, NULL),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),

	SPINBOX_DESCRIPTION,

  GB_END_DECLARE
};

