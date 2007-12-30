/***************************************************************************

  CPicture.h

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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

#ifndef __CPICTURE_H
#define __CPICTURE_H

#include "gambas.h"
#include "widgets.h"


typedef  struct 
{
	GB_BASE  ob;
	gPicture *picture;
	void     *fdata;

} CPICTURE;

#ifndef __CPICTURE_CPP

extern GB_DESC CPictureDesc[];

#else

#define MAX_KEY 255

#define STOCK_PREFIX "icon:/"
#define STOCK_PREFIX_LEN 6

#define THIS OBJECT(CPICTURE)

#endif

#define PICTURE (((CPICTURE*)_object)->picture)


void* GTK_GetPicture(GdkPixbuf *buf);


#endif
