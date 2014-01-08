/***************************************************************************

	gpicture.cpp

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

#include <math.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gpicture.h"


/*****************************************************************

gPicture

******************************************************************/

#define LOAD_INC 65536L

static bool pixbufFromMemory(GdkPixbuf **ppixbuf, char *addr, unsigned int len, bool *trans)
{
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	GError *error = NULL;
	gsize size;

	*ppixbuf = 0;

	loader = gdk_pixbuf_loader_new();
	
	while (len > 0)
	{
		size = len > LOAD_INC ? LOAD_INC : len;
		if (!gdk_pixbuf_loader_write(loader,(guchar*)addr,size, &error))
		{
			//g_debug("ERROR: %s", error->message);
			goto __ERROR;
		}
		addr += size;
		len -= size;
	}
	
	if (!gdk_pixbuf_loader_close(loader, &error))
	{
		//g_debug("ERROR: %s", error->message);
		goto __ERROR;
	}
	
	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	g_object_ref(pixbuf);
	
	if (gdk_pixbuf_get_n_channels(pixbuf) == 3)
	{
		// Rowstride breaks gb.image (it is rounded up so that a line is always a four bytes multiple).
		GdkPixbuf *aimg;
		aimg = gdk_pixbuf_add_alpha(pixbuf, FALSE, 0, 0, 0);
		g_object_unref(pixbuf);
		pixbuf = aimg;
		*trans = false;
	}
	else
		*trans = true;

	g_object_unref(G_OBJECT(loader));

	*ppixbuf = pixbuf;
	return true;
	
__ERROR:
	g_object_unref(G_OBJECT(loader));
	return false;
}


void gPicture::initialize()
{
#ifndef GTK3
	pixmap = NULL;
	mask = NULL;
#endif
	pixbuf = NULL;
	surface = NULL;
	_transparent = false;
	_type = VOID;
	_width = 0;
	_height = 0;
}

gPicture::gPicture() : gShare()
{
	//fprintf(stderr, "gPicture(): %p\n", this);
	initialize();
}

#ifdef GTK3
static cairo_surface_t *create_surface(int w, int h)
{
	return cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
}
#else
static GdkPixmap *create_pixmap(int w, int h)
{
	GdkScreen *scr;
	gint depth;
	GdkPixmap *pixmap;

	scr = gdk_screen_get_default();
	depth = (gdk_screen_get_system_visual(scr))->depth;

	pixmap = gdk_pixmap_new(NULL, w, h, depth);
	gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap), gdk_colormap_get_system());
	return pixmap;
}
#endif

#ifndef GTK3
void gPicture::createMask(bool opaque)
{
	GdkGC *gc;
	GdkGCValues gc_values;
	
	if (mask || !_transparent)
		return;
		
	mask = gdk_pixmap_new(NULL, _width, _height, 1);
	
	gc_values.foreground.pixel = opaque ? 1 : 0;
	gc = gdk_gc_new_with_values(mask, &gc_values, GDK_GC_FOREGROUND);
	
	gdk_gc_set_fill(gc, GDK_SOLID);
	gdk_draw_rectangle(mask, gc, TRUE, 0, 0, _width, _height);
	
	g_object_unref(gc);
}
#endif

gPicture::gPicture(gPictureType type, int w, int h, bool trans) : gShare()
{
	//fprintf(stderr, "gPicture(): %p: %d %d %d %d\n", this, type, w, h, trans);
	initialize();
	
	_transparent = trans;

	if (!type)
		return;

	if (w <= 0 || h <= 0) return;

	_type = type;
	_width = w;
	_height = h;

#ifdef GTK3
	if (_type == SURFACE)
	{
		surface = create_surface(w, h);
	}
#else
	if (_type == PIXMAP)
	{
		pixmap = create_pixmap(w, h);
		createMask(false);
	}
	else
#endif
	if (_type == PIXBUF)
	{
		pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
	}
}


// The gPicture takes the GdkPixbuf object. Do not unreference it after.

