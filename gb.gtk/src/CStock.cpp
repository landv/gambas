/***************************************************************************

  CStock.cpp

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

#define __CSTOCK_CPP

#include "main.h"
#include "widgets.h"
#include "gstock.h"
#include "CPicture.h"
#include "CStock.h"

/*******************************************************************************

  class Stock

*******************************************************************************/

BEGIN_METHOD(CSTOCK_get, GB_STRING path)

	CPICTURE *Pic;
	gPicture *pic;
	
	pic=gStock::get( GB.ToZeroString(ARG(path)) );
	
	if (!pic)
	{
		GB.ReturnNull();
		return;
	}
	
	GB.New((void **)&Pic, GB.FindClass("Picture"), 0, 0);
	if (Pic->picture) Pic->picture->unref();
	Pic->picture=pic;
	GB.ReturnObject(Pic);

END_METHOD




GB_DESC CStockDesc[] =
{
  GB_DECLARE("Stock", 0), GB_NOT_CREATABLE(),

 
  GB_STATIC_METHOD("_get", "Picture", CSTOCK_get, "(Key)s"),
  

  GB_END_DECLARE
};
