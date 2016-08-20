/***************************************************************************

  CSlider.cpp

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

#define __CSLIDER_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"

#include "CSlider.h"
#include "CContainer.h"
#include "CWidget.h"

DECLARE_EVENT(EVENT_Change);

void gb_raise_slider_Click(gSlider *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Change,0);
}


BEGIN_METHOD(CSLIDER_new, GB_OBJECT parent)

	InitControl(new gSlider(CONTAINER(VARG(parent))),(CWIDGET*)THIS);
	
	SLIDER->onChange=gb_raise_slider_Click;
	
END_METHOD

BEGIN_METHOD(CSCROLLBAR_new, GB_OBJECT parent)

	InitControl(new gScrollBar(CONTAINER(VARG(parent))),(CWIDGET*)THIS);
	
	SBAR->onChange=gb_raise_slider_Click;

END_METHOD


BEGIN_PROPERTY(CSLIDER_tracking)

	if (READ_PROPERTY) { GB.ReturnBoolean(SLIDER->tracking()); return; }
	SLIDER->setTracking(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CSLIDER_value)

	if (READ_PROPERTY) { GB.ReturnInteger(SLIDER->value()); return; }
	SLIDER->setValue(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSLIDER_minval)

	if (READ_PROPERTY) { GB.ReturnInteger(SLIDER->min()); return; }
	SLIDER->setMin(VPROP(GB_INTEGER));	

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_maxval)

	if (READ_PROPERTY) { GB.ReturnInteger(SLIDER->max()); return; }
	SLIDER->setMax(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_linestep)

	if (READ_PROPERTY) { GB.ReturnInteger(SLIDER->step()); return; }
	SLIDER->setStep(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_pagestep)

	if (READ_PROPERTY) { GB.ReturnInteger(SLIDER->pageStep()); return; }
	SLIDER->setPageStep(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_mark)

	if (READ_PROPERTY){ GB.ReturnBoolean(SLIDER->mark()); return; }
	SLIDER->setMark(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Slider_DefaultSize)

	GB.ReturnInteger(SLIDER->getDefaultSize());

END_PROPERTY

GB_DESC CSliderDesc[] =
{
  GB_DECLARE("Slider", sizeof(CSLIDER)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CSLIDER_new, "(Parent)Container;"),

  GB_PROPERTY("Tracking", "b", CSLIDER_tracking),
  GB_PROPERTY("Value", "i", CSLIDER_value),
  GB_PROPERTY("Mark", "b", CSLIDER_mark),
  GB_PROPERTY("MinValue", "i", CSLIDER_minval),
  GB_PROPERTY("MaxValue", "i", CSLIDER_maxval),
  GB_PROPERTY("Step", "i", CSLIDER_linestep),
  GB_PROPERTY("PageStep", "i", CSLIDER_pagestep),

  GB_EVENT("Change", 0, 0, &EVENT_Change),

  SLIDER_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CScrollBarDesc[] =
{
  GB_DECLARE("ScrollBar", sizeof(CSCROLLBAR)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CSCROLLBAR_new, "(Parent)Container;"),

  GB_PROPERTY("Tracking", "b", CSLIDER_tracking),
  GB_PROPERTY("Value", "i", CSLIDER_value),
  GB_PROPERTY("MinValue", "i", CSLIDER_minval),
  GB_PROPERTY("MaxValue", "i", CSLIDER_maxval),
  GB_PROPERTY("Step", "i", CSLIDER_linestep),
  GB_PROPERTY("PageStep", "i", CSLIDER_pagestep),
  GB_PROPERTY("DefaultSize", "i", Slider_DefaultSize),

  GB_EVENT("Change", 0, 0, &EVENT_Change),

  SCROLLBAR_DESCRIPTION,

  GB_END_DECLARE
};