gPicture::gPicture(GdkPixbuf *image, bool trans) : gShare()
{
	//fprintf(stderr, "gPicture(image): %p: %p %d\n", this, image, trans);
	initialize();
	if (!image)
		return;
		
	_type = PIXBUF;
	_width = gdk_pixbuf_get_width(image);
	_height = gdk_pixbuf_get_height(image);
	_transparent = trans;
	pixbuf = image;

	if (gdk_pixbuf_get_n_channels(pixbuf) == 3)
	{
		GdkPixbuf *aimg;
		aimg = gdk_pixbuf_add_alpha(pixbuf, FALSE, 0, 0, 0);
		g_object_unref(G_OBJECT(pixbuf));
		//g_object_ref(G_OBJECT(aimg));
		pixbuf = aimg;
		_transparent = false;
	}

}

#ifdef GTK3

gPicture::gPicture(cairo_surface_t *surf) : gShare()
{
	initialize();
	if (!surf)
		return;
	_type = SURFACE;
	surface = surf;
	_width = cairo_image_surface_get_width(surf);
	_height = cairo_image_surface_get_height(surf);
}

#else

// The gPicture takes the GdkPixmap object. Do not unreference it after.

gPicture::gPicture(GdkPixmap *pix) : gShare()
{
	initialize();
	if (!pix)
		return;
		
	_type = PIXMAP;
	gdk_drawable_get_size((GdkDrawable *)pix, &_width, &_height);
	pixmap = pix;
}
#endif

gPicture::~gPicture()
{
	//fprintf(stderr, "~gPicture: %p\n", this);
	clear();
}

void gPicture::invalidate()
{
#ifndef GTK3
	if (pixmap && _type != PIXMAP)
	{
		g_object_unref(G_OBJECT(pixmap));
		pixmap = NULL;
		if (mask)
		{
			g_object_unref(mask);
			mask = NULL;
		}
	}
#endif
	
	if (pixbuf && _type != PIXBUF)
	{
		g_object_unref(G_OBJECT(pixbuf));
		pixbuf = NULL;
	}
	
	if (surface && _type != SURFACE)
	{
		cairo_surface_destroy(surface);
		surface = NULL;
	}
}

GdkPixbuf *gPicture::getPixbuf()
{
	if (_type == VOID)
		return NULL;
	
	if (pixbuf)
		return pixbuf;
	
#ifndef GTK3
	if (_type == PIXMAP)
	{
		pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, width(), height());
		gdk_pixbuf_get_from_drawable(pixbuf, pixmap, NULL, 0, 0, 0, 0, width(), height());
		
		if (mask)
		{
			uchar *s, *d;
			int i;
			GdkPixbuf *alpha;
			
			alpha = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, width(), height());
			gdk_pixbuf_get_from_drawable(alpha, mask, NULL, 0, 0, 0, 0, width(), height());
			s = gdk_pixbuf_get_pixels(alpha);
			d = gdk_pixbuf_get_pixels(pixbuf) + 3;
			//fprintf(stderr, "mask -> pixbuf\n\n");
			for (i = 0; i < (_width * _height); i++)
			{
				//fprintf(stderr, "%08X%c", *((uint *)s), (1 + (i % _width)) == _width ? '\n' : ' ');
				d[0] = s[0];
				d += 4;
				s += 4;
			}
			g_object_unref(alpha);
		}
	}
	else
#endif
	if (_type == SURFACE)
	{
#ifdef GTK3
		pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, _width, _height);
#else
		fprintf(stderr, "gb.gtk: warning: cairo surface to pixbuf conversion not implemented.\n");
		return NULL;
#endif
	}
	
	_type = PIXBUF;
	return pixbuf;
}

#ifdef GTK3
cairo_surface_t *gPicture::getSurface()
{
	if (_type == VOID)
		return NULL;

	if (_type != SURFACE)
	{
		getPixbuf();
		surface = gt_cairo_create_surface_from_pixbuf(pixbuf);
	}

	_type = SURFACE;
	return surface;
}
#else
cairo_surface_t *gPicture::getSurface()
{
	if (_type == VOID)
		return NULL;

	if (surface)
		return surface;

	getPixbuf();
	surface = gt_cairo_create_surface_from_pixbuf(pixbuf);

	return surface;
}
#endif

