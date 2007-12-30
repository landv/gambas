/***************************************************************************

  CLCDNumber.cpp

  (c) 2002-2003 Nigel Gerrard <ngerrard@ngerrard.uklinux.net>

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

#define __CLCDNUMBER_CPP

#include "main.h"

#include <qapplication.h>
#include <qlcdnumber.h>

#include "gambas.h"

#include "CLCDNumber.h"

/* #define DEBUG_CLCDNUMBER */


BEGIN_METHOD(CLCDNUMBER_new, GB_OBJECT parent)

  QLCDNumber *wid = new QLCDNumber(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);
  //QT.SetBackgroundRole(_object, QColorGroup::Base);

  wid->setFrameStyle(QFrame::NoFrame);

  wid->show();

END_METHOD

BEGIN_PROPERTY(CLCDNUMBER_value)

  if (READ_PROPERTY)
    GB.ReturnFloat(WIDGET->value());
  else
    WIDGET->display(VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(CLCDNUMBER_digits)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->numDigits());
  else
  {
    int n = VPROP(GB_INTEGER);

    if (n < 1)
      n = 1;
    else if (n > 32)
      n = 32;

    WIDGET->setNumDigits(n);
    /* Increasing the number of digits does not redisplay value */
    WIDGET->repaint(WIDGET->visibleRect());
    WIDGET->display(WIDGET->value());
  }

END_PROPERTY

BEGIN_PROPERTY(CLCDNUMBER_decimalpoint)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->smallDecimalPoint());
  else
  {
    WIDGET->setSmallDecimalPoint(VPROP(GB_BOOLEAN));
    WIDGET->repaint(WIDGET->visibleRect());
    WIDGET->display(WIDGET->value()); /* Solves issue where decimal disappears */
  }

END_PROPERTY

BEGIN_PROPERTY(CLCDNUMBER_overflow)

  GB.ReturnBoolean(WIDGET->checkOverflow(WIDGET->value()));

END_PROPERTY


BEGIN_PROPERTY(CLCDNUMBER_mode)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->mode());
  else
  {
    switch (VPROP(GB_INTEGER))
    {
      case QLCDNumber::HEX: WIDGET->setHexMode(); break;
      case QLCDNumber::DEC: WIDGET->setDecMode(); break;
      //case QLCDNumber::OCT: WIDGET->setOctMode(); break;
      case QLCDNumber::BIN: WIDGET->setBinMode(); break;
    }
    WIDGET->repaint(WIDGET->visibleRect());
    WIDGET->display(WIDGET->value());
  }

END_PROPERTY


BEGIN_PROPERTY(CLCDNUMBER_segmentStyle)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->segmentStyle());
  else
  {
    switch VPROP(GB_INTEGER)
    {
      case QLCDNumber::Outline: WIDGET->setSegmentStyle(QLCDNumber::Outline); break;
      case QLCDNumber::Filled: WIDGET->setSegmentStyle(QLCDNumber::Filled); break;
      case QLCDNumber::Flat: WIDGET->setSegmentStyle(QLCDNumber::Flat); break;
    }
  }

END_PROPERTY

BEGIN_PROPERTY(CLCDNUMBER_border)

  QT.FullBorderProperty(_object, _param);

END_PROPERTY


GB_DESC CLCDNumberDesc[] =
{
  GB_DECLARE("LCDNumber", sizeof(CLCDNUMBER)), GB_INHERITS("Control"),

  GB_CONSTANT("Hexadecimal","i", QLCDNumber::HEX),
  GB_CONSTANT("Decimal","i", QLCDNumber::DEC),
  //GB_CONSTANT("Octal","i", QLCDNumber::OCT),
  GB_CONSTANT("Binary","i", QLCDNumber::BIN),

  GB_CONSTANT("Outline","i", QLCDNumber::Outline),
  GB_CONSTANT("Filled","i", QLCDNumber::Filled),
  GB_CONSTANT("Flat","i", QLCDNumber::Flat),

  GB_METHOD("_new", NULL, CLCDNUMBER_new, "(Parent)Container;"),

  GB_PROPERTY("Value", "f", CLCDNUMBER_value),
  GB_PROPERTY("Mode", "i", CLCDNUMBER_mode),
  GB_PROPERTY("SmallDecimalPoint", "b", CLCDNUMBER_decimalpoint),
  GB_PROPERTY_READ("Overflow", "b", CLCDNUMBER_overflow),
  GB_PROPERTY("Digits", "i", CLCDNUMBER_digits),
  GB_PROPERTY("Border", "i", CLCDNUMBER_border),
  GB_PROPERTY("Style", "i", CLCDNUMBER_segmentStyle),

  GB_CONSTANT("_Properties", "s", "*,Value,Digits{Range:1;64}=1,SmallDecimalPoint,Style{LCDNumber.Outline;Filled;Flat}=Outline,Mode{LCDNumber.Decimal;Hexadecimal;Binary}=Decimal,Border{Border.*}"),
  GB_CONSTANT("_DefaultSize", "s", "12,6"),

  GB_END_DECLARE
};
