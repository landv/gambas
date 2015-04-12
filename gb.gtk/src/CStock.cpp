/***************************************************************************

  CStock.cpp

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

#define __CSTOCK_CPP

#include "main.h"
#include "widgets.h"
#include "CPicture.h"
#include "CStock.h"

/*******************************************************************************

  class Stock

*******************************************************************************/

BEGIN_METHOD_VOID(CSTOCK_get)

	GB.Deprecated("gb.gtk", "Stock class", NULL);
	GB.ReturnNull();

END_METHOD


GB_DESC CStockDesc[] =
{
  GB_DECLARE("Stock", 0), GB_NOT_CREATABLE(),
 
  GB_STATIC_METHOD("_get", "Picture", CSTOCK_get, "(Key)s[(Default)s]"),

  GB_END_DECLARE
};
