/***************************************************************************

  CWatcher.h

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CWATCHER_H
#define __CWATCHER_H

#include "gambas.h"
#include "CWidget.h"

typedef	
	struct 
	{
		GB_BASE ob;
		CWIDGET *wid;
		int x, y, w, h;
	}
	CWATCHER;

#ifndef __CWATCHER_CPP
extern GB_DESC CWatcherDesc[];
#else
#define THIS ((CWATCHER*)_object)
#endif
#endif
