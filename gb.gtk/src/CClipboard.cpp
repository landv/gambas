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

#include "gclipboard.h"

#include "CWidget.h"
#include "CClipboard.h"
#include "CPicture.h"

/***************************************************************************

  Clipboard

***************************************************************************/

BEGIN_METHOD_VOID(CCLIPBOARD_clear)

	gClipboard::clear();

END_METHOD

static char *get_format(int i = 0, bool charset = false)
{
  char *format = gClipboard::getFormat(i);
  char *p;

	if (format && !charset)
	{
		p = index(format, ';');
		if (p)
		{
			char *f = g_strndup(format, p - format);
			g_free(format);
			format = f;
		}
	}
	
  return format;
}

static void get_formats(GB_ARRAY array)
{
  int i, j;
  char *fmt = NULL;
  char *str;
  
  for (i = 0;; i++)
  {
  	if (fmt) g_free(fmt);
  	
  	fmt = get_format(i, true);
  	if (!fmt)
  		break;
  		
    if (*fmt < 'a' || *fmt > 'z')
      continue;
    
    for (j = 0; j < GB.Array.Count(array); j++)
    {
      if (strcasecmp(fmt, *((char **)GB.Array.Get(array, j))) == 0)
        break;
    }
    
    if (j < GB.Array.Count(array))
      continue;
		
		GB.NewString(&str, fmt, 0);
		*((char **)GB.Array.Add(array)) = str;
  }
  
  if (fmt) g_free(fmt);
}

static bool exist_format(char *format)
{
  int i;
  char *fmt = NULL;
  bool ret = false;
  
  for (i = 0;; i++)
  {
  	if (fmt) g_free(fmt);
  	
  	fmt = get_format(i, true);
  	if (!fmt)
  		break;
  		
    if (*fmt < 'a' || *fmt > 'z')
      continue;
    
    if (!strcasecmp(format, fmt))
    {
    	ret = true;
    	break;
    }
  }
  
  if (fmt) g_free(fmt);
  return ret;
}

BEGIN_PROPERTY(CCLIPBOARD_format)

	char *buf = get_format();
	
	GB.ReturnNewString(buf, 0);
	if (buf)
		g_free(buf);

END_PROPERTY

BEGIN_PROPERTY(CCLIPBOARD_formats)

  GB_ARRAY array;
  
  GB.Array.New(&array, GB_T_STRING, 0);
  get_formats(array);
  GB.ReturnObject(array);

END_PROPERTY

BEGIN_PROPERTY(CCLIPBOARD_type)

	GB.ReturnInteger(gClipboard::getType());

END_PROPERTY

BEGIN_METHOD(CCLIPBOARD_copy, GB_VARIANT data; GB_STRING format)

	char *format;
	
	if (VARG(data).type == GB_T_STRING)
	{
		if (MISSING(format))
			format = NULL;
		else
		{
			format = GB.ToZeroString(ARG(format));
			if (strlen(format) < 6 || strncmp(format, "text/", 5) )
				goto _BAD_FORMAT;
		}
		
		gClipboard::setText(VARG(data)._string.value, format);
		return;
	}
	
	if (VARG(data).type >= GB_T_OBJECT && GB.Is(VARG(data)._object.value, GB.FindClass("Image")))
	{
		CIMAGE *img;
		
		if (!MISSING(format))
			goto _BAD_FORMAT;
		
		img = (CIMAGE *)VARG(data)._object.value;
		gClipboard::setImage(img->picture);
		return;
	}

_BAD_FORMAT:

  GB.Error("Bad clipboard format");

END_METHOD


