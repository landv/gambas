/***************************************************************************

  gpicture.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  Gtkmae "GTK+ made easy" classes

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

#include <math.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gpicture.h"

typedef
	unsigned char uchar;

/*****************************************************************

alphaCache

******************************************************************/

class alphaCache : gShare
{
public:
	int vx;
	int vy;
	unsigned char *buffer;
//"Public"
	alphaCache(GdkPixbuf *buf);
	alphaCache(int x, int y);
	~alphaCache();
	alphaCache *copy();
	
	bool active();
	void fillAlpha(GdkPixbuf *buf);
	//void copyArea(alphaCache *orig, int src_x, int src_y);
};


alphaCache::alphaCache(int x, int y) : gShare()
{
	if (x < 1 || y < 1) { buffer=NULL; return; }
	
	vx = x;
	vy = y;

	buffer=(unsigned char*)g_malloc(sizeof(*buffer)*x*y);
	if (!buffer) return;

  memset(buffer, 255, vx * vy * sizeof(*buffer));
}

alphaCache::alphaCache(GdkPixbuf *buf) : gShare()
{
	long p_chan;
	long p_row;
	guchar *p_pix;
	unsigned char *ptr;
	int bx,by;

	if (!buf) { buffer=NULL; return; }
	if (!gdk_pixbuf_get_has_alpha(buf)) { buffer=NULL; return; }

	vx=gdk_pixbuf_get_width (buf);
	vy=gdk_pixbuf_get_height (buf);

	p_chan = gdk_pixbuf_get_n_channels (buf);

	if (p_chan!=4) { buffer=NULL; return; }

	buffer=(unsigned char*)g_malloc(sizeof(unsigned char)*vx*vy);

	if (!buffer) return;

	p_row = gdk_pixbuf_get_rowstride (buf);

	ptr=buffer;
	for (bx=0;bx<vx;bx++)
			for (by=0; by<vy;by++)
			{
				p_pix=gdk_pixbuf_get_pixels(buf) + (by*p_row) + (bx*p_chan);
				ptr[0]= p_pix[3]; ptr++;
			}


}

alphaCache::~alphaCache()
{
	if (buffer) g_free(buffer);
}

/*void alphaCache::copyArea(alphaCache *orig,long src_x,long src_y)
{
	long bx,by;
	long dx=src_x,dy=src_y;

	if (!buffer) return;
	if (!orig->buffer) return;

	for (bx=0;bx<vx;bx++)
	{
		for (by=0;by<vy;by++)
		{
			buffer[ (bx*vx) + by]=orig->buffer[ (dx*orig->vx) + dy   ];
			dy++;
		}
		dy=src_y;
		dx++;
	}

}*/

alphaCache *alphaCache::copy()
{
  alphaCache *ret = new alphaCache(vx, vy);
  memcpy(ret->buffer, buffer, vx * vy * sizeof(*buffer));
  return ret;
}

bool alphaCache::active()
{
	return (bool)buffer;
}

void alphaCache::fillAlpha(GdkPixbuf *buf)
{
	long p_chan;
	long p_row;
	long bx,by;
	guchar *p_pix;
	unsigned char *ptr;

	if (!buf) return;
	if (!buffer) return;

	if (vx!=gdk_pixbuf_get_width (buf)) return;
	if (vy!=gdk_pixbuf_get_height (buf)) return;
	p_chan=gdk_pixbuf_get_n_channels(buf);
	if (p_chan!=4) return;

	p_row = gdk_pixbuf_get_rowstride (buf);
	ptr=buffer;

	for (bx=0;bx<vx;bx++)
			for (by=0; by<vy;by++)
			{
				p_pix=gdk_pixbuf_get_pixels(buf) + (by*p_row) + (bx*p_chan);
				p_pix=gdk_pixbuf_get_pixels(buf) + (by*p_row) + (bx*p_chan);
				p_pix[3]=ptr[0]; ptr++;
			}
}

/*****************************************************************

gPicture

******************************************************************/

static bool pixbufFromMemory(GdkPixbuf **img,char *addr,unsigned int len)
{
	GdkPixbufLoader* loader;

  *img = 0;

	loader=gdk_pixbuf_loader_new();
	if (!gdk_pixbuf_loader_write(loader,(guchar*)addr,(gsize)len,NULL)){
		 g_object_unref(G_OBJECT(loader));
		 return false;
	}
	gdk_pixbuf_loader_close(loader,NULL);
	(*img)=gdk_pixbuf_loader_get_pixbuf(loader);
	g_object_ref(G_OBJECT(*img));
	g_object_unref(G_OBJECT(loader));

	if (gdk_pixbuf_get_n_channels(*img) == 3)
	{
		// BM: convert to 4 bytes per pixels
		GdkPixbuf *aimg;
		aimg = gdk_pixbuf_add_alpha(*img, FALSE, 0, 0, 0);
		g_object_unref(G_OBJECT(*img));
  	g_object_ref(G_OBJECT(aimg));
		*img = aimg;
	}

	return true;
}

