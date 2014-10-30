/***************************************************************************

  CClipboard.cpp

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

#define __CCLIPBOARD_CPP

#include "gclipboard.h"

#include "CWidget.h"
#include "CClipboard.h"
#include "CImage.h"
#include "CPicture.h"

/***************************************************************************

  Clipboard

***************************************************************************/

static CIMAGE *_clipboard_image = NULL;

BEGIN_METHOD_VOID(CCLIPBOARD_clear)

	gClipboard::clear();
	GB.StoreObject(NULL, POINTER(&_clipboard_image));

END_METHOD

static char *get_format(int i = 0, bool charset = false, bool drag = false)
{
  char *format = drag ? gDrag::getFormat(i) : gClipboard::getFormat(i);
  char *p;

	if (format && !charset)
	{
		p = index(format, ';');
		if (p)
		{
			format = gt_free_later(g_strndup(format, p - format));
		}
	}
	
  return format;
}

static void get_formats(GB_ARRAY array, bool drag = false)
{
  int i, j;
  char *fmt;
  
  for (i = 0;; i++)
  {
  	fmt = get_format(i, true, drag);
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
		
		*((char **)GB.Array.Add(array)) = GB.NewZeroString(fmt);
  }
}

static bool exist_format(char *format, bool drag = false)
{
  int i;
  char *fmt;
  bool ret = false;
  
  for (i = 0;; i++)
  {
  	fmt = get_format(i, true, drag);
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
  
  return ret;
}

BEGIN_PROPERTY(CCLIPBOARD_format)

	GB.ReturnNewZeroString(get_format());

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
		
		gClipboard::setText(VARG(data).value._string, -1, format);
		return;
	}
	
	if (VARG(data).type >= GB_T_OBJECT && GB.Is(VARG(data).value._object, GB.FindClass("Image")))
	{
		CIMAGE *img;
		
		if (!MISSING(format))
			goto _BAD_FORMAT;
		
		img = (CIMAGE *)VARG(data).value._object;
		GB.Unref(POINTER(&_clipboard_image));
		GB.Ref(img);
		_clipboard_image = img;
		gClipboard::setImage(CIMAGE_get(img));
		return;
	}

_BAD_FORMAT:

  GB.Error("Bad clipboard format");

END_METHOD


BEGIN_METHOD(CCLIPBOARD_paste, GB_STRING format)

	CIMAGE *img;
	char *format = NULL;
	char *text;
	int len;
	int type;
	
	type = gClipboard::getType();

	if (!MISSING(format))
	{
		format = GB.ToZeroString(ARG(format));
		if (!exist_format(format))
		{
			GB.ReturnVariant(NULL);
			return;
		}
		if (strncasecmp(format, "text/", 5) == 0)
			type = gClipboard::Text;
	}
	
	switch(type)
	{
		case gClipboard::Text:
			text = gClipboard::getText(&len, format);
			if (text)
				GB.ReturnNewString(text, len);
			else
				GB.ReturnNull();
			break;
		
		case gClipboard::Image:
		  img = CIMAGE_create(gClipboard::getImage());
		  GB.ReturnObject((void *)img);
		  break;
		  
		case gClipboard::Nothing:
		default:
			GB.ReturnNull();
	}
	
	GB.ReturnConvVariant();
	 
END_METHOD


