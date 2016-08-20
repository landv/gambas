/***************************************************************************

  c_image.c

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_IMAGE_C

#include "c_image.h"

#define LOAD_INC 65536L

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	GdkPixbuf *img = NULL;
	char *addr, *laddr;
	int len, llen;
	GdkPixbufLoader* loader = NULL;
	GError *error = NULL;
	gsize size;
	int format;
	GB_IMG *image;

	/*if (LENGTH(path) && *STRING(path) == '/')
	{
		img = gdk_pixbuf_new_from_file(GB.ToZeroString(ARG(path)), &error);
		if (error)
		{
			GB.Error(error->message);
			return;
		}
	}
	else
	{*/
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
				fprintf(stderr, "error write: %ld\n", size);
				GB.Error(error->message);
				goto __END;
			}
			laddr += size;
			llen -= size;
		}

		gdk_pixbuf_loader_close(loader, NULL);

		img = gdk_pixbuf_loader_get_pixbuf(loader);
		if (!img)
		{
			GB.Error("Unable to load image");
			goto __END;
		}

		g_object_ref(G_OBJECT(img));

	//}
	
	// Rowstride breaks gb.image (it is rounded up so that a line is always a four bytes multiple).
	if (gdk_pixbuf_get_n_channels(img) == 3)
	{
		GdkPixbuf *aimg;
		aimg = gdk_pixbuf_add_alpha(img, FALSE, 0, 0, 0);
		g_object_unref(G_OBJECT(img));
  	img = aimg;
	}
	
	//fprintf(stderr, "nchannels = %d size = %d x %d rowstride = %d\n", gdk_pixbuf_get_n_channels(img), gdk_pixbuf_get_width(img), gdk_pixbuf_get_height(img), gdk_pixbuf_get_rowstride(img));
	
	switch (gdk_pixbuf_get_n_channels(img))
	{
		case 3: format = GB_IMAGE_RGB; break;
		case 4: format = GB_IMAGE_RGBA; break;
		default: 
			g_object_unref(G_OBJECT(img));
			GB.Error("Unsupported number of channels"); 
			goto __END;
	}
	
	image = IMAGE.Create(gdk_pixbuf_get_width(img), gdk_pixbuf_get_height(img), format, gdk_pixbuf_get_pixels(img));
	IMAGE.Convert(image, IMAGE.GetDefaultFormat());
	GB.ReturnObject(image);
	
	g_object_unref(G_OBJECT(img));

__END:
	
	if (loader)
	{
		g_object_unref(G_OBJECT(loader));
		GB.ReleaseFile(addr, len);
	}

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
	char arg[4];
	GError *error = NULL;
	
	ext = FILE_get_ext(path);
	if (!ext || !*ext)
	{
		GB.Error("No extension specified");
		goto __END;
	}

	SYNCHRONIZE_IMAGE(THIS);
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
	{
		if (strcmp(format, "jpeg") == 0)
		{
			if (quality > 100)
				quality = 100;
			sprintf(arg, "%d", quality);
			b = gdk_pixbuf_save(image, path, format, &error, "quality", arg, (void *)NULL);
		}
		else if (strcmp(format, "png") == 0)
		{
			if (quality > 9)
				quality = 9;
			sprintf(arg, "%d", quality);
			b = gdk_pixbuf_save(image, path, format, &error, "compression", arg, (void *)NULL);
		}
		else
			b = gdk_pixbuf_save(image, path, format, &error, (void *)NULL);
	}
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


static GdkPixbuf *stretch_pixbuf(GdkPixbuf *img, int w, int h)
{
	GdkPixbuf *image = NULL;
	int wimg, himg;
	int ws, hs;

	if (w <= 0 && h <= 0)
		goto __RETURN;

	wimg = gdk_pixbuf_get_width(img);
	himg = gdk_pixbuf_get_height(img);

	if (w < 0)
		w = wimg * h / himg;
	else if (h < 0)
		h = himg * w / wimg;

	if (w <= 0 || h <= 0)
		goto __RETURN;

	ws = w;
	hs = h;
	if (ws < (wimg / 4))
		ws = w * 4;
	if (hs < (himg / 4))
		hs = h * 4;
	if (ws != w || hs != h)
	{
		image = gdk_pixbuf_scale_simple(img, ws, hs, GDK_INTERP_NEAREST);
		g_object_unref(G_OBJECT(img));
		img = image;
	}

	image = gdk_pixbuf_scale_simple(img, w, h, GDK_INTERP_BILINEAR);

__RETURN:

	g_object_unref(G_OBJECT(img));
	return image;
}

BEGIN_METHOD(Image_Stretch, GB_INTEGER width; GB_INTEGER height)

	GdkPixbuf *img;
	GB_IMG *image;

	SYNCHRONIZE_IMAGE(THIS);
	IMAGE.Convert(THIS, GB_IMAGE_RGBA);
	img = gdk_pixbuf_new_from_data((const guchar *)THIS->data, GDK_COLORSPACE_RGB, TRUE, 8, THIS->width, THIS->height, THIS->width * sizeof(uint), NULL, NULL);

	img = stretch_pixbuf(img, VARG(width), VARG(height));

	image = IMAGE.Create(gdk_pixbuf_get_width(img), gdk_pixbuf_get_height(img), GB_IMAGE_RGBA, gdk_pixbuf_get_pixels(img));
	IMAGE.Convert(image, IMAGE.GetDefaultFormat());
	GB.ReturnObject(image);

	g_object_unref(G_OBJECT(img));

END_METHOD


GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s[(Quality)i]"),
  GB_METHOD("Stretch", "Image", Image_Stretch, "(Width)i(Height)i"),

  GB_END_DECLARE
};