static void render_pixbuf(gPicture *pic, GdkPixbuf *img)
{
	GdkColor col={0,0xFF00,0xFF00,0xFF00};
	GdkColormap *map=gdk_colormap_get_system();
	GdkGC *gc;
	gint b_w,b_h;
	gint depth;

	pic->_transparent=false;

	if (pic->pic) 
	{
	 g_object_unref(pic->pic);
	 delete pic->cache;
	 pic->cache = 0;
	 pic->pic = 0;
  }
  
	if (gdk_pixbuf_get_has_alpha(img)) {
		pic->_transparent=true;
		pic->cache=new alphaCache(img);
		if (!pic->cache->active()) { delete pic->cache; pic->cache=NULL; }
	}

	b_w=gdk_pixbuf_get_width(img);
	b_h=gdk_pixbuf_get_height(img);

	depth=gdk_screen_get_system_visual(gdk_screen_get_default())->depth;
	pic->pic=gdk_pixmap_new(NULL,b_w,b_h,depth);
	gc=gdk_gc_new(pic->pic);
	gdk_colormap_alloc_color(map,&col,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&col);
	gdk_draw_rectangle(pic->pic,gc,TRUE,0,0,b_w-1,b_h-1);
	g_object_unref(G_OBJECT(gc));
	gdk_draw_pixbuf(pic->pic,NULL,img,0,0,0,0,b_w,b_h,GDK_RGB_DITHER_MAX,0,0);
}

void gPicture::initialize()
{
  pic = 0;
  img = 0;
  cache = 0;
  _transparent = false;
  _type = VOID;
  _width = 0;
  _height = 0;
  tag = NULL;
}

gPicture::gPicture() : gShare()
{
  initialize();
}

static GdkPixmap *create_pixmap(int w, int h)
{
	GdkScreen *scr;
	gint depth;
	GdkPixmap *pic;

  scr = gdk_screen_get_default();
  depth = (gdk_screen_get_system_visual(scr))->depth;

  pic = gdk_pixmap_new(NULL, w, h, depth);
  gdk_drawable_set_colormap(GDK_DRAWABLE(pic), gdk_colormap_get_system());
  
  return pic;
}


gPicture::gPicture(gPictureType type, long w, long h, bool trans) : gShare()
{
  initialize();
  if (!type)
    return;

	_transparent=trans;

	if (w<=0)
	{
		if (h>0) w=h;
		else     w=0;
	}
	if (h<=0)
	{
		if (w>0) h=w;
		else     h=0;
	}

	if ( (!w) && (!h) ) return;

  _type = type;
  _width = w;
  _height = h;

  if (_type == SERVER)
  {
    pic = create_pixmap(w, h);
  }
  else if (_type == MEMORY)
  {
  	img = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
  }
}


// The gPicture takes the GdkPixbuf object. Do not unreference it after.

gPicture::gPicture(GdkPixbuf *image) : gShare()
{
  initialize();
  if (!image)
    return;
    
  _type = MEMORY;
  _width = gdk_pixbuf_get_width(image);
  _height = gdk_pixbuf_get_height(image);
  img = image;
}

// The gPicture takes the GdkPixmap object. Do not unreference it after.

gPicture::gPicture(GdkPixmap *pixmap) : gShare()
{
  initialize();
  if (!pixmap)
    return;
    
  _type = SERVER;
  gdk_drawable_get_size((GdkDrawable *)pixmap, &_width, &_height);
  pic = pixmap;
}

gPicture::~gPicture()
{
  clear();
}

void gPicture::invalidate()
{
	if (_type == MEMORY && pic) 
	{
    g_object_unref(G_OBJECT(pic));
    pic = 0;
  }
  else if (_type == SERVER && img)
  {
    g_object_unref(G_OBJECT(img));
    img = 0;
  }
}

GdkPixbuf *gPicture::getPixbuf()
{
  if (_type == VOID)
    return NULL;
    
  if (!img && pic)
  {
    img = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, width(), height());
    gdk_pixbuf_get_from_drawable(img, pic, NULL, 0, 0, 0, 0, width(), height());
    if (cache && _transparent) 
      cache->fillAlpha(img);
  }
  
  _type = MEMORY;
  return img;
}

