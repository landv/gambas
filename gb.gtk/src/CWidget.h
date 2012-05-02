/***************************************************************************

  CWidget.h

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

#ifndef __CWIDGET_H
#define __CWIDGET_H

#include "main.h"
#include "gcontrol.h"
#include "gplugin.h"

#ifndef __CWIDGET_CPP

extern GB_DESC CWidgetDesc[];
extern GB_DESC CPluginDesc[];

#else

#define THIS ((CWIDGET *)_object)
#define CONTROL (THIS->widget)
#define PLUGIN (((CPLUGIN*)_object)->widget)
#define CPLUGIN_PROPERTIES "*"

#endif

typedef  
	struct
	{
	  GB_BASE ob;
	  gControl *widget;
		GB_VARIANT_VALUE tag;
		void *font;
		void *cursor;
		char *popup;
		char *action;
	}  
	CWIDGET;

typedef
	struct
	{
		GB_BASE ob;
		gPlugin *widget;
		GB_VARIANT_VALUE tag;
		void *font;
		char *popup;
	}
	CPLUGIN;


void InitControl(gControl *control, CWIDGET *widget);
void DeleteControl(gControl *control);
CWIDGET *GetContainer(CWIDGET *control);
#define GetObject(_control) ((CWIDGET *)((_control) ? (_control)->hFree : NULL))

#define CWIDGET_PROPERTIES CCONTROL_PROPERTIES

#define CONTAINER(_parent) ((gContainer *)GetContainer((CWIDGET *)_parent)->widget)

DECLARE_PROPERTY(CCONTROL_action);

int CWIDGET_check(void *_object);
int CWIDGET_get_handle(void *_object);

void CACTION_register(void *control, const char *old, const char *key);
void CACTION_raise(void *control);

#endif

