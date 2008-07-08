/***************************************************************************

  CWindow.h

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

#ifndef __CWINDOW_H
#define __CWINDOW_H

#include "main.h"
#include "gmainwindow.h"
#include "CWidget.h"
#include "CPicture.h"

typedef  
	struct
	{
		CWIDGET ob;
		int ret;
		unsigned embed : 1;
	}  
	CWINDOW;

typedef
	CWINDOW CFORM;

#ifndef __CWINDOW_CPP

extern GB_DESC CWindowMenusDesc[];
extern GB_DESC CWindowControlsDesc[];
extern GB_DESC CWindowDesc[];
extern GB_DESC CWindowsDesc[];
extern GB_DESC CFormDesc[];

extern CWINDOW *CWINDOW_Active;

#else

#define THIS ((CWINDOW *)_object)
#define WINDOW ((gMainWindow*)THIS->ob.widget)
  
#endif

CWINDOW* WINDOW_get_main();
void WINDOW_kill(CWINDOW *win);

#endif

