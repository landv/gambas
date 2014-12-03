/***************************************************************************

  CMenu.h

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

#ifndef __CMENU_H
#define __CMENU_H

#include "main.h"
#include "gmenu.h"
#include "CPicture.h"

#ifndef __CMENU_CPP
extern GB_DESC CMenuDesc[];
extern GB_DESC CMenuChildrenDesc[];
#else

#define THIS  ((CMENU*)_object)
#define MENU  ((gMenu*)THIS->widget)

#endif

typedef 
	struct
	{
		GB_BASE ob;
		gMenu *widget;
		GB_VARIANT_VALUE tag;
		char *action;
		char *save_text;
		unsigned init_shortcut : 1;
	} 
	CMENU;

void CMENU_check_popup_click(void);

#endif