#ifndef GTK3
GdkPixmap *gPicture::getPixmap()
{
	if (_type == VOID)
		return NULL;
	
	if (_type != PIXMAP)
	{
		if (_type != PIXBUF)
			getPixbuf();
	
		if (pixmap)
			g_object_unref(G_OBJECT(pixmap));
		if (mask)
			g_object_unref(G_OBJECT(mask));

		gt_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &mask, 128);
	}
	
	_type = PIXMAP;
	return pixmap;
}

GdkBitmap *gPicture::getMask()
{
	getPixmap();
	return mask;
}
#endif


gPicture *gPicture::fromMemory(char *addr, unsigned int len)
{
	GdkPixbuf *pixbuf;
	bool trans;

	if (!pixbufFromMemory(&pixbuf, addr, len, &trans))
		return 0;
	else
	{
		gPicture *pic = new gPicture(pixbuf);
		return pic;
	}
}

gPicture *gPicture::fromData(const char *data, int width, int height)
{
	GdkPixbuf *pixbuf;
	
	if (width <= 0 || height <= 0)
		return new gPicture();
	else
	{
		pixbuf = gdk_pixbuf_new_from_data((const guchar *)data, GDK_COLORSPACE_RGB, TRUE, 8, width, height, width * sizeof(int), NULL, NULL);
		return new gPicture(pixbuf);
	}
}

int gPicture::depth()
{
	int depth = 0;

#ifndef GTK3
	if (pixmap) 
		depth = gdk_drawable_get_depth(GDK_DRAWABLE(pixmap));
	else
#endif
	if (pixbuf || surface)
		depth = 32;

	return depth;
}

void gPicture::setTransparent(bool vl)
{
	if (vl == _transparent)
		return;
		
	_transparent = vl;
	
#ifndef GTK3
	if (_type == PIXMAP)
	{
		if (_transparent)
			createMask(true);
		else
		{
			if (mask)
			{
				g_object_unref(G_OBJECT(mask));
				mask = 0;
			}
		}
	}
#endif
}

void gPicture::fill(gColor col)
{
#ifndef GTK3
	if (_type == PIXMAP)
	{
		gt_pixmap_fill(pixmap, col, NULL);
	}
	else
#endif
	if (_type == PIXBUF)
	{
		int r, g, b, a;
		union
		{
			char c[4];
			uint value;
		}
		color;
		
		gt_color_to_rgba(col, &r, &g, &b, &a);
		
		color.c[0] = a ^ 0xFF;
		color.c[1] = b;
		color.c[2] = g;
		color.c[3] = r;
		
		gdk_pixbuf_fill(pixbuf, color.value);
	}
#ifdef GTK3
	else if (_type == SURFACE)
	{
		cairo_t *cr = cairo_create(surface);
		gt_cairo_set_source_color(cr, col);
		cairo_paint(cr);
		cairo_destroy(cr);
	}
#endif
	
	invalidate();
}


// returns -> 0, OK / -1, Bad format / -2 invalid path

int gPicture::save(const char *path, int quality)
{
	bool ok=false;
	int b;
	char *type;
	const char *buf=NULL;
	GSList *formats = gdk_pixbuf_get_formats();
	GSList *iter=formats;
	GdkPixbuf *image = getPixbuf();
	char arg[16];

	for (b=strlen(path)-1;b>=0;b--)
		if (path[b]=='.') { buf=path+b+1; break; }

	if (!buf) return -1;

	while (iter && (!ok) )
	{
		if (gdk_pixbuf_format_is_writable ((GdkPixbufFormat*)iter->data))
		{
			type=gdk_pixbuf_format_get_name((GdkPixbufFormat*)iter->data);
			if (!strcasecmp(type,buf))
			{
				ok=true;
				break;
			}
			else
				g_free(type);
		}
		iter=iter->next;
	}

	if (!ok)
	{
		g_slist_free(formats);
		if (!strcasecmp("jpg", buf))
			type = (char *)"jpeg";
		else
			return -1;
	}

	if (quality >= 0)
	{
		sprintf(arg, "%d", quality);
		b = gdk_pixbuf_save(image, path, type, NULL, "quality", arg, (void *)NULL);
	}
	else
		b = gdk_pixbuf_save(image, path, type, NULL, (void *)NULL);


	if (ok) {
		g_free(type);
		g_slist_free(formats);
	}

	if (!b) return -2;
	return 0;
}


