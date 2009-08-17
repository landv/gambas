/***************************************************************************

  CTreeView.h

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

#ifndef __CTREEVIEW_H
#define __CTREEVIEW_H

#include "main.h"
#include "gtreeview.h"

#ifndef __CTREEVIEW_CPP

extern GB_DESC CTreeViewItemDesc[];
extern GB_DESC CListViewItemDesc[];
extern GB_DESC CColumnViewItemDesc[];

extern GB_DESC CTreeViewDesc[];
extern GB_DESC CListViewDesc[];

extern GB_DESC CColumnViewColumnDesc[];
extern GB_DESC CColumnViewColumnsDesc[];

extern GB_DESC CColumnViewDesc[];

#else

#define THIS ((CTREEVIEW *)_object)
#define WIDGET ((gTreeView*)THIS->ob.widget)

#endif

typedef 
  struct 
  {
    CWIDGET ob;
    char *item;
    char *save;
    int compare;
    int column;
  }  
  CTREEVIEW;

#endif
