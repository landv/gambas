/***************************************************************************

  CSVG.c

  GTK+ SVG loader component

  (C) 2006 Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CSVG_C

#include <stdio.h>
#include <gtk/gtk.h>
#include <librsvg/rsvg.h>
#include "main.h"
#include "CSVG.h"

GB_CLASS SVG_class;

BEGIN_METHOD_VOID(CSVG_init)

	SVG_class=GB.FindClass("SVG");

END_METHOD

BEGIN_METHOD_VOID(CSVG_free)

	if (THIS->handle) rsvg_handle_free(THIS->handle);

END_METHOD

BEGIN_METHOD(CSVG_load,GB_STRING Path;)

	RsvgHandle  *handle=NULL;
	CSVG        *ret=NULL;
	char        *addr=NULL;
	int        len=0;
	int        sck=1024;

	if ( GB.LoadFile (STRING(Path),LENGTH(Path),&addr,&len ) )
	{
		GB.Error("File not found");
		GB.ReturnNull();
		return;
	}

	if (!len) 
	{
		GB.ReleaseFile (&addr,len); 
		GB.Error("Invalid format");
		GB.ReturnNull(); 
		return; 
	}

	handle=rsvg_handle_new();
	rsvg_handle_set_dpi(handle,72);

	if (!handle) 
	{ 
		GB.ReleaseFile (&addr,len);
		GB.Error("Unable to create SVG handle"); 
		GB.ReturnNull();
		return;
	}
	
	while (len)
	{
		if (len<1024) sck=len;

		len-=sck;	

		if (!rsvg_handle_write(handle,(const guchar*)addr,sck,NULL))
		{
			rsvg_handle_free(handle);
			handle=NULL;
			GB.ReleaseFile (&addr,len);
			GB.Error("Invalid format");
			GB.ReturnNull();
			return;		
		}
		addr+=sck;
	}

	GB.ReleaseFile (&addr,len);
	if (!rsvg_handle_close(handle,NULL))
	{
		rsvg_handle_free(handle);
		handle=NULL;
		GB.Error("Invalid format");
		GB.ReturnNull();
		return;
	}
	

	GB.New (POINTER(&ret),SVG_class,NULL,NULL);
	ret->handle=handle;
	ret->dpi=72;
	GB.ReturnObject((void*)ret);

END_METHOD

BEGIN_PROPERTY(CSVG_image)

	GB_IMAGE ret=NULL;
	GdkPixbuf *buf;
	int w,h;
	unsigned char *p;

	if (!THIS->handle) { GB.ReturnNull(); return; }
	
	buf=rsvg_handle_get_pixbuf(THIS->handle);
	if (!buf) { GB.ReturnNull(); return; }

	w=gdk_pixbuf_get_width(buf);
	h=gdk_pixbuf_get_height(buf);
	p=gdk_pixbuf_get_pixels(buf);

	switch(gdk_pixbuf_get_n_channels(buf))
	{
		case 3:
			GB.Image.Create(&ret,(void*)p,w,h,GB_IMAGE_RGB);
			break;
		case 4:
			GB.Image.Create(&ret,(void*)p,w,h,GB_IMAGE_RGBA);
			break;
	}

	g_object_unref(G_OBJECT(buf));
	GB.ReturnObject((void*)ret);


END_PROPERTY

BEGIN_PROPERTY(CSVG_picture)

	GB_PICTURE ret=NULL;
	GdkPixbuf *buf;
	int w,h;
	unsigned char *p;

	if (!THIS->handle) { GB.ReturnNull(); return; }
	
	buf=rsvg_handle_get_pixbuf(THIS->handle);
	if (!buf) { GB.ReturnNull(); return; }

	w=gdk_pixbuf_get_width(buf);
	h=gdk_pixbuf_get_height(buf);
	p=gdk_pixbuf_get_pixels(buf);

	switch(gdk_pixbuf_get_n_channels(buf))
	{
		case 3:
			GB.Picture.Create(&ret,(void*)p,w,h,GB_IMAGE_RGB);
			break;
		case 4:
			GB.Picture.Create(&ret,(void*)p,w,h,GB_IMAGE_RGBA);
			break;
	}

	g_object_unref(G_OBJECT(buf));
	GB.ReturnObject((void*)ret);


END_PROPERTY

BEGIN_PROPERTY(CSVG_width)

	RsvgDimensionData data;

	if (!THIS->handle) { GB.ReturnInteger(0); return; }
	
	rsvg_handle_get_dimensions(THIS->handle,&data);
	GB.ReturnInteger(data.width);
	

END_PROPERTY

BEGIN_PROPERTY(CSVG_height)

	RsvgDimensionData data;

	if (!THIS->handle) { GB.ReturnInteger(0); return; }
	
	rsvg_handle_get_dimensions(THIS->handle,&data);
	GB.ReturnInteger(data.height);

END_PROPERTY

BEGIN_PROPERTY (CSVG_dpi)

	if (READ_PROPERTY) { GB.ReturnFloat(THIS->dpi); return; }
	
	THIS->dpi=VPROP(GB_FLOAT);
	rsvg_handle_set_dpi(THIS->handle,THIS->dpi);

END_PROPERTY

GB_DESC CSVGDesc[] =
{

  GB_DECLARE("SVG", sizeof(CSVG)), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_init",NULL,CSVG_init,NULL),
  GB_METHOD("_free",NULL,CSVG_free,NULL),

  GB_STATIC_METHOD("Load","SVG",CSVG_load,"(Path)s"),

  GB_PROPERTY_READ("Width","i",CSVG_width),
  GB_PROPERTY_READ("Height","i",CSVG_height),
  GB_PROPERTY_READ("Image","Image",CSVG_image),
  GB_PROPERTY_READ("Picture","Picture",CSVG_picture),

  GB_PROPERTY("DPI","f",CSVG_dpi),

  GB_END_DECLARE
};