/***********************************************************************
The following function tries to load an icon from predefined or "stock"
items. It accepts the format: StockSize/IconName, where StockSize can be:

"Menu", "SmallToolBar","LargeToolBar","Button","Dnd","Dialog"

And IconName can be:

"Add","Apply","Bold","Cancel",
"CDRom","Clear","Close","ColorPicker",
"Convert","Copy","Cut","Delete",
"DialogAuthentication""DialogError","DialogInfo","DialogQuestion",
"DialogWarning","Dnd","DndMultiple", "Execute",
"Find","FindAndReplace","Floppy","GotoBottom",
"GotoFirst", "GotoLast","GotoTop","GoBack",
"GoDown","GoForward","GoUp","HardDisk"
"Help","Home","Indent","Index",
"Italic","JumpTo","JustifyCenter", "JustifyFill",
"JustifyLeft","JustifyRight","MissingImage","Network",
"New","No","Ok","Open",
"Paste","Preferences","Print","PrintPreview",
"Properties","Quit","Redo","Refresh",
"Remove","RevertToSaved","Save","SaveAs",
"SelectColor","SelectFont","SortAscending","SortDescending",
"SpellCheck","Stop","StrikeThrough","Undelete",
"Underline","Undo","Unindent","Yes",
"Zoom100","ZoomFit","ZoomIn","ZoomOut"
*************************************************************************/



/***********************************************************************
The following function tries to load an icon from predefined system
paths
***********************************************************************/
gPicture* gPicture::fromNamedIcon(const char *name, int len)
{
	GtkIconTheme* theme;
	GdkPixbuf *buf;
	gPicture *pic = NULL;
	int r_type=32;
	char *c_name, *r_name;
	
	if (len < 0) len = strlen(name);
	
	c_name = g_strndup(name, len);
	r_name = strchr(c_name, '/');

	if (!r_name)
		r_name = c_name;
	else
	{
		r_name[0] = 0; r_name++;
		if      (!strcasecmp(c_name,"menu"))         r_type=8;
		else if (!strcasecmp(c_name,"smalltoolbar")) r_type=16;
		else if (!strcasecmp(c_name,"largetoolbar")) r_type=32;
		else if (!strcasecmp(c_name,"button"))       r_type=16;
		else if (!strcasecmp(c_name,"dnd"))          r_type=32;
		else if (!strcasecmp(c_name,"dialog"))       r_type=48;
		else { r_name--; r_name[0]='/'; g_free(c_name); return NULL; }
	}


	theme=gtk_icon_theme_get_default();
	buf=gtk_icon_theme_load_icon(theme,r_name,r_type,GTK_ICON_LOOKUP_USE_BUILTIN,NULL);
	g_free(c_name);
	if (!buf) return NULL;

	pic = gPicture::fromPixbuf(buf);
	g_object_unref(buf);

	return pic;
}

void gPicture::clear()
{
	//fprintf(stderr, "gPicture::clear: %p (%d %d) pixmap = %p pixbuf = %p\n", this, _width, _height, pixmap, pixbuf);

	_width = 0;
	_height = 0;
	_type = VOID;
	
#ifndef GTK3
	if (pixmap)
		g_object_unref(G_OBJECT(pixmap));
	if (mask)
		g_object_unref(G_OBJECT(mask));

	pixmap = NULL;
	mask = NULL;
#endif

	if (pixbuf)
		g_object_unref(G_OBJECT(pixbuf));
	if (surface)
		cairo_surface_destroy(surface);
	
	pixbuf = NULL;
	surface = NULL;
}

