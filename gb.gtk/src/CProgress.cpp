/***************************************************************************

  CProgress.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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

#define __CPROGRESS_CPP

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CProgress.h"
#include "CWidget.h"
#include "CContainer.h"


BEGIN_METHOD(CPROGRESS_new, GB_OBJECT parent)

	InitControl(new gProgressBar(CONTAINER(VARG(parent))), (CWIDGET*)THIS);

END_METHOD



BEGIN_PROPERTY(CPROGRESS_value)

	if (READ_PROPERTY) { GB.ReturnFloat(PB->value()); return; }
	PB->setValue(VPROP(GB_FLOAT));


END_PROPERTY


BEGIN_PROPERTY(CPROGRESS_label)

	if (READ_PROPERTY) { GB.ReturnBoolean(PB->label()); return; }
	PB->setLabel(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CPROGRESS_reset)

	PB->reset();

END_METHOD


GB_DESC CProgressDesc[] =
{
  GB_DECLARE("ProgressBar", sizeof(CPROGRESS)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CPROGRESS_new, "(Parent)Container;"),

  GB_PROPERTY("Value", "f", CPROGRESS_value),
  GB_PROPERTY("Label", "b", CPROGRESS_label),
  GB_METHOD("Reset", NULL, CPROGRESS_reset, NULL),

  PROGRESSBAR_DESCRIPTION,

  GB_END_DECLARE
};