GdkPixmap *gPicture::getPixmap()
{
  if (_type == VOID)
    return NULL;
  
  if (!pic && img)
    render_pixbuf(this, img);
  
  _type = SERVER;
  return pic;
}

#if 0
GdkPixbuf* gPicture::getPixbuf()
{
	GdkPixbuf *buf=NULL;
	GdkPixbuf *fmask=NULL;
	GdkColormap* cmap=gdk_colormap_get_system();

	if (!pic) return NULL;

	buf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,_transparent,8,width(),height());
	gdk_pixbuf_get_from_drawable(buf,pic,NULL,0,0,0,0,width(),height());
	if (cache && _transparent) cache->fillAlpha(buf);

	return buf;
}
#endif

gPicture *gPicture::fromMemory(char *addr, unsigned int len)
{
	GdkPixbuf *img;

	pixbufFromMemory(&img, addr, len);
	//fprintf(stderr, "gPicture::fromMemory: pixbuf = %p\n", img);
	return new gPicture(img);
}


int gPicture::depth()
{
	int depth=0;

	if (pic) 
    depth = gdk_drawable_get_depth(GDK_DRAWABLE(pic));
  else if (img)
    depth = 32;

	return depth;
}

void gPicture::setTransparent(bool vl)
{
	_transparent=vl;
}

void gPicture::fill(gColor col)
{
	//GdkColormap* map;
	GdkColor color;
	GdkGC*   gc;
	gint myw,myh;

  if (_type == SERVER)
  {
  	fill_gdk_color(&color, col);
		gdk_drawable_get_size(GDK_DRAWABLE(pic),&myw,&myh);
		gc=gdk_gc_new(pic);
		gdk_gc_set_foreground(gc,&color);
		gdk_gc_set_background(gc,&color);
		gdk_draw_rectangle(pic,gc,true,0,0,myw,myh);
		g_object_unref(gc);    
  }
  else if (_type == MEMORY)
  {
  	int r, g, b, a;
  	char c[4];
  	
  	gt_color_to_rgba(col, &r, &g, &b, &a);
  	
  	c[0] = a ^ 0xFF;
  	c[1] = b;
  	c[2] = g;
  	c[3] = r;
  	
  	gdk_pixbuf_fill(img, *((guint32 *)c));
  }
  
  invalidate();
}


// returns -> 0, OK / -1, Bad format / -2 invalid path

int gPicture::save(char *path)
{
	bool ok=false;
	long b;
	char *type;
	char *buf=NULL;
	GSList *formats = gdk_pixbuf_get_formats();
	GSList *iter=formats;
	GdkPixbuf *image = getPixbuf();

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
		if (!strcasecmp("jpg",buf))
			type="jpeg";
		else
			return -1;
	}

	b=gdk_pixbuf_save (image, path,type, NULL, NULL);


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
gPicture* gPicture::fromNamedIcon(char* name)
{
	GtkIconTheme* theme;
	GdkPixbuf *buf;
	gPicture *pic=NULL;
	long r_type=32;
	char *r_name=NULL;

	r_name=strchr(name,'/');

	if (!r_name) { r_name=name; }
	else
	{
		r_name[0]=0; r_name++;
		if      (!strcasecmp(name,"menu"))         r_type=8;
		else if (!strcasecmp(name,"smalltoolbar")) r_type=16;
		else if (!strcasecmp(name,"largetoolbar")) r_type=32;
		else if (!strcasecmp(name,"button"))       r_type=16;
		else if (!strcasecmp(name,"dnd"))          r_type=32;
		else if (!strcasecmp(name,"dialog"))       r_type=48;
		else { r_name--; r_name[0]='/'; return NULL; }
	}


	theme=gtk_icon_theme_get_default();
	buf=gtk_icon_theme_load_icon(theme,r_name,r_type,GTK_ICON_LOOKUP_USE_BUILTIN,NULL);
	if (!buf) return NULL;

	pic=gPicture::fromPixbuf(buf);
	g_object_unref(buf);

	return pic;
}

void gPicture::clear()
{
  //fprintf(stderr, "gPicture::clear: %p (%d %d) pic = %p img = %p\n", this, _width, _height, pic, img);
  _width = 0;
  _height = 0;
  _type = VOID;
  if (pic)
  {
  	g_object_unref(G_OBJECT(pic));
  	pic = NULL;
  }
	if (img) 
	{
		g_object_unref(G_OBJECT(img));
		img = NULL;
	}
	
	delete cache;
  pic = 0;
  img = 0;
  cache = 0;
}