void gPicture::resize(int w, int h)
{
	if (_width <= 0 || _height <= 0)
	{
		clear();
		return;
	}

#ifndef GTK3
	if (_type == PIXMAP)
	{
		GdkPixmap *buf;
		GdkBitmap *save;
		GdkGC *gc;

		buf = create_pixmap(w, h);
		
		gc=gdk_gc_new(buf);
		gdk_draw_drawable(buf, gc, pixmap, 0, 0, 0, 0, w, h);
		g_object_unref(gc);
		
		g_object_unref(G_OBJECT(pixmap));
		pixmap = buf;
		
		if (_transparent)
		{
			save = mask;
			
			mask = gdk_pixmap_new(NULL, w, h, 1);
			
			gc=gdk_gc_new(mask);
			gdk_draw_drawable(mask, gc, save, 0, 0, 0, 0, w, h);
			g_object_unref(gc);
			
			g_object_unref(save);
		}
	}
	else
#endif
	if (_type == PIXBUF)
	{
		GdkPixbuf *buf;
		
		if (w > width() || h > height())
		{
			buf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
			if (w > width()) w = width();
			if (h > height()) h = height();
			gdk_pixbuf_copy_area(pixbuf, 0, 0, w, h, buf, 0, 0);
		}
		else
		{
			buf = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, w, h);
		}
		
		g_object_unref(G_OBJECT(pixbuf));
		pixbuf = buf;
	}
#ifdef GTK3
	else if (_type == SURFACE)
	{
		cairo_surface_t *buf = create_surface(w, h);
		cairo_t *cr = cairo_create(buf);
		cairo_set_source_surface(cr, surface, 0, 0);
		cairo_paint(cr);
		cairo_destroy(cr);
	}
#endif
	
	_width = w;
	_height = h;
	
	invalidate();
}


gPicture *gPicture::copy(int x, int y, int w, int h)
{
	gPicture *ret = NULL;
	
	if (_type == VOID || w <= 0 || h <= 0)
		return new gPicture();

#ifndef GTK3
	if (_type == PIXMAP)
	{
		GdkGC *gc;
		
		ret = new gPicture(_type, w, h, _transparent);
	
		gc=gdk_gc_new(ret->pixmap);
		gdk_draw_drawable(ret->pixmap, gc, pixmap, x, y, 0, 0, w, h);
		g_object_unref(gc);
		
		if (ret->mask)
		{
			gc=gdk_gc_new(ret->mask);
			gdk_draw_drawable(ret->mask, gc, mask, x, y, 0, 0, w, h);
			g_object_unref(gc);
		}
	}
	else
#endif
	if (_type == PIXBUF)
	{
		GdkPixbuf *buf;
		if (x == 0 && y == 0 && w == width() && h == height())
			buf = gdk_pixbuf_copy(pixbuf);
		else
		{
			buf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
			gdk_pixbuf_copy_area(pixbuf, x, y, w, h, buf, 0, 0);
		}
		
		ret = new gPicture(buf, _transparent);
	}
#ifdef GTK3
	else if (_type == SURFACE)
	{
		cairo_surface_t *buf = create_surface(w, h);
		cairo_t *cr = cairo_create(buf);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_rectangle(cr, 0, 0, w, h);
		cairo_fill(cr);
		cairo_destroy(cr);

		ret = new gPicture(buf);
	}
#endif
	
	return ret;
}

gPicture *gPicture::copy()
{
	return copy(0, 0, width(), height());
}

