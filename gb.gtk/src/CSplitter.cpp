/***************************************************************************

  CSplitter.cpp

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

#define __CSPLITTER_CPP

#include "CSplitter.h"


DECLARE_EVENT(EVENT_Resize);

static void send_event(CSPLITTER *_object)
{
	if (!WIDGET)
		return;
	GB.Raise(THIS, EVENT_Resize, 0);
	THIS->event = FALSE;
}

static void gb_raise_splitter_Resize(gControl *sender)
{
	CWIDGET *_object = GetObject(sender);
	
	if (THIS->event)
		return;
	
	THIS->event = true;
	GB.Post((void (*)())send_event, (intptr_t)THIS);
}

BEGIN_METHOD(CHSPLIT_new, GB_OBJECT parent)

	InitControl(new gSplitter(CONTAINER(VARG(parent)), false),(CWIDGET*)THIS);
	WIDGET->onResize = gb_raise_splitter_Resize;

END_METHOD


BEGIN_METHOD(CVSPLIT_new, GB_OBJECT parent)

	InitControl(new gSplitter(CONTAINER(VARG(parent)), true),(CWIDGET*)THIS);
	WIDGET->onResize = gb_raise_splitter_Resize;

END_METHOD


BEGIN_PROPERTY(CSPLITTER_layout)

	GB_ARRAY array;
	
	if (READ_PROPERTY)
	{
		GB.Array.New(&array, GB_T_INTEGER, WIDGET->layoutCount());
		WIDGET->getLayout((int *)GB.Array.Get(array, 0));
		GB.ReturnObject(array);
	}
	else
	{
		GB_ARRAY array = (GB_ARRAY)VPROP(GB_OBJECT);
		
		if (!array || GB.CheckObject(array) || GB.Array.Count(array) == 0)
			return;
		
		WIDGET->setLayout((int *)GB.Array.Get(array, 0), GB.Array.Count(array));
	}


END_PROPERTY


GB_DESC CHSplitDesc[] =
{
  GB_DECLARE("HSplit", sizeof(CSPLITTER)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHSPLIT_new, "(Parent)Container;"),

  GB_PROPERTY("Layout", "Integer[]", CSPLITTER_layout),
  GB_PROPERTY("Settings", "Integer[]", CSPLITTER_layout),

  GB_EVENT("Resize", 0, 0, &EVENT_Resize),

  HSPLIT_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CVSplitDesc[] =
{
  GB_DECLARE("VSplit", sizeof(CSPLITTER)), GB_INHERITS("Container"),

  GB_METHOD("_new", 0, CVSPLIT_new, "(Parent)Container;"),

  GB_PROPERTY("Layout", "Integer[]", CSPLITTER_layout),
  GB_PROPERTY("Settings", "Integer[]", CSPLITTER_layout),

  GB_EVENT("Resize", 0, 0, &EVENT_Resize),

  VSPLIT_DESCRIPTION,

  GB_END_DECLARE
};





