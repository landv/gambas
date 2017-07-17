/***************************************************************************

  CSeparator.cpp

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

#define __CSEPARATOR_CPP

#include "CSeparator.h"
#include "CContainer.h"


BEGIN_METHOD(CSEPARATOR_new, GB_OBJECT parent)

	InitControl(new gSeparator(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	
END_METHOD



GB_DESC CSeparatorDesc[] =
{
  GB_DECLARE("Separator", sizeof(CSEPARATOR)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CSEPARATOR_new, "(Parent)Container;"),

  SEPARATOR_DESCRIPTION,

  GB_END_DECLARE
};