void gPicture::putPixel(int x, int y, gColor col)
{
	guchar *p;
	unsigned int nchannels;
	unsigned int rowstride;
	GdkPixbuf *image;

	if ( (x<0) || (x>width()) ) return;
	if ( (y<0) || (y>height()) ) return;

	image = getPixbuf();
	
	nchannels=gdk_pixbuf_get_n_channels(image);
	rowstride=gdk_pixbuf_get_rowstride(image);

	p = gdk_pixbuf_get_pixels(image) + (x * nchannels) + (y * rowstride);

	/*if (nchannels>0) p[0]=( (col>>16) & 0xFF);
	if (nchannels>1) p[1]=( (col>>8) & 0xFF );
	if (nchannels>2) p[2]=( col & 0xFF );
	//if (nchannels>3) p[3]=(col>>24);*/
	p[0]=((col>>16) & 0xFF);
	p[1]=((col>>8) & 0xFF);
	p[2]=(col & 0xFF);
	if (nchannels>3) p[3]=255 - (col >> 24);   

	invalidate();
}

gColor gPicture::getPixel(int x, int y)
{
	guchar *p;
	unsigned int nchannels;
	unsigned int rowstride;
	gColor ret=0;
	GdkPixbuf *image;

	if ( (x<0) || (x>width()) ) return 0;
	if ( (y<0) || (y>height()) ) return 0;

	image = getPixbuf();
	
	nchannels=gdk_pixbuf_get_n_channels(image);
	rowstride=gdk_pixbuf_get_rowstride(image);

	p = gdk_pixbuf_get_pixels(image) + (x * nchannels) + (y * rowstride);

	if (nchannels>3) ret += (((gColor)255-p[3]) << 24);	
	if (nchannels>0) ret += (((gColor)p[0]) << 16);
	if (nchannels>1) ret += (((gColor)p[1]) << 8);
	if (nchannels>2) ret += ((gColor)p[2]);
	

	return ret;
}

unsigned char *gPicture::data()
{
	GdkPixbuf *pixbuf = getPixbuf();
	if (!pixbuf)
		return NULL;
	else
		return gdk_pixbuf_get_pixels(pixbuf);
}

// void gPicture::replace(gColor src, gColor dst, bool noteq)
// {
// 	if (_type == VOID)
// 		return;
// 	
// 	gt_pixbuf_replace_color(getPixbuf(), src, dst, noteq);
// 	invalidate();
// }


gPicture* gPicture::flip(bool mirror)
{
	gPicture *ret;
	guint32 *src, *dst;
	int w, h;
	register guint32 *s, *d;
	register int x, y;
	int rowstride;

	getPixbuf();
	ret = copy();
	
	if (!isVoid())
	{
		src = (guint32 *)data();
		dst = (guint32 *)ret->data();
		w = width();
		h = height();
		
		rowstride = gdk_pixbuf_get_rowstride(getPixbuf()) / sizeof(guint32);
	
		if (!mirror)
		{
			for (y = 0; y < h; y++)
			{
				s = src + y * rowstride;
				d = dst + y * rowstride + w;
				for (x = 0; x < w; x++)
					*--d = *s++;
			}
		}
		else
		{
			s = src;
			d = dst + h * rowstride;
			for (y = 0; y < h; y++)
			{
				d -= rowstride;
				memcpy(d, s, w * sizeof(guint32));
				s += rowstride;
			}
		}
	}
	
	return ret;
}

/*
	This algorithm is inspired from the QT one
*/

static void rotate_image(double mat11, double mat12, double mat21, double mat22, double matdx, double matdy,
	uchar *dptr, int dbpl, int dWidth, int dHeight,
	uchar *sptr, int sbpl, int sWidth, int sHeight
	)
{
	int m11 = int(mat11 * 65536.0 + 1.0);
	int m12 = int(mat12 * 65536.0 + 1.0);
	int m21 = int(mat21 * 65536.0 + 1.0);
	int m22 = int(mat22 * 65536.0 + 1.0);
	
	uint trigx;
	uint trigy;
	uint maxws = sWidth << 16;
	uint maxhs = sHeight << 16;

	int m21ydx = int(matdx * 65536.0 + 1.0);
	int m22ydy = int(matdy * 65536.0 + 1.0);

	for (int y = 0; y < dHeight; y++ ) 
	{
		trigx = m21ydx;
		trigy = m22ydy;
		uchar *maxp = dptr + dbpl;
		while (dptr < maxp) 
		{
			if (trigx < maxws && trigy < maxhs)
				*((uint*)dptr) = *((uint *)(sptr + sbpl * (trigy >> 16) + ((trigx >> 16) << 2)));
			trigx += m11;
			trigy += m12;
			dptr += 4;
		}
		m21ydx += m21;
		m22ydy += m22;
	}
}

