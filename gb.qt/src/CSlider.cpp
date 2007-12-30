/***************************************************************************

  CSlider.cpp

  The Slider class

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

#define __CSLIDER_CPP

#include "main.h"
#include "gambas.h"

#include <qapplication.h>
#include <qslider.h>

#include "CWidget.h"
#include "CSlider.h"

DECLARE_EVENT(EVENT_Change);


/***************************************************************************

  class MySlider

***************************************************************************/


MySlider::MySlider(QWidget *parent)
: QSlider(parent)
{
}


void MySlider::resizeEvent(QResizeEvent *e)
{
  //CSCROLLVIEW *ob = (CSCROLLVIEW *)CWidget::get(this);

  QSlider::resizeEvent(e);

  if (width() >= height())
    setOrientation(Qt::Horizontal);
  else
    setOrientation(Qt::Vertical);
}


/***************************************************************************

  Slider

***************************************************************************/

BEGIN_METHOD(CSLIDER_new, GB_OBJECT parent)

  MySlider *wid = new MySlider(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, _object);
  //QT.SetBackgroundRole(_object, QColorGroup::Base);

  QObject::connect(wid, SIGNAL(valueChanged(int)), &CSlider::manager, SLOT(event_change()));
  //QObject::connect(wid, SIGNAL(sliderPressed()), &CSlider::manager,
  //SLOT(event_sliderpressed()));
  //QObject::connect(wid, SIGNAL(sliderMoved(int)), &CSlider::manager,
  //SLOT(event_slidermove()));
  //QObject::connect(wid, SIGNAL(sliderReleased()), &CSlider::manager,
  //SLOT(event_sliderreleased()));

  wid->setTracking(true); //Set the tracking off by default
  wid->setMinValue(0);
  wid->setMaxValue(100);
  wid->setLineStep(1);
  wid->setPageStep(10);

  wid->show();

END_METHOD


BEGIN_PROPERTY(CSLIDER_tracking)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->tracking());
  else
    WIDGET->setTracking(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_tickinterval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->tickInterval());
  else
    WIDGET->setTickInterval(VPROP(GB_INTEGER));

END_PROPERTY

/*
BEGIN_PROPERTY(CSLIDER_orientation)

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

BEGIN_PROPERTY(CSLIDER_value)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->value());
  else
    WIDGET->setValue(VPROP(GB_INTEGER));

END_PROPERTY

/*
BEGIN_PROPERTY(CSLIDER_tickmarks)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->tickmarks());
  else
  {
    switch PROPERTY(GB_INTEGER)
    {
      case QSlider::NoMarks: WIDGET->setTickmarks(QSlider::NoMarks);break;
      case QSlider::Both: WIDGET->setTickmarks(QSlider::Both);break;
      case QSlider::Above: WIDGET->setTickmarks(QSlider::Above);break;
      case QSlider::Below: WIDGET->setTickmarks(QSlider::Below);break;
      //case QSlider::Left: WIDGET->setTickmarks(QSlider::Left);break;
      //case QSlider::Right: WIDGET->setTickmarks(QSlider::Right);break;
      default: WIDGET->setTickmarks(QSlider::NoMarks);
    }
  }

END_PROPERTY
*/


BEGIN_PROPERTY(CSLIDER_mark)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->tickmarks() != QSlider::NoMarks);
  else
  {
    if (VPROP(GB_BOOLEAN))
      WIDGET->setTickmarks(QSlider::Both);
    else
      WIDGET->setTickmarks(QSlider::NoMarks);
  }

END_PROPERTY


BEGIN_PROPERTY(CSLIDER_minval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->minValue());
  else
    WIDGET->setMinValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_maxval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->maxValue());
  else
    WIDGET->setMaxValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_linestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->lineStep());
  else
    WIDGET->setLineStep(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSLIDER_pagestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->pageStep());
  else
  {
    WIDGET->setPageStep(VPROP(GB_INTEGER));
    WIDGET->update();
  }

END_PROPERTY


/***************************************************************************

  class CSlider

***************************************************************************/

CSlider CSlider::manager;

void CSlider::event_change(void)
{
  GET_SENDER(object);
  GB.Raise(object, EVENT_Change, 0);
}

/*void CSlider::event_sliderpressed(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_SliderPressed, NULL);
}*/

/*void CSlider::event_slidermove(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_SliderMove, NULL);
}*/

/*
void CSlider::event_sliderreleased(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_SliderReleased, NULL);
}
*/

GB_DESC CSliderDesc[] =
{
  GB_DECLARE("Slider", sizeof(CSLIDER)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CSLIDER_new, "(Parent)Container;"),

  //GB_CONSTANT("NoMarks","i", QSlider::NoMarks),
  //GB_CONSTANT("Both","i", QSlider::Both),
  //GB_CONSTANT("AboveOrLeft","i", QSlider::Above), //when orientation is horizontal Above is valid, when Vertical Left
  //GB_CONSTANT("BelowOrRight","i", QSlider::Below),
  //GB_CONSTANT("Vertical","i", Qt::Vertical),
  //GB_CONSTANT("Horizontal","i", Qt::Horizontal),
  GB_PROPERTY("Tracking", "b", CSLIDER_tracking),
  GB_PROPERTY("Value", "i", CSLIDER_value),
  //GB_PROPERTY("MarkGap", "i", CSLIDER_tickinterval),
  GB_PROPERTY("Mark", "b", CSLIDER_mark),
  GB_PROPERTY("MinValue", "i", CSLIDER_minval),
  GB_PROPERTY("MaxValue", "i", CSLIDER_maxval),
  GB_PROPERTY("Step", "i", CSLIDER_linestep),
  GB_PROPERTY("PageStep", "i", CSLIDER_pagestep),
  //GB_PROPERTY("Orientation", "i<Slider,Vertical,Horizontal>", CSLIDER_orientation),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),

	SLIDER_DESCRIPTION,

  GB_END_DECLARE
};