GB_DESC CClipboardDesc[] =
{
  GB_DECLARE("Clipboard", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_STATIC_METHOD("_exit", 0, CCLIPBOARD_clear, 0),
  GB_STATIC_METHOD("Clear", 0, CCLIPBOARD_clear, 0),

  GB_STATIC_PROPERTY_READ("Format", "s", CCLIPBOARD_format),
  GB_STATIC_PROPERTY_READ("Formats", "String[]", CCLIPBOARD_formats),
  GB_STATIC_PROPERTY_READ("Type", "i", CCLIPBOARD_type),

  GB_STATIC_METHOD("Copy", 0, CCLIPBOARD_copy, "(Data)v[(Format)s]"),
  GB_STATIC_METHOD("Paste", "v", CCLIPBOARD_paste, "[(Format)s]"),

  GB_END_DECLARE
};


/***************************************************************************

  Drag

***************************************************************************/

void *CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, char *format)
{
	gControl *dest;
	
  if (GB.CheckObject(source))
    return NULL;

	if (gDrag::isActive())
	{
		GB.Error("Undergoing drag");
		return NULL;
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
    
    dest = gDrag::dragText(source->widget, data->value._string, format);
  }
  else if (data->type >= GB_T_OBJECT && GB.Is(data->value._object, GB.FindClass("Image")))
  {
    if (format && *format)
      goto _BAD_FORMAT;

		dest = gDrag::dragImage(source->widget, CIMAGE_get((CIMAGE *)data->value._object));
  }
  else
    goto _BAD_FORMAT;

  //hide_frame(NULL);
  //GB.Post((GB_POST_FUNC)post_exit_drag, 0);

  return GetObject(dest);

_BAD_FORMAT:

  GB.Error("Bad drag format");
  return NULL;
}

BEGIN_METHOD(CDRAG_call, GB_OBJECT source; GB_VARIANT data; GB_STRING format)

	GB.ReturnObject(CDRAG_drag((CWIDGET *)VARG(source), &VARG(data), MISSING(format) ? NULL : GB.ToZeroString(ARG(format))));

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
	
	GB.ReturnNewZeroString(get_format(0, false, true));

END_PROPERTY

BEGIN_PROPERTY(CDRAG_formats)

  GB_ARRAY array;
  
	CHECK_VALID();
	
  GB.Array.New(&array, GB_T_STRING, 0);
  get_formats(array, true);
  GB.ReturnObject(array);

END_PROPERTY



static void paste_drag(char *format)
{
	CIMAGE *image;
	char *text;
	int len;

	//if (format)
	//	g_debug("format: %s drag: %s\n", format, gDrag::getFormat());
	
	//fprintf(stderr, "paste_drag: %s\n", format);
	
	if (format)
	{
		if (!exist_format(format, true))
		{
			GB.ReturnVariant(NULL);
			return;
		}
	}
	
	switch(gDrag::getType())
	{
		case gDrag::Text:
			text = gDrag::getText(&len, format);
			if (text)
				GB.ReturnNewString(text, len);
			else
				GB.ReturnNull();
			break;
		
		case gDrag::Image:
		  image = CIMAGE_create(gDrag::getImage()->copy());
		  GB.ReturnObject((void *)image);
		  break;
		  
		default:
			GB.ReturnNull();
	}
	
	GB.ReturnConvVariant();
}

BEGIN_PROPERTY(CDRAG_data)

	CHECK_VALID();  
  
  if (!gDrag::isActive())
  {
    GB.ReturnNull();
    return;
  }

  paste_drag(NULL);

END_PROPERTY


BEGIN_METHOD(CDRAG_paste, GB_STRING format)

	CHECK_VALID();
  
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
		
	/*if (!gDrag::isActive())
	{
		GB.Error("No undergoing drag");
		return;
	}*/

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

  GB_CONSTANT("None", "i", gDrag::Nothing),
  GB_CONSTANT("Text", "i", gDrag::Text),
  GB_CONSTANT("Image", "i", gDrag::Image),

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

  GB_STATIC_METHOD("_call", "Control", CDRAG_call, "(Source)Control;(Data)v[(Format)s]"),
  GB_STATIC_METHOD("_exit", 0, CDRAG_exit, 0),
  GB_STATIC_METHOD("Show", 0, CDRAG_show, "(Control)Control;[(X)i(Y)i(Width)i(Height)i]"),
  GB_STATIC_METHOD("Hide", 0, CDRAG_hide, 0),
  GB_STATIC_METHOD("Paste", "v", CDRAG_paste, "[(Format)s]"),

  GB_END_DECLARE
};

