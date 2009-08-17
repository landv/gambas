/***************************************************************************

  CListBox.h

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CLISTBOX_H
#define __CLISTBOX_H

#include "main.h"
#include "glistbox.h"

#ifndef __CLISTBOX_CPP
extern GB_DESC CListBoxDesc[];
extern GB_DESC CListBoxItemDesc[];
#else

#define THIS   ((CLISTBOX *)_object)
#define LISTBOX ((gListBox *)THIS->ob.widget)

#endif


typedef  
	struct 
	{
		CWIDGET ob;
		int index;
	} 
	CLISTBOX;

#endif
