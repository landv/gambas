/***************************************************************************

  CScrollBar.cpp

  The ScrollBar class

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

#define __CSCROLLBAR_CPP

#include "main.h"
#include "gambas.h"

#include <qapplication.h>
#include <qscrollbar.h>

#include "CWidget.h"
#include "CScrollBar.h"

DECLARE_EVENT(EVENT_Change);

/***************************************************************************

  class MyScrollBar

***************************************************************************/


MyScrollBar::MyScrollBar(QWidget *parent)
: QScrollBar(parent)
{
}


void MyScrollBar::resizeEvent(QResizeEvent *e)
{
  //CSCROLLVIEW *ob = (CSCROLLVIEW *)CWidget::get(this);

  QScrollBar::resizeEvent(e);

  if (width() >= height())
    setOrientation(Qt::Horizontal);
  else
    setOrientation(Qt::Vertical);
}


/***************************************************************************

  ScrollBar

***************************************************************************/

BEGIN_METHOD(CSCROLLBAR_new, GB_OBJECT parent)

  MyScrollBar *wid = new MyScrollBar(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, _object);
  //QT.SetBackgroundRole(_object, QColorGroup::Base);

  QObject::connect(wid, SIGNAL(valueChanged(int)), &CScrollBar::manager,
  SLOT(event_change()));
//  QObject::connect(wid, SIGNAL(sliderPressed()), &CScrollBar::manager,
//  SLOT(event_sliderpressed()));
//  QObject::connect(wid, SIGNAL(sliderMoved(int)), &CScrollBar::manager,
//  SLOT(event_slidermove()));
//  QObject::connect(wid, SIGNAL(sliderReleased()), &CScrollBar::manager,
//  SLOT(event_sliderreleased()));

  wid->setTracking(true);
  wid->setMinValue(0);
  wid->setMaxValue(100);
  wid->setLineStep(1);
  wid-> setPageStep(10);

  wid->show();

END_METHOD


BEGIN_PROPERTY(CSCROLLBAR_tracking)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->tracking());
  else
    WIDGET->setTracking(VPROP(GB_BOOLEAN));

END_PROPERTY

/*
BEGIN_PROPERTY(CSCROLLBAR_orientation)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->orientation());
  else
  {
    switch PROPERTY(GB_INTEGER)
    {
      case Qt::Vertical: WIDGET->setOrientation(Qt::Vertical);break;
      case Qt::Horizontal: WIDGET->setOrientation(Qt::Horizontal);break;
      default: WIDGET->setOrientation(Qt::Vertical);
    }
  }

END_PROPERTY
*/

BEGIN_PROPERTY(CSCROLLBAR_value)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->value());
  else
    WIDGET->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSCROLLBAR_minval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->minValue());
  else
    WIDGET->setMinValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSCROLLBAR_maxval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->maxValue());
  else
    WIDGET->setMaxValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSCROLLBAR_linestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->lineStep());
  else
    WIDGET->setLineStep(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSCROLLBAR_pagestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->pageStep());
  else
    WIDGET->setPageStep(VPROP(GB_INTEGER));

END_PROPERTY


/***************************************************************************

  class CScrollBar

***************************************************************************/

CScrollBar CScrollBar::manager;

void CScrollBar::event_change(void)
{
  GET_SENDER(object);
  GB.Raise(object, EVENT_Change, 0);
}

/*void CScrollBar::event_sliderpressed(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_SliderPressed, NULL);
} */

/*void CScrollBar::event_slidermove(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_SliderMove, NULL);
}*/
/*
void CScrollBar::event_sliderreleased(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_SliderReleased, NULL);
}
*/

GB_DESC CScrollBarDesc[] =
{
  GB_DECLARE("ScrollBar", sizeof(CSCROLLBAR)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CSCROLLBAR_new, "(Parent)Container;"),

  //GB_CONSTANT("Vertical","i", Qt::Vertical),
  //GB_CONSTANT("Horizontal","i", Qt::Horizontal),
  GB_PROPERTY("Tracking", "b", CSCROLLBAR_tracking),
  GB_PROPERTY("Value", "i", CSCROLLBAR_value),
  GB_PROPERTY("MinValue", "i", CSCROLLBAR_minval),
  GB_PROPERTY("MaxValue", "i", CSCROLLBAR_maxval),
  GB_PROPERTY("Step", "i", CSCROLLBAR_linestep),
  GB_PROPERTY("PageStep", "i", CSCROLLBAR_pagestep),
  //GB_PROPERTY("Orientation", "i<ScrollBar,Vertical,Horizontal>", CSCROLLBAR_orientation),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),

	SCROLLBAR_DESCRIPTION,
	
  GB_END_DECLARE
};

