/***************************************************************************

  CButton.h

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

#ifndef __CBUTTON_H
#define __CBUTTON_H

#include "main.h"
#include "gbutton.h"
#include "CWidget.h"
#include "CPicture.h"


#ifndef __CBUTTON_CPP
extern GB_DESC CButtonDesc[];
extern GB_DESC CToggleButtonDesc[];
extern GB_DESC CCheckBoxDesc[];
extern GB_DESC CRadioButtonDesc[];
extern GB_DESC CToolButtonDesc[];
#else

#define THIS ((CBUTTON *)_object)
#define BUTTON ((gButton*)THIS->ob.widget)

#endif 

typedef  
	struct
	{
		CWIDGET ob;
		int f_value;
	}  
	CBUTTON;

#endif