BEGIN_METHOD(CCLIPBOARD_paste, GB_STRING format)

	CIMAGE *img;
	char *data;
	char *format;
	
	if (!MISSING(format))
	{
		format = GB.ToZeroString(ARG(format));
		if (!exist_format(format))
		{
			GB.ReturnNull();
			return;
		}
	}
	
	switch(gClipboard::getType())
	{
		case gClipboard::Text:
			data = gClipboard::getText();
			GB.ReturnNewString(data, 0);
			if (data) g_free(data);
			break;
		
		case gClipboard::Image:
		  img = CIMAGE_create(gClipboard::getImage());
		  GB.ReturnObject((void *)img);
		  break;
		  
		case gClipboard::Nothing:
		default:
			GB.ReturnNull();
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
  GB_STATIC_PROPERTY_READ("Formats", "String[]", CCLIPBOARD_formats),
  GB_STATIC_PROPERTY_READ("Type", "i", CCLIPBOARD_type),

  GB_STATIC_METHOD("Copy", NULL, CCLIPBOARD_copy, "(Data)v[(Format)s]"),
  GB_STATIC_METHOD("Paste", "v", CCLIPBOARD_paste, "[(Format)s]"),

  GB_END_DECLARE
};


/***************************************************************************

  Drag

***************************************************************************/

void CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, char *format)
{
  if (GB.CheckObject(source))
    return;

	if (gDrag::isActive())
	{
		GB.Error("Undergoing drag");
		return;
	}

  if (data->type == GB_T_STRING)
  {
  	if (format)
  	{
  		if (strlen(format) < 5)
  			goto _BAD_FORMAT;
  		if (strncasecmp(format, "text/", 5))
        goto _BAD_FORMAT;
    }
    
    gDrag::dragText(source->widget, data->_string.value, format);
  }
  else if (data->type >= GB_T_OBJECT && GB.Is(data->_object.value, GB.FindClass("Image")))
  {
    if (format && *format)
      goto _BAD_FORMAT;

		gDrag::dragImage(source->widget, ((CIMAGE *)data->_object.value)->picture);
  }
  else
    goto _BAD_FORMAT;

  //hide_frame(NULL);
  //GB.Post((GB_POST_FUNC)post_exit_drag, 0);

  return;

_BAD_FORMAT:

  GB.Error("Bad drag format");
}

BEGIN_METHOD(CDRAG_call, GB_OBJECT source; GB_VARIANT data; GB_STRING format)

	CDRAG_drag((CWIDGET *)VARG(source), &VARG(data), MISSING(format) ? NULL : GB.ToZeroString(ARG(format)));

END_METHOD


BEGIN_PROPERTY(CDRAG_icon)

	if (READ_PROPERTY)
	{
		gPicture *pic = gDrag::getIcon();
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		gDrag::setIcon(pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CDRAG_icon_x)

	int x, y;
	
	gDrag::getIconPos(&x, &y);

	if (READ_PROPERTY)
		GB.ReturnInteger(x);
	else
		gDrag::setIconPos(VPROP(GB_INTEGER), y);

END_PROPERTY


BEGIN_PROPERTY(CDRAG_icon_y)

	int x, y;
	
	gDrag::getIconPos(&x, &y);

	if (READ_PROPERTY)
		GB.ReturnInteger(y);
	else
		gDrag::setIconPos(x, VPROP(GB_INTEGER));

END_PROPERTY


#define CHECK_VALID() \
  if (!gDrag::isEnabled()) \
  { \
    GB.Error("No drag data"); \
    return; \
  }


BEGIN_PROPERTY(CDRAG_type)

	CHECK_VALID();
	GB.ReturnInteger(gDrag::getType());

END_PROPERTY


BEGIN_PROPERTY(CDRAG_format)

	CHECK_VALID();
	GB.ReturnNewZeroString(gDrag::getFormat());

END_PROPERTY


BEGIN_PROPERTY(CDRAG_formats)

	CHECK_VALID();
	stub("Drag.Formats");

END_PROPERTY


static void paste_drag(char *format)
{
	CIMAGE *image;

	//if (format)
	//	g_debug("format: %s drag: %s\n", format, gDrag::getFormat());
	
	if (format && strcasecmp(format, gDrag::getFormat()))
	{
		GB.ReturnNull();
		return;
	}
	
	switch(gDrag::getType())
	{
		case gDrag::Text:
			GB.ReturnNewZeroString(gDrag::getDropText());
			break;
		
		case gClipboard::Image:
		  image = CIMAGE_create(gDrag::getDropImage()->copy());
		  GB.ReturnObject((void *)image);
		  break;
		  
		default:
			GB.ReturnNull();
	}
}

BEGIN_PROPERTY(CDRAG_data)

  if (!gDrag::isActive())
  {
    GB.ReturnNull();
    return;
  }

  paste_drag(NULL);

END_PROPERTY


BEGIN_METHOD(CDRAG_paste, GB_STRING format)

  if (!gDrag::isActive())
  {
    GB.ReturnNull();
    return;
  }

  paste_drag(MISSING(format) ?  NULL : GB.ToZeroString(ARG(format)));

END_METHOD


BEGIN_PROPERTY(CDRAG_action)

	CHECK_VALID();
	GB.ReturnInteger(gDrag::getAction());

END_PROPERTY


BEGIN_PROPERTY(CDRAG_source)

	CHECK_VALID();
	GB.ReturnObject(GetObject(gDrag::getSource()));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_x)

	CHECK_VALID();
	GB.ReturnInteger(gDrag::getDropX());

