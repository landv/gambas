/***************************************************************************

  CDial.cpp

  The Dial class

  Hacked together by Nigel Gerrard using code provided by

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CDIAL_CPP

#include "main.h"
#include "gambas.h"

#include <qapplication.h>
#include <qdial.h>

#include "CDial.h"

DECLARE_EVENT(EVENT_Change);
//DECLARE_EVENT(EVENT_Activate);
//DECLARE_EVENT(EVENT_Deactivate);

BEGIN_METHOD(CDIAL_new, GB_OBJECT parent)

  QDial *wid = new QDial(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);
  //QT.SetBackgroundRole(_object, QColorGroup::Base);

  QObject::connect(wid, SIGNAL(valueChanged(int)), &CDial::manager, SLOT(event_change()));
  //QObject::connect(wid, SIGNAL(dialPressed()), &CDial::manager,
  //SLOT(event_activate()));
  //QObject::connect(wid, SIGNAL(dialMoved(int)), &CDial::manager,
  //SLOT(event_change()));
  //QObject::connect(wid, SIGNAL(dialReleased()), &CDial::manager,
  //SLOT(event_deactivate()));

  wid->setMinValue(0);
  wid->setMaxValue(100);
  wid->setLineStep(1);
  wid->setPageStep(10);
  wid->setNotchesVisible(true);
  
  wid->show();

END_METHOD


BEGIN_PROPERTY(CDIAL_tracking)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->tracking());
  else
    WIDGET->setTracking(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDIAL_notchsize)

  GB.ReturnInteger(WIDGET->notchSize());

END_PROPERTY

/*
BEGIN_PROPERTY(CDIAL_notchtarget)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->notchTarget());
  else
    WIDGET->setNotchTarget(PROPERTY(GB_INTEGER));

END_PROPERTY
*/

BEGIN_PROPERTY(CDIAL_wrapping)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->wrapping());
  else
    WIDGET->setWrapping(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDIAL_value)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->value());
  else
    WIDGET->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDIAL_notchesvisible)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->notchesVisible());
  else
    WIDGET->setNotchesVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDIAL_minval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->minValue());
  else
    WIDGET->setMinValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDIAL_maxval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->maxValue());
  else
    WIDGET->setMaxValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDIAL_linestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->lineStep());
  else
  {
  	int step = VPROP(GB_INTEGER);
  	if (step > 0)
	    WIDGET->setLineStep(step);
	}

END_PROPERTY

BEGIN_PROPERTY(CDIAL_pagestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->pageStep());
  else
  {
  	int step = VPROP(GB_INTEGER);
  	if (step > 0)
    	WIDGET->setPageStep(step);
	}
	
END_PROPERTY


/***************************************************************************

  class CDial

***************************************************************************/

CDial CDial::manager;

void CDial::event_change(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Change, 0);
}



GB_DESC CDialDesc[] =
{
  GB_DECLARE("Dial", sizeof(CDIAL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CDIAL_new, "(Parent)Container;"),

  //GB_PROPERTY("Tracking", "b", CDIAL_tracking),
  //GB_PROPERTY_READ("NotchSize", "i", CDIAL_notchsize),
  GB_PROPERTY("Value", "i", CDIAL_value),
  //GB_PROPERTY("MarkGap", "i", CDIAL_notchtarget),
  GB_PROPERTY("Mark", "b", CDIAL_notchesvisible),
  GB_PROPERTY("MinValue", "i", CDIAL_minval),
  GB_PROPERTY("MaxValue", "i", CDIAL_maxval),
  GB_PROPERTY("Step", "i", CDIAL_linestep),
  GB_PROPERTY("Wrap", "b", CDIAL_wrapping),
  GB_PROPERTY("PageStep", "i", CDIAL_pagestep),

  GB_CONSTANT("_Properties", "s", "*,MinValue=0,MaxValue=100,Step=1,PageStep=10,Wrap,Mark=True"),
  GB_CONSTANT("_DefaultEvent", "s", "Change"),
  GB_CONSTANT("_DefaultSize", "s", "6,6"),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),

  GB_END_DECLARE
};

