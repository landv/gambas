/***************************************************************************

  CClipboard.cpp

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


#define __CCLIPBOARD_CPP


#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "CWidget.h"
#include "CClipboard.h"
#include "CImage.h"
#include "CPicture.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

static void *CLASS_Image=NULL;
static CPICTURE *DRAG_picture;

/***************************************************************************

  Clipboard

***************************************************************************/


BEGIN_METHOD_VOID(CCLIPBOARD_clear)

	clipBoard_Clear();

END_METHOD


BEGIN_PROPERTY(CCLIPBOARD_format)

	char *buf;
	
	buf=clipBoard_Format();
	GB.ReturnNewString(buf,0);
	if (buf) free(buf);

END_PROPERTY


BEGIN_PROPERTY(CCLIPBOARD_type)

	GB.ReturnInteger(clipBoard_Type());

END_PROPERTY


BEGIN_METHOD(CCLIPBOARD_copy, GB_VARIANT data; GB_STRING format)

	GB_VARIANT Data;
	CIMAGE *img;
		
	if (VARG(data).type == GB_T_STRING)
	{
		if (MISSING(format))
			clipBoard_setText(VARG(data)._string.value,"text/plain");
		else
		{
			if ( LENGTH(format) <  6) { GB.Error("Bad clipboard format"); return; }
			if ( strncmp (STRING(format),"text/",5) ) { GB.Error("Bad clipboard format"); return; }
			clipBoard_setText(VARG(data)._string.value,STRING(format));
		}
		
		return;
	}
	
	if (!CLASS_Image) CLASS_Image = GB.FindClass("Image");
	
	if (VARG(data).type >= GB_T_OBJECT && GB.Is(VARG(data)._object.value, CLASS_Image))
	{
		if (!MISSING(format)) { GB.Error("Bad clipboard format"); return; }
		
		img = (CIMAGE *)VARG(data)._object.value;
		clipBoard_setImage(img->image);
		return;
	}

	GB.Error("Bad clipboard format");

END_METHOD


BEGIN_METHOD(CCLIPBOARD_paste, GB_STRING format)

	CIMAGE *img;
	char *data;
	 
	if (clipBoard_Type()==1)
	{
		data=clipBoard_getText();
		GB.ReturnNewString(data,0);
		if (data) free(data);
	}
	
	if (clipBoard_Type()==2)
	{
		GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
		if (img->image) delete img->image;
		img->image=clipBoard_getImage();
		GB.ReturnObject((void*)img);
	}


END_METHOD


GB_DESC CClipboardDesc[] =
{
  GB_DECLARE("Clipboard", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_STATIC_METHOD("Clear", NULL, CCLIPBOARD_clear, NULL),

  GB_STATIC_PROPERTY_READ("Format", "s", CCLIPBOARD_format),
  GB_STATIC_PROPERTY_READ("Type", "i", CCLIPBOARD_type),

  GB_STATIC_METHOD("Copy", NULL, CCLIPBOARD_copy, "(Data)v[(Format)s]"),
  GB_STATIC_METHOD("Paste", "v", CCLIPBOARD_paste, "[(Format)s]"),

  GB_END_DECLARE
};


/***************************************************************************

  Drag

***************************************************************************/


BEGIN_METHOD(CDRAG_call, GB_OBJECT source; GB_VARIANT data; GB_STRING format)


END_METHOD


BEGIN_PROPERTY(CDRAG_picture)

	CPICTURE *Pic;

	if (READ_PROPERTY)
	{
		GB.ReturnObject((void*)DRAG_picture);
		return;
	}
	
	Pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (Pic) GB.Ref((void*)Pic);
	if (DRAG_picture) GB.Unref((void**)&DRAG_picture);
	DRAG_picture=Pic;

	if (!Pic) drag_setIcon(NULL);
	else      drag_setIcon(Pic->picture);

END_PROPERTY



BEGIN_PROPERTY(CDRAG_type)

	if (!drag_IsEnabled()) { GB.Error("No drag data"); return; }
	GB.ReturnInteger(drag_Type());

END_PROPERTY


BEGIN_PROPERTY(CDRAG_format)

	if (!drag_IsEnabled()) { GB.Error("No drag data"); return; }
	GB.ReturnNewString(drag_Format(),0);

END_PROPERTY


BEGIN_PROPERTY(CDRAG_data)

	CIMAGE *img;

	if (!drag_IsEnabled()) { GB.ReturnNull();return; }
	
	if (drag_Type()==1)
	{
		GB.ReturnNewString(drag_Text(),0);
		return;
	}
	if (drag_Type()==2)
	{
		GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
		if (img->image) delete img->image;
		img->image=drag_Image();
		GB.ReturnObject((void*)img);
	}
	
	

END_PROPERTY


BEGIN_PROPERTY(CDRAG_action)

	if (!drag_IsEnabled()) { GB.Error("No drag data"); return; }
	GB.ReturnInteger(drag_Action());

END_PROPERTY


BEGIN_PROPERTY(CDRAG_source)

	if (!drag_IsEnabled()) { GB.Error("No drag data"); return; }
	gControl *src=gControl::dragWidget();
	if (!src) { GB.ReturnNull(); return; }
	GB.ReturnObject(GetObject(src));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_x)

	if (!drag_IsEnabled()) { GB.Error("No drag data"); return; }
	GB.ReturnInteger(drag_X());
	

END_PROPERTY


BEGIN_PROPERTY(CDRAG_y)

	if (!drag_IsEnabled()) { GB.Error("No drag data"); return; }
	GB.ReturnInteger(drag_Y());

END_PROPERTY

BEGIN_METHOD_VOID(CDRAG_init)

	DRAG_picture=NULL;

END_METHOD

BEGIN_METHOD_VOID(CDRAG_exit)

	if (DRAG_picture) { GB.Unref((void**)&DRAG_picture); DRAG_picture=NULL; }
	drag_setIcon(NULL);

END_METHOD


GB_DESC CDragDesc[] =
{
  GB_DECLARE("Drag", 0), GB_VIRTUAL_CLASS(),
  
  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_CONSTANT("Copy", "i", 0),
  GB_CONSTANT("Link", "i", 1),
  GB_CONSTANT("Move", "i", 2),

  GB_STATIC_PROPERTY("Icon", "Picture", CDRAG_picture),

  GB_STATIC_PROPERTY_READ("Data", "v", CDRAG_data),
  GB_STATIC_PROPERTY_READ("Format", "s", CDRAG_format),
  GB_STATIC_PROPERTY_READ("Type", "i", CDRAG_type),
  GB_STATIC_PROPERTY_READ("Action", "i", CDRAG_action),
  GB_STATIC_PROPERTY_READ("Source", "Control", CDRAG_source),
  GB_STATIC_PROPERTY_READ("X", "i", CDRAG_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAG_y),

  GB_STATIC_METHOD("_call", NULL, CDRAG_call, "(Source)Control;(Data)v[(Format)s]"),
  GB_STATIC_METHOD("_init",NULL,CDRAG_init,NULL),
  GB_STATIC_METHOD("_exit",NULL,CDRAG_exit,NULL),

  GB_END_DECLARE
};