END_PROPERTY


BEGIN_PROPERTY(CDRAG_y)

	CHECK_VALID();
	GB.ReturnInteger(gDrag::getDropY());

END_PROPERTY


BEGIN_METHOD_VOID(CDRAG_exit)

	gDrag::exit();

END_METHOD


BEGIN_PROPERTY(CDRAG_pending)

	GB.ReturnBoolean(gDrag::isActive());

END_PROPERTY


BEGIN_METHOD(CDRAG_show, GB_OBJECT control; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	if (GB.CheckObject(VARG(control)))
		return;
		
	if (!gDrag::isActive())
	{
		GB.Error("No undergoing drag");
		return;
	}

	if (MISSING(x) || MISSING(y) || MISSING(w) || MISSING(h))
		gDrag::show(((CWIDGET *)VARG(control))->widget);
	else
		gDrag::show(((CWIDGET *)VARG(control))->widget, VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD


BEGIN_METHOD_VOID(CDRAG_hide)

	gDrag::hide();
	
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

  GB_STATIC_PROPERTY("Icon", "Picture", CDRAG_icon),
  GB_STATIC_PROPERTY("IconX", "i", CDRAG_icon_x),
  GB_STATIC_PROPERTY("IconY", "i", CDRAG_icon_y),

  GB_STATIC_PROPERTY_READ("Data", "v", CDRAG_data),
  GB_STATIC_PROPERTY_READ("Format", "s", CDRAG_format),
  GB_STATIC_PROPERTY_READ("Formats", "String[]", CDRAG_formats),
  GB_STATIC_PROPERTY_READ("Type", "i", CDRAG_type),
  GB_STATIC_PROPERTY_READ("Action", "i", CDRAG_action),
  GB_STATIC_PROPERTY_READ("Source", "Control", CDRAG_source),
  GB_STATIC_PROPERTY_READ("X", "i", CDRAG_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAG_y),
  GB_STATIC_PROPERTY_READ("Pending", "b", CDRAG_pending),

  GB_STATIC_METHOD("_call", NULL, CDRAG_call, "(Source)Control;(Data)v[(Format)s]"),
  GB_STATIC_METHOD("_exit", NULL, CDRAG_exit, NULL),
  GB_STATIC_METHOD("Show", NULL, CDRAG_show, "(Control)Control;[(X)i(Y)i(Width)i(Height)i]"),
  GB_STATIC_METHOD("Hide", NULL, CDRAG_hide, NULL),
  GB_STATIC_METHOD("Paste", "v", CDRAG_paste, "[(Format)s]"),

  GB_END_DECLARE
};

