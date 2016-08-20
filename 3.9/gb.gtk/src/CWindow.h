/***************************************************************************

  CWindow.h

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
//extern GB_DESC CWindowTypeDesc[];
extern GB_DESC CWindowsDesc[];
extern GB_DESC CFormDesc[];

extern CWINDOW *CWINDOW_Active;
extern CWINDOW *CWINDOW_Main;

#else

#define THIS ((CWINDOW *)_object)
#define WINDOW ((gMainWindow*)THIS->ob.widget)
  
#endif

void CWINDOW_check_main_window(CWINDOW *win);
bool CWINDOW_must_quit();
void CWINDOW_close_all();
void CWINDOW_delete_all();

#endif

