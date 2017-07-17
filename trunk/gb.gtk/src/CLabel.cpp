/***************************************************************************

  CLabel.cpp

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

#define __CLABEL_CPP

#include <stdio.h>

#include "gambas.h"
#include "widgets.h"
#include "CLabel.h"
#include "CContainer.h"


BEGIN_METHOD(CLABEL_new, GB_OBJECT parent)

	InitControl(new gLabel(CONTAINER(VARG(parent))),(CWIDGET*)THIS);

END_METHOD

BEGIN_METHOD(CTEXTLABEL_new, GB_OBJECT parent)

	InitControl(new gLabel(CONTAINER(VARG(parent))),(CWIDGET*)THIS);
	
	WIDGET->setWrap(true);
	WIDGET->enableMarkup(true);
	WIDGET->setAlignment(ALIGN_TOP_NORMAL);
	
END_METHOD

BEGIN_PROPERTY(CLABEL_auto_resize)

  if (READ_PROPERTY)
  	GB.ReturnBoolean(WIDGET->autoResize());
  else
  	WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLABEL_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(WIDGET->text());
	else
		WIDGET->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CLABEL_border)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->getFrameBorder());
	else
		WIDGET->setFrameBorder(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CLABEL_alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->alignment());
	else
		WIDGET->setAlignment(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLABEL_transparent)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isTransparent());
	else
		WIDGET->setTransparent(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CLABEL_padding)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->padding());
  else
    WIDGET->setPadding(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD_VOID(CLABEL_adjust)

	WIDGET->adjust();

END_METHOD

BEGIN_PROPERTY(Label_Wrap)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->wrap());
	else
		WIDGET->setWrap(VPROP(GB_BOOLEAN));

END_PROPERTY



GB_DESC CLabelDesc[] =
{
  GB_DECLARE("Label", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CLABEL_text),
  GB_PROPERTY("Caption", "s", CLABEL_text),
  GB_PROPERTY("Alignment", "i", CLABEL_alignment),
  GB_PROPERTY("Border", "i", CLABEL_border),
  GB_PROPERTY("AutoResize", "b", CLABEL_auto_resize),
  GB_PROPERTY("Transparent","b",CLABEL_transparent),
  GB_PROPERTY("Padding", "i", CLABEL_padding),
  GB_METHOD("Adjust", 0, CLABEL_adjust, 0),
  
  LABEL_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CTextLabelDesc[] =
{
  GB_DECLARE("TextLabel", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CTEXTLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CLABEL_text),
  GB_PROPERTY("Caption", "s", CLABEL_text),
  GB_PROPERTY("Alignment", "i", CLABEL_alignment),
  GB_PROPERTY("Border", "i", CLABEL_border), 
  GB_PROPERTY("AutoResize", "b", CLABEL_auto_resize),
  GB_PROPERTY("Transparent","b",CLABEL_transparent),
  GB_PROPERTY("Wrap","b",Label_Wrap),
  GB_PROPERTY("Padding", "i", CLABEL_padding),
  GB_METHOD("Adjust", 0, CLABEL_adjust, 0),

  TEXTLABEL_DESCRIPTION,

  GB_END_DECLARE
};



