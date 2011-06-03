/***************************************************************************

  CGridView.h

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

#ifndef __CGRIDVIEW_H
#define __CGRIDVIEW_H

#include "main.h"
#include "ggridview.h"
#include "CPicture.h"

#ifndef __CGRIDVIEW_CPP
extern GB_DESC CGridViewItemDesc[];
extern GB_DESC CGridViewDataDesc[];
extern GB_DESC CGridViewColumnDesc[];
extern GB_DESC CGridViewRowDesc[];
extern GB_DESC CGridViewColumnsDesc[];
extern GB_DESC CGridViewRowsDesc[];
extern GB_DESC CGridViewDesc[];
#else

#define THIS ((CGRIDVIEW *)_object)
#define WIDGET ((gGridView*)THIS->ob.widget)

#endif

typedef 
	struct 
	{
		CWIDGET ob;
		int row;
		int col;
		gTableData *data;
	}
  CGRIDVIEW;

DECLARE_PROPERTY(CGRIDVIEW_footer);
DECLARE_PROPERTY(CGRIDVIEW_column_footer_text);
DECLARE_METHOD(CGRIDVIEW_columns_get);

#endif