void gPicture::resize(int w, int h)
{
  if (_width <= 0 || _height <= 0)
  {
    clear();
    return;
  }

  if (_type == SERVER)
  {
  	GdkPixmap *buf;
  	GdkGC *gc;

    buf = create_pixmap(w, h);
		
		gc=gdk_gc_new(buf);
		gdk_draw_drawable(buf, gc, pic, 0, 0, 0, 0, -1, -1);
		g_object_unref(gc);
		
		g_object_unref(G_OBJECT(pic));
		pic = buf;
  }
  else if (_type == MEMORY)
  {
  	GdkPixbuf *buf;
  	
		if (w > width() || h > height())
		{
			buf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
			if (w > width()) w = width();
			if (h > height()) h = height();
			gdk_pixbuf_copy_area(img, 0, 0, w, h, buf, 0, 0);
		}
		else
		{
			buf = gdk_pixbuf_new_subpixbuf(img, 0, 0, w, h);
		}
		
		g_object_unref(G_OBJECT(img));
		img = buf;
  }
  
  _width = w;
  _height = h;
  
  invalidate();
}


gPicture *gPicture::copy()
{
  gPicture *ret = new gPicture();
  
  ret->_type = _type;
  ret->_transparent = _transparent;
  ret->_width = _width;
  ret->_height = _height;
  
  if (pic)
  {
    ret->pic = pic; 
    g_object_ref(G_OBJECT(pic));
    if (cache)
    	ret->cache = cache->copy();
  }
  
  if (img)
  {
    ret->img = img; 
    g_object_ref(G_OBJECT(img));
  }
  
  return ret;
}

gPicture *gPicture::copy(int x, int y, int w, int h)
{
  gPicture *ret;
  
  if (w <= 0 || h <= 0)
    return new gPicture();

  ret = new gPicture(_type, w, h, _transparent);
  
  if (_type == SERVER)
  {
    GdkGC *gc;
    
		gc=gdk_gc_new(ret->pic);
		gdk_draw_drawable(ret->pic, gc, pic, x, y, 0, 0, w, h);
		g_object_unref(gc);
  }
  else if (_type == MEMORY)
  {
		gdk_pixbuf_copy_area(img, x, y, w, h, ret->img, 0, 0);
  }
  
  return ret;
}