gPicture* gPicture::rotate(double angle)
{
	double cosa = cos(-angle);
	double sina = sin(-angle);
	
	if (angle == 0.0 || (cosa == 1.0 && sina == 0.0) || (width() <= 1 && height() <= 1))
		return copy();
	
	double dx, dy, cx, cy;
	int nw, nh;
	double minx, miny, maxx, maxy;
	int i, px[3], py[3];
	
	nw = 0;
	nh = 0;
	minx = miny = maxx = maxy = 0;
	
	px[0] = (int)(width() * cosa + height() * (-sina) + 0.5);
	py[0] = (int)(width() * sina + height() * cosa + 0.5);
	
	px[1] = (int)(width() * cosa + 0.5);
	py[1] = (int)(width() * sina + 0.5);
	
	px[2] = (int)(height() * (-sina) + 0.5);
	py[2] = (int)(height() * cosa + 0.5);
	
	for (i = 0; i < 3; i++)
	{
		if (px[i] > maxx) maxx = px[i];
		if (px[i] < minx) minx = px[i];
		if (py[i] > maxy) maxy = py[i];
		if (py[i] < miny) miny = py[i];
	}	
	
	nw = (int)(maxx - minx + 0.5);
	nh = (int)(maxy - miny + 0.5);
	
	cx = nw / 2.0 * cosa + nh / 2.0 * sina;
	cy = nw / 2.0 * -sina + nh / 2.0 * cosa;
	
	dx = width() / 2.0 - cx;
	dy = height() / 2.0 - cy;
	
	GdkPixbuf *src = getPixbuf();
	gPicture *npic = new gPicture(PIXBUF, nw, nh, isTransparent());
	GdkPixbuf *dst = npic->getPixbuf();
	
	npic->fill(isTransparent() ? -1 : 0);
	
	rotate_image(cosa, -sina, sina, cosa, dx, dy,
		gdk_pixbuf_get_pixels(dst), nw * 4, nw, nh,  
		gdk_pixbuf_get_pixels(src), width() * 4, width(), height());
	
	return npic;
}

gPicture *gPicture::stretch(int w, int h, bool smooth)
{
	gPicture *ret;
	GdkPixbuf *image;
	int ws, hs;
	
	if (w <= 0 && h <= 0)
		return new gPicture();
	
	if (w < 0)
		w = width() * h / height();
	else if (h < 0)
		h = height() * w / width();
	
	if (w <= 0 || h <= 0)
		return new gPicture();
	
	ret = copy();
	if (ret->isVoid())
		return ret;
		
	image = ret->getPixbuf();
	
	if (smooth)
	{
		ws = w;
		hs = h;
		if (ws < (width() / 4))
			ws = w * 4;
		if (hs < (height() / 4))
			hs = h * 4;
		if (ws != w || hs != h)
		{
			ret->pixbuf = gdk_pixbuf_scale_simple(image, ws, hs, GDK_INTERP_NEAREST);
			g_object_unref(G_OBJECT(image));
			image = ret->pixbuf;
		}
		ret->pixbuf = gdk_pixbuf_scale_simple(image, w, h, GDK_INTERP_BILINEAR);
	}
	else
		ret->pixbuf = gdk_pixbuf_scale_simple(image, w, h, GDK_INTERP_NEAREST);

	g_object_unref(G_OBJECT(image));
	
	ret->_width = w;
	ret->_height = h;

	ret->invalidate();

	return ret;
}


