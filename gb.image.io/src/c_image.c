/***************************************************************************

  c_image.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __C_IMAGE_C

#include "c_image.h"

#define LOAD_INC 65536L

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	GdkPixbuf *img = NULL;
	char *addr, *laddr;
	int len, llen;
	GdkPixbufLoader* loader;
	GError *error = NULL;
	gsize size;
	int format;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		GB.Error("Unable to load image");
		return;
	}
		
	loader = gdk_pixbuf_loader_new();
	
	laddr = addr;
	llen = len;
	while (llen > 0)
	{
		size = llen > LOAD_INC ? LOAD_INC : llen;
		if (!gdk_pixbuf_loader_write(loader, (guchar*)laddr, size, &error))
		{
			GB.Error(error->message);
			goto __END;
		}
		laddr += size;
		llen -= size;
	}
	
	if (!gdk_pixbuf_loader_close(loader, &error))
	{
		GB.Error(error->message);
		goto __END;
	}
	
	img = gdk_pixbuf_loader_get_pixbuf(loader);
	g_object_ref(G_OBJECT(img));
	
	/*if (gdk_pixbuf_get_n_channels(img) == 3)
	{
		// BM: convert to 4 bytes per pixels
		GdkPixbuf *aimg;
		aimg = gdk_pixbuf_add_alpha(img, FALSE, 0, 0, 0);
		g_object_unref(G_OBJECT(img));
  	g_object_ref(G_OBJECT(aimg));
		img = aimg;
	}*/
	
	switch (gdk_pixbuf_get_n_channels(img))
	{
		case 3: format = GB_IMAGE_RGB; break;
		case 4: format = GB_IMAGE_RGBA; break;
		default: GB.Error("Unsupported number of channels"); goto __END;
	}
	
	GB.ReturnObject(IMAGE.Create(gdk_pixbuf_get_width(img), gdk_pixbuf_get_width(img), format, gdk_pixbuf_get_pixels(img)));
	g_object_unref(G_OBJECT(img));

__END:
	
	g_object_unref(G_OBJECT(loader));
	GB.ReleaseFile(addr, len);

END_METHOD

static const char *FILE_get_ext(const char *path)
{
  const char *p;

  p = rindex(path, '/');
  if (p)
    path = &p[1];

  p = rindex(path, '.');
  if (p == NULL)
    return &path[strlen(path)];
  else
    return p + 1;
}

BEGIN_METHOD(CIMAGE_save, GB_STRING path; GB_INTEGER quality)

	char *path = GB.FileName(STRING(path), LENGTH(path));
	const char *ext = NULL;
	bool ok = FALSE;
	int b;
	char *format = NULL;
	GSList *formats, *iter;
	GdkPixbuf *image = NULL;
	int quality = VARGOPT(quality, -1);
	GError *error = NULL;
	
	ext = FILE_get_ext(path);
	if (!ext || !*ext)
	{
		GB.Error("No extension specified");
		goto __END;
	}

	IMAGE.Convert(THIS, GB_IMAGE_RGBA);
	image = gdk_pixbuf_new_from_data((const guchar *)THIS->data, GDK_COLORSPACE_RGB, TRUE, 8, THIS->width, THIS->height, THIS->width * sizeof(uint), NULL, NULL);

	formats = gdk_pixbuf_get_formats();
	
	iter = formats;
	while (iter && (!ok) )
	{
		if (gdk_pixbuf_format_is_writable((GdkPixbufFormat*)iter->data))
		{
			format = gdk_pixbuf_format_get_name((GdkPixbufFormat*)iter->data);
			if (!strcasecmp(format, ext))
			{
				ok = TRUE;
				break;
			}
			else
				g_free(format);
		}
		iter = iter->next;
	}

	g_slist_free(formats);
	
	if (!ok)
	{
		if (!strcasecmp("jpg", ext))
			format = (char *)"jpeg";
	}
	
	if (!format)
	{
		GB.Error("Unknown format");
		goto __END;
	}

	if (quality >= 0)
		b = gdk_pixbuf_save(image, path, format, &error, "quality", quality, (void *)NULL);
	else
		b = gdk_pixbuf_save(image, path, format, &error, (void *)NULL);

	if (ok)
		g_free(format);

	if (!b)
	{
		GB.Error(error->message);
		goto __END;
	}
	
__END:

	if (image)
		g_object_unref(G_OBJECT(image));

END_METHOD

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s[(Quality)i]"),
  
  GB_END_DECLARE
};

