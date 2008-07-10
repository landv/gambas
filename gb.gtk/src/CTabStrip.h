/***************************************************************************

  CTabStrip.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

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

#ifndef __CTABSTRIP_H
#define __CTABSTRIP_H

#include "main.h"
#include "gtabstrip.h"
#include "CWidget.h"
#include "CContainer.h"

#ifndef __CTABSTRIP_CPP
extern GB_DESC CTabStripDesc[];
extern GB_DESC CTabDesc[];
extern GB_DESC CTabChildrenDesc[];
#else


#define THIS ((CTABSTRIP*)_object)
#define TABSTRIP ((gTabStrip*)THIS->ob.widget)

#endif

typedef  
	struct
	{
		CWIDGET ob;
		int index;
	}  
	CTABSTRIP;

#endif