void gPicture::draw(gPicture *pic, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	if (isVoid() || pic->isVoid())
		return;

	GT_NORMALIZE(x, y, w, h, sx, sy, sw, sh, pic->width(), pic->height());

	if (x >= width() || y >= height())
		return;
	

#ifndef GTK3
	if (_type == PIXMAP)
	{
		GdkPixmap *dst = getPixmap();
		
		if (pic->type() == gPicture::PIXMAP && !pic->isTransparent() && w == sw && h == sh)
		{
			GdkGC *gc = gdk_gc_new(GDK_DRAWABLE(dst));
			GdkPixmap *src = pic->getPixmap();
			gdk_draw_drawable(GDK_DRAWABLE(dst), gc, src, sx, sy, x, y, sw, sh);
			g_object_unref(G_OBJECT(gc));
		}
		else
		{
			bool del = false;
			
			if (w != sw || h != sh)
			{
				gPicture *pic2;
				pic2 = pic->copy(sx, sy, sw, sh);
				pic = pic2->stretch(w, h, true);
				delete pic2;
				del = true;
				sx = 0; sy = 0; sw = w; sh = h;
			}
			
			gdk_draw_pixbuf(GDK_DRAWABLE(dst), NULL, pic->getPixbuf(), sx, sy, x, y, sw, sh, GDK_RGB_DITHER_MAX, 0, 0);
			
			if (del)
				delete pic;
		}
	}
	else
#endif
	if (_type == PIXBUF)
	{
		GdkPixbuf *dst = getPixbuf();
		GdkPixbuf *src = pic->getPixbuf();
		double scale_x, scale_y, offset_x, offset_y;
		
		scale_x = (double)w / sw;
		scale_y = (double)h / sh;
		offset_x = x - sx * scale_x;
		offset_y = y - sy * scale_y;
		
		if (x < 0)
			x = 0;
		
		if (y < 0)
			y = 0;
		
		if ((x + w) > width())
			w = width() - x;
		
		if ((y + h) > height())
			h = height() - y;
		
		gdk_pixbuf_composite(src, dst, x, y, w, h, offset_x, offset_y, scale_x, scale_y, GDK_INTERP_BILINEAR, 255);
	}
	
	invalidate();
}


/*****************************************************************

gPictureCache

******************************************************************/

GHashTable *gPictureCache::cache = 0;

static void destroy_key(char *key)
{
	g_free(key);
}

static void destroy_value(gPicture *pic)
{
	//fprintf(stderr, "gPictureCache: destroy_value %p\n", pixmap);
	pic->unref();
}

void gPictureCache::init()
{
	cache = g_hash_table_new_full((GHashFunc)g_str_hash, (GEqualFunc)g_str_equal, (GDestroyNotify)destroy_key, (GDestroyNotify)destroy_value);
}

void gPictureCache::exit()
{
	g_hash_table_destroy(cache);
}

void gPictureCache::put(const char *key, gPicture *pic)
{
	if (!key || !*key) return;
	
	//fprintf(stderr, "gPictureCache: put %p\n", pixmap);
	pic->ref();
	g_hash_table_replace(cache, (gpointer)g_strdup(key), (gpointer)pic);
}

gPicture *gPictureCache::get(const char *key)
{
	if (!key || !*key) return 0;
	
	return (gPicture *)g_hash_table_lookup(cache, (gconstpointer)key);
}

void gPictureCache::flush()
{
	exit();
	init();
}

void gPicture::makeGray()
{
	if (_type == VOID)
		return;
	
	gt_pixbuf_make_gray(getPixbuf());
	invalidate();
}

void gPicture::makeTransparent(gColor color)
{
	if (_type == VOID)
		return;
	
	gt_pixbuf_make_alpha(getPixbuf(), color);
	invalidate();
}

GdkPixbuf *gPicture::getIconPixbuf()
{
	GdkPixbuf *icon = getPixbuf();
	
	if ((_width & 7) || (_height & 7))
	{
		icon = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, (_width + 7) & ~7, (_height + 7) & ~7);
		gdk_pixbuf_fill(icon, 0);
		gdk_pixbuf_copy_area(getPixbuf(), 0, 0, _width, _height, icon, 0, 0);
	}
	
	return icon;
}

