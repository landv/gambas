/***************************************************************************

  CTextBox.h

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

#ifndef __CTEXTBOX_H
#define __CTEXTBOX_H

#include "main.h"
#include "gtextbox.h"
#include "gcombobox.h"

#ifndef __CTEXTBOX_CPP

extern GB_DESC CTextBoxSelectionDesc[];
extern GB_DESC CTextBoxDesc[];
extern GB_DESC CComboBoxDesc[];
extern GB_DESC CComboBoxItemDesc[];

#else

#define THIS ((CTEXTBOX *)_object)
#define TEXTBOX ((gTextBox *)THIS->ob.widget)
#define COMBOBOX ((gComboBox *)THIS->ob.widget)

#endif

typedef  
	struct 
	{
		CWIDGET ob;
	} 
	CTEXTBOX;

typedef  
	struct 
	{
		CWIDGET ob;
		int index;
		bool click;
	} 
	CCOMBOBOX;

#endif
