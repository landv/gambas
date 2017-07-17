/***************************************************************************

  CTabStrip.h

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

#ifndef __CTABSTRIP_H
#define __CTABSTRIP_H

#include "main.h"
#include "gtabstrip.h"
#include "CWidget.h"
#include "CContainer.h"

#ifndef __CTABSTRIP_CPP
extern GB_DESC CTabStripDesc[];
extern GB_DESC CTabStripContainerDesc[];
extern GB_DESC CTabStripContainerChildrenDesc[];
#else


#define THIS ((CTABSTRIP*)_object)
#define TABSTRIP ((gTabStrip*)THIS->ob.widget)

#endif

typedef  
	struct
	{
		CWIDGET ob;
		int index;
		void *textFont;
	}  
	CTABSTRIP;

#endif