void gPicture::putPixel(int x, int y, gColor col)
{
	guchar *p;
	unsigned long nchannels;
	unsigned long rowstride;
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
	unsigned long nchannels;
	unsigned long rowstride;
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

void gPicture::replace(gColor src, gColor dst)
{
	guchar *p;
	unsigned long nchannels;
	unsigned long rowstride;
	int x, y, w, h;
	bool ok;
	GdkPixbuf *image;

	h=width();
	w=height();
  image = getPixbuf();
	nchannels=gdk_pixbuf_get_n_channels(image);
	rowstride=gdk_pixbuf_get_rowstride(image);

	for (x=0;x<w;x++)
	{
		for (y=0;y<h;y++)
		{
			p = gdk_pixbuf_get_pixels(image) + (x * nchannels) + (y * rowstride);
			ok=true;
			if (nchannels>0) if (p[0]!=( (src>>16) & 0xFF ) ) ok=false;
			if (nchannels>1) if (p[1]!=( (src>>8) & 0xFF ) ) ok=false;
			if (nchannels>2) if (p[2]!=( src & 0xFF ) ) ok=false;
			if (ok)
			{
				if (nchannels>0) p[0]= (dst>>16) & 0xFF;
				if (nchannels>1) p[1]= (dst>>8) & 0xFF;
				if (nchannels>2) p[2]= dst & 0xFF;
			}
		}
	}
	
	invalidate();
}


/*void gPicture::fromMemory(char *addr,unsigned int len)
{
	GdkPixbuf *img;

	if (!pixbufFromMemory(&img,addr,len)) return;

	if (image) g_object_unref(G_OBJECT(image));
	image=img;
}*/


gPicture* gPicture::flip(bool mirror)
{
	gPicture *ret;
	GdkPixbuf *old;

	ret = copy();
	
	if (!isVoid())
	{
    ret->getPixbuf();
    old = ret->img;
    ret->img = gdk_pixbuf_flip(old, !mirror);
    g_object_unref(G_OBJECT(old));
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
	gPicture *npic = new gPicture(MEMORY, nw, nh, transparent());
	GdkPixbuf *dst = npic->getPixbuf();
	
	npic->fill(-1);
	
	rotate_image(cosa, -sina, sina, cosa, dx, dy,
		gdk_pixbuf_get_pixels(dst), nw * 4, nw, nh,  
		gdk_pixbuf_get_pixels(src), width() * 4, width(), height());
	
	return npic;
}

gPicture *gPicture::stretch(int w, int h, bool smooth)
{
  gPicture *ret;
  GdkPixbuf *image;
  
  if (w <= 0 || h <= 0)
    return new gPicture();
  
  ret = copy();
  if (ret->isVoid())
    return ret;
    
  image = ret->getPixbuf();
  
  if (smooth)
    ret->img = gdk_pixbuf_scale_simple(image, w, h, GDK_INTERP_HYPER);
  else
    ret->img = gdk_pixbuf_scale_simple(image, w, h, GDK_INTERP_NEAREST);

  g_object_unref(G_OBJECT(image));
  
  ret->_width = w;
  ret->_height = h;

  ret->invalidate();

	return ret;
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
	//fprintf(stderr, "gPictureCache: destroy_value %p\n", pic);
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

void gPictureCache::put(char *key, gPicture *pic)
{
	if (!key || !*key) return;
  
	//fprintf(stderr, "gPictureCache: put %p\n", pic);
	pic->ref();
	g_hash_table_replace(cache, (gpointer)g_strdup(key), (gpointer)pic);
}

gPicture *gPictureCache::get(char *key)
{
	if (!key || !*key) return 0;
	
	return (gPicture *)g_hash_table_lookup(cache, (gconstpointer)key);
}

void gPictureCache::flush()
{
  exit();
  init();
}

#if 0
GdkPixbuf **buf_img_cache=NULL;
char **buf_txt_cache=NULL;
long buf_img_count=0;

void gPictureCache::save(char *key, gPicture *img)
{
	long b;
	long pos=-1;

	if (!key || !*key) return;

	for (b=0;b<buf_img_count;b++)
	{
		if (!strcmp(buf_txt_cache[b],key)) { pos=b; break; }
	}

	if (pos==-1)
	{

	  if (!buf_img_count)
	  {
	    buf_img_cache=(GdkPixbuf**)g_malloc(sizeof(GdkPixbuf*)*(buf_img_count+1));
		buf_txt_cache=(char**)g_malloc(sizeof(char*)*(buf_img_count+1));
	  }
	  else
	  {
	    buf_img_cache=(GdkPixbuf**)g_realloc((void*)buf_img_cache,sizeof(GdkPixbuf*)*(buf_img_count+1));
	    buf_txt_cache=(char**)g_realloc((void*)buf_txt_cache,sizeof(char*)*(buf_img_count+1));
	  }
	  pos=buf_img_count;
	  buf_img_cache[pos]=NULL;
	  buf_txt_cache[pos]=(char*)g_malloc(sizeof(char)*(strlen(key)+1));
	  strcpy(buf_txt_cache[pos],key);
	}

	buf_img_count++;

	if (buf_img_cache[pos]) {
		g_object_unref(G_OBJECT(buf_img_cache[pos]));
		buf_img_cache[pos]=NULL;
	}

	if (!img) return;
	if (!img->image) return;
	buf_img_cache[pos]=gdk_pixbuf_new_subpixbuf(img->image,0,0,img->width(),img->height());


}

gPicture* gPictureCache::load(char *key)
{
	long b;
	long w,h;
	long pos=-1;
	gPicture *ret;

	for (b=0;b<buf_img_count;b++)
	{
		if (!strcmp(buf_txt_cache[b],key)) { pos=b; break; }
	}

	if (pos==-1) return NULL;

	ret=new gPicture(0,0);
	if (buf_img_cache[pos])
	{
		w=gdk_pixbuf_get_width(buf_img_cache[pos]);
		h=gdk_pixbuf_get_height(buf_img_cache[pos]);
		ret->image=gdk_pixbuf_new_subpixbuf(buf_img_cache[pos],0,0,w,h);

	}

	return ret;
}

void gPictureCache::flush()
{
	long b;

	if (!buf_img_count) return;

	for (b=0; b<buf_img_count;b++)
	{
		g_free( buf_txt_cache[b] );
		if (buf_img_cache[b]) g_object_unref(G_OBJECT(buf_img_cache[b]));
	}

	g_free((void*)buf_txt_cache);
	g_free((void*)buf_img_cache);
	buf_img_count=0;


}
#endif
