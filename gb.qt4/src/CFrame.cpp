/***************************************************************************

  CFrame.cpp

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CFRAME_CPP

#include <qapplication.h>
#include <QGroupBox>

#include "gambas.h"

#include "CFrame.h"


BEGIN_METHOD(CFRAME_new, GB_OBJECT parent)

  QGroupBox *wid = new QGroupBox(QCONTAINER(VARG(parent)));

  THIS->container = wid;

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(CFRAME_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->title()));
  else
    WIDGET->setTitle(QSTRING_PROP());

END_PROPERTY


GB_DESC CFrameDesc[] =
{
  GB_DECLARE("Frame", sizeof(CFRAME)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CFRAME_new, "(Parent)Container;"),

  GB_PROPERTY("Caption", "s", CFRAME_text),
  GB_PROPERTY("Text", "s", CFRAME_text),
  GB_PROPERTY("Title", "s", CFRAME_text),

	FRAME_DESCRIPTION,

  GB_END_DECLARE
};


