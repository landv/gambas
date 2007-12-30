/***************************************************************************

  CProgress.cpp

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

#define __CPROGRESS_CPP



#include <qprogressbar.h>

#include "gambas.h"

#include "CProgress.h"


BEGIN_METHOD(CPROGRESS_new, GB_OBJECT parent)

  QProgressBar *wid = new QProgressBar(100, QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object, "ProgressBar");

  wid->setTotalSteps(10000);
  wid->setCenterIndicator(true);
  //wid->setFrameStyle(QFrame::Panel + QFrame::Sunken);
  //wid->setLineWidth(2);
  wid->show();

END_METHOD

#if 0
BEGIN_PROPERTY(CPROGRESS_max)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->totalSteps());
  else
  {
    long max = PROPERTY(long);

    if (max < 1 || max < WIDGET->progress())
    {
      GB.Error("Bad argument");
      return;
    }

    WIDGET->setTotalSteps(max);
  }

END_PROPERTY
#endif

BEGIN_PROPERTY(CPROGRESS_value)

  if (READ_PROPERTY)
  {
  	int pr = WIDGET->progress();
    GB.ReturnFloat(pr < 0 ? 0 : (double)pr / WIDGET->totalSteps());
  }
  else
  {
    double val = VPROP(GB_FLOAT);

    if (val < 0)
      WIDGET->reset();
    else
    {
      if (val > 1)
        val = 1;
      WIDGET->setProgress((int)(WIDGET->totalSteps() * val + 0.5));
    }
  }

END_PROPERTY


BEGIN_PROPERTY(CPROGRESS_label)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->percentageVisible());
  else
    WIDGET->setPercentageVisible(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CPROGRESS_reset)

  WIDGET->reset();

END_METHOD


GB_DESC CProgressDesc[] =
{
  GB_DECLARE("ProgressBar", sizeof(CPROGRESS)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CPROGRESS_new, "(Parent)Container;"),

  GB_PROPERTY("Value", "f", CPROGRESS_value),
  GB_PROPERTY("Label", "b", CPROGRESS_label),
  //GB_PROPERTY("Max", "i", CPROGRESS_max),
  //GB_PROPERTY("Border", "i<Border>", CWIDGET_border),
  GB_METHOD("Reset", NULL, CPROGRESS_reset, NULL),

	PROGRESSBAR_DESCRIPTION,

  GB_END_DECLARE
};


