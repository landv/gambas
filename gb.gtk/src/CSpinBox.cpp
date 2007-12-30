/***************************************************************************

  CSpinBox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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

void spinBox_raise_Change(gSpinBox *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Change,0);
}

BEGIN_METHOD(CSPINBOX_new, GB_OBJECT parent)
	
	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gSpinBox(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	SPINBOX->onChange=spinBox_raise_Change;


END_METHOD


BEGIN_PROPERTY(CSPINBOX_wrapping)

	if (READ_PROPERTY) { GB.ReturnBoolean(SPINBOX->wrap()); return; }
	SPINBOX->setWrap(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_value)

	if (READ_PROPERTY) { GB.ReturnInteger(SPINBOX->value()); return; }
	SPINBOX->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_text)



END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_cleantext)



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

BEGIN_PROPERTY(CSPINBOX_prefix)

	stub("CSPINBOX_prefix");
	if (READ_PROPERTY) GB.ReturnNewString("",0);

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_suffix)

	stub("CSPINBOX_suffix");
	if (READ_PROPERTY) GB.ReturnNewString("",0);

END_PROPERTY



GB_DESC CSpinBoxDesc[] =
{
  GB_DECLARE("SpinBox", sizeof(CSPINBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CSPINBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Prefix", "s", CSPINBOX_prefix),
  GB_PROPERTY("Suffix", "s", CSPINBOX_suffix),
  GB_PROPERTY("Value", "i", CSPINBOX_value),

  GB_PROPERTY_READ("FullText", "s", CSPINBOX_text),
  GB_PROPERTY_READ("Text", "s", CSPINBOX_cleantext),
  GB_PROPERTY("MinValue", "i", CSPINBOX_minval),
  GB_PROPERTY("MaxValue", "i", CSPINBOX_maxval),
  GB_PROPERTY("Step", "i", CSPINBOX_linestep),

  GB_PROPERTY("Wrap", "b", CSPINBOX_wrapping),

  GB_CONSTANT("_Properties", "s", CSPINBOX_PROPERTIES),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),

  GB_END_DECLARE
};

