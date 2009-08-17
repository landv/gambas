/***************************************************************************

  CGridView.cpp

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


#define __CGRIDVIEW_CPP

#include "main.h"
#include "CGridView.h"

BEGIN_PROPERTY(CGRIDVIEW_footer)

	GTK.GridView.FooterProperty(_object, _param);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_footer_text)

	GTK.GridView.ColumnFooterTextProperty(_object, _param);

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDVIEW_columns_get)

	GTK.GridView.ColumnsGetMethod(_object, _param);

END_METHOD

/***************************************************************************

  Gambas Interfaces

***************************************************************************/

GB_DESC CGridViewColumnDesc[] =
{
  GB_DECLARE(".GridViewColumn", 0), GB_VIRTUAL_CLASS(),
  
  //GB_PROPERTY("HeaderText","s",CGRIDVIEW_column_header_text),
  GB_PROPERTY("FooterText","s",CGRIDVIEW_column_footer_text),
  
  GB_END_DECLARE
};

GB_DESC CGridViewColumnsDesc[] =
{
  GB_DECLARE(".GridViewColumns", 0), GB_VIRTUAL_CLASS(),
  
  //GB_PROPERTY("HeaderText","s",CGRIDVIEW_column_header_text),
  GB_METHOD("_get", ".GridViewColumn", CGRIDVIEW_columns_get, "(Column)i"),
  
  GB_END_DECLARE
};

GB_DESC CGridViewDesc[] =
{
  GB_DECLARE("GridView", sizeof(CGRIDVIEW)),

	GB_PROPERTY_SELF("Columns", ".GridViewColumns"),
  GB_PROPERTY("Footer", "b", CGRIDVIEW_footer),
  
  GB_CONSTANT("_Properties", "s", "*,Footer"),
    
  GB_END_DECLARE
};
