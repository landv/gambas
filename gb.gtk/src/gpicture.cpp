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
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "widgets.h"
#include "widgets_private.h"

#include <stdio.h>
#include <math.h>



alphaCache::alphaCache(long x,long y)
{
	long bx,by;
	unsigned char *ptr;

	count=1;
	if (x<1) { buffer=NULL; return; }
	if (y<1) { buffer=NULL; return; }
	vx=x;
	vy=y;

	buffer=(unsigned char*)g_malloc(sizeof(unsigned char)*x*y);
	if (!buffer) return;

	ptr=buffer;
	for (bx=0;bx<vx;bx++)
		for (by=0; by<vy;by++)
			{ ptr[0]= 255; ptr++; }


}

alphaCache::alphaCache(GdkPixbuf *buf)
{
	long p_chan;
	long p_row;
	guchar *p_pix;
	unsigned char *ptr;
	long bx,by;

	count=1;
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

void alphaCache::copyArea(alphaCache *orig,long src_x,long src_y)
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

void alphaCache::ref()
{
	count++;
}

void alphaCache::unref()
{
	count--;
	if (count<1) delete this;
}

int pixbufFromMemory(GdkPixbuf **img,char *addr,unsigned int len)
{
	GdkPixbufLoader* loader;

	loader=gdk_pixbuf_loader_new();
	if (!gdk_pixbuf_loader_write(loader,(guchar*)addr,(gsize)len,NULL)){
		 g_object_unref(G_OBJECT(loader));
		 return 0;
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

	return 1;
}

void render_pixbuf(gPicture *pic,GdkPixbuf *img)
{
	GdkColor col={0,0xFF00,0xFF00,0xFF00};
	GdkColormap *map=gdk_colormap_get_system();
	GdkGC *gc;
	gint b_w,b_h;
	gint depth;

	pic->_transparent=false;

	if (pic->pic) g_object_unref(pic->pic);
	if (pic->cache) { pic->cache->unref(); pic->cache=NULL; }
	pic->pic=NULL;

	if (gdk_pixbuf_get_has_alpha(img)) {
		pic->_transparent=true;
		pic->cache=new alphaCache(img);
		if (!pic->cache->active()) { pic->cache->unref(); pic->cache=NULL; }
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
/*****************************************************************

gPicture

******************************************************************/
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


void gPicture::fromMemory(char *addr,unsigned int len)
{
	GdkPixbuf *img;

	if (!pixbufFromMemory(&img,addr,len)) return;

	render_pixbuf(this,img);
	g_object_unref(G_OBJECT(img));
}

gPicture* gPicture::fromPixbuf(GdkPixbuf *buf)
{
	gPicture *pic;

	if (!buf) return new gPicture(0,0,false);

	pic=new gPicture(0,0,false);
	render_pixbuf(pic,buf);

	return pic;
}

gPicture::gPicture(long w,long h,bool trans)
{
	GdkScreen*  scr;
	gint depth;

	refcount=1;
	pic=NULL;
	cache=NULL;
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

	scr=gdk_screen_get_default();
	depth=(gdk_screen_get_system_visual(scr))->depth;

	pic=gdk_pixmap_new (NULL,w,h,depth);
	gdk_drawable_set_colormap(GDK_DRAWABLE(pic),gdk_colormap_get_system());
}

gPicture::~gPicture()
{
	if (pic) g_object_unref(G_OBJECT(pic));
	if (cache) cache->unref();
}

void gPicture::ref()
{
	refcount++;
}

void gPicture::unref()
{
	refcount--;
	if (!refcount) delete this;
}

long gPicture::width()
{
	gint myw=0;

	if (pic) gdk_drawable_get_size(GDK_DRAWABLE(pic),&myw,NULL);
	return myw;
}

long gPicture::height()
{
	gint myh=0;

	if (pic) gdk_drawable_get_size(GDK_DRAWABLE(pic),NULL,&myh);
	return myh;
}

long gPicture::depth()
{
	gint depth=0;

	if (pic) depth=gdk_drawable_get_depth(GDK_DRAWABLE(pic));
	return depth;
}

bool gPicture::transparent()
{
	return _transparent;
}

void gPicture::setTransparent(bool vl)
{
	_transparent=vl;
}

void gPicture::Fill(long col)
{
	GdkColormap* map;
	GdkColor color;
	GdkGC*   gc;
	gint myw,myh;

	if (!pic) return;
	map=gdk_colormap_get_system();

	color.red=0xFF + ((col & 0xFF0000)>>8);
	color.green=0xFF + (col & 0x00FF00);
	color.blue=0xFF + ((col & 0x0000FF)<<8);

	if(gdk_colormap_alloc_color(map,&color,false,true))
	{
		gdk_drawable_get_size(GDK_DRAWABLE(pic),&myw,&myh);
		gc=gdk_gc_new(pic);
		gdk_gc_set_foreground(gc,&color);
		gdk_gc_set_background(gc,&color);
		gdk_draw_rectangle(pic,gc,true,0,0,myw,myh);
		g_free(G_OBJECT(gc));
	}
}

gImage* gPicture::getImage()
{
	gImage *img;

	img=new gImage(0,0);

	if (pic) img->image=getPixbuf();

	return img;
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
int gPicture::save(char *path)
{
	gImage *img=getImage();
	int ret;

	ret=img->save(path);
	delete img;
	return ret;
}



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

void  gPicture::resize(int width,int height)
{
	GdkPixbuf *buf;
	GdkPixbuf *ret;

	if (width<0) width=height;
	if (height<0) height=width;
	if ( (width<=0) || (height<=0) ) return;


	buf=getPixbuf();
	if (!buf) return;

	ret=gdk_pixbuf_scale_simple(buf,width,height,GDK_INTERP_BILINEAR);
	render_pixbuf(this,ret);

	g_object_unref(G_OBJECT(ret));
	g_object_unref(G_OBJECT(buf));
}

gPicture* gPicture::copy(long x,long y,long w,long h)
{
	GdkGC *gc;
	gPicture *ret=NULL;
	int src_x=x,src_y=y;
	int dw=w,dh=h;
	int dest_x=0,dest_y=0;


	if (src_x<0) { dest_x-=src_x;  src_x=0; }
	if (src_y<0) { dest_y-=src_y;  src_y=0; }


	if ( (dw-src_x) > width() ) dw=width()-src_x;
	if ( (dh-src_y) > height() ) dh=height()-src_y;

	if (pic && (dw>0) && (dh>0) )
	{
		ret=new gPicture(dw,dh,_transparent);
		gc=gdk_gc_new(pic);
		gdk_draw_drawable(ret->pic,gc,pic,src_x,src_y,dest_x,dest_y,dw,dh);
		g_object_unref(gc);

		if (_transparent && cache)
		{
			ret->cache=new alphaCache(dw+dest_x,dh+dest_y);
			//ret->cache->copyArea(cache,src_x,src_y);
		}
	}

	return ret;
}

/*****************************************************************

gImage

******************************************************************/

gImage::gImage(long w,long h)
{
	image=NULL;

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

	image=gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,w,h);
}

gImage::~gImage()
{
	if (image) g_object_unref(G_OBJECT(image));
}

void gImage::putPixel(long x,long y,long col)
{
	guchar *p;
	unsigned long nchannels;
	unsigned long rowstride;
	unsigned long b;
	long ret=0;

	if (!image) return;
	if ( (x<0) || (x>width()) ) return;
	if ( (y<0) || (y>height()) ) return;

	nchannels=gdk_pixbuf_get_n_channels(image);
	rowstride=gdk_pixbuf_get_rowstride(image);


	p = gdk_pixbuf_get_pixels(image) + (x * nchannels) + (y * rowstride);

	if (nchannels>0) p[0]=( (col>>16) & 0xFF);
	if (nchannels>1) p[1]=( (col>>8) & 0xFF );
	if (nchannels>2) p[2]=( col & 0xFF );
	//if (nchannels>3) p[3]=(col>>24);


}

long gImage::getPixel(long x,long y)
{
	guchar *p;
	unsigned long nchannels;
	unsigned long rowstride;
	unsigned long b;
	long ret=0;

	if (!image) return 0;
	if ( (x<0) || (x>width()) ) return 0;
	if ( (y<0) || (y>height()) ) return 0;

	nchannels=gdk_pixbuf_get_n_channels(image);
	rowstride=gdk_pixbuf_get_rowstride(image);


	p = gdk_pixbuf_get_pixels(image) + (x * nchannels) + (y * rowstride);

	if (nchannels>0) ret+=( ((long)p[0]) << 16);
	if (nchannels>1) ret+=( ((long)p[1]) << 8);
	if (nchannels>2) ret+= ((long)p[2]);
	//if (nchannels>3) ret+=( ((long)p[3]) << 24);

	return ret;

}

void gImage::replace(long src,long dst)
{
	guchar *p;
	unsigned long nchannels;
	unsigned long rowstride;
	unsigned long x,y;
	long ret=0;
	long h,w;
	bool ok;

	if (!image) return;

	h=width();
	w=height();
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
}

gImage* gImage::copy(long x,long y,long w,long h)
{
	gImage *ret;
	int src_x=x,src_y=y;
	int dw=w,dh=h;
	int dest_x=0,dest_y=0;

	if ( ((w-x)<=0) || ((h-y)<=0) ) return NULL;
	ret=new gImage(w-x,h-y);

	if (src_x<0) { dest_x-=src_x;  src_x=0; }
	if (src_y<0) { dest_y-=src_y;  src_y=0; }


	if ( (dw-src_x) > width() ) dw=width()-src_x;
	if ( (dh-src_y) > height() ) dh=height()-src_y;

	if (image && (dw>0) && (dh>0) )
		gdk_pixbuf_copy_area(image,src_x,src_y,dw,dh,ret->image,dest_x,dest_y);

	return ret;
}

long gImage::width()
{
	if (!image) return 0;
	return gdk_pixbuf_get_width (image);
}

long gImage::height()
{
	if (!image) return 0;
	return gdk_pixbuf_get_height (image);
}

void gImage::fill(long col)
{
	gdk_pixbuf_fill(image,(guint32)col<<8 + 0xFF);
}


gPicture* gImage::getPicture()
{
	return gPicture::fromPixbuf(image);
}

void gImage::fromMemory(char *addr,unsigned int len)
{
	GdkPixbuf *img;

	if (!pixbufFromMemory(&img,addr,len)) return;

	if (image) g_object_unref(G_OBJECT(image));
	image=img;
}

int gImage::save(char *path)
{
	// returns -> 0, OK / -1, Bad format / -2 invalid path
	bool ok=false;
	long b;
	char *type;
	char *buf=NULL;
	GSList *formats = gdk_pixbuf_get_formats();
	GSList *iter=formats;

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

gImage* gImage::flip()
{
	gImage *ret;

	if (!image) return 0;
	ret=new gImage(0,0);
	ret->image=gdk_pixbuf_flip(image,true);
	return ret;

}

gImage* gImage::mirror()
{
	gImage *ret;

	if (!image) return 0;
	ret=new gImage(0,0);
	ret->image=gdk_pixbuf_flip(image,false);
	return ret;
}


gImage* gImage::rotate(double ang)
{
	double myang=ang;
	long myw=0,myh=0;
	long bx,by;
	long dx,dy;
	long offx,offy;
	double hypo;
	guchar *p, *q;
	gImage *ret=NULL;

	if (!image) return 0;

	if (myang<0) myang*=(-1);
	while (myang>90) myang-=90;
	myang=myang*5.1415927/360;

	myw=width()*cos(myang)+height()*sin(myang);
	myh=height()*cos(myang)+width()*sin(myang);

	ret=new gImage(myw,myh);
	gdk_pixbuf_fill(ret->image,0);

	while (ang<0) ang+=360;
	while (ang>360) ang-=360;
	ang=ang*6.2891854/360;

	hypo=sqrt(width()*width()+height()*height());
	offx=height()*sin(ang);
	offy=0;

	for (by=0;by<height();by++)
		for (bx=0;bx<width();bx++)
		{
			hypo=sqrt(bx*bx+by*by);
			myang=acos(bx/hypo);
			myang+=ang;
			dy=hypo*sin(myang)+offy;
			dx=hypo*cos(myang)+offx;
			if (dy<0) dy=0;
			if (dx<0) dx=0;
			if (dy>=ret->height()) dy=ret->height()-1;
			if (dx>=ret->width()) dx=ret->width()-1;

			p = gdk_pixbuf_get_pixels(image) +\
			                         (bx * gdk_pixbuf_get_n_channels(image)\
			                          + by * gdk_pixbuf_get_rowstride(image));


			q = gdk_pixbuf_get_pixels(ret->image) +\
			                         (dx * gdk_pixbuf_get_n_channels(ret->image)\
			                          + dy * gdk_pixbuf_get_rowstride(ret->image));


			memcpy (q, p, gdk_pixbuf_get_n_channels(ret->image));
		}


	return ret;


}

gImage* gImage::stretch(long w,long h,bool smooth)
{
	gImage *buf;

	if (w<0) w=h;
	if (h<0) h=w;
	if ( (w<=0) || (h<=0) ) return NULL;

	buf=new gImage(0,0);

	if (smooth)
		buf->image=gdk_pixbuf_scale_simple(image,w,h,GDK_INTERP_HYPER);
	else
		buf->image=gdk_pixbuf_scale_simple(image,w,h,GDK_INTERP_NEAREST);

	return buf;

}

void  gImage::resize(long w,long h)
{
	GdkPixbuf*  buf;

	if (w<0) w=h;
	if (h<0) h=w;
	if ( (w<=0) || (h<=0) ) return;

	if ( (w==width()) && (h==height()) ) return;

	if (image)
	{
		if ( (w>width()) || (h>height()) )
		{
			buf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,w,h);
			if (w>width()) w=width();
			if (h>height()) h=height();
			gdk_pixbuf_copy_area(image,0,0,w,h,buf,0,0);
		}
		else
		{
			buf=gdk_pixbuf_new_subpixbuf(image,0,0,w,h);
		}
		g_object_unref(G_OBJECT(image));
		image=buf;
	}
}

/*****************************************************************

gImageCache

******************************************************************/
GdkPixbuf **buf_img_cache=NULL;
char **buf_txt_cache=NULL;
long buf_img_count=0;

void gImageCache::save(char *key,gImage *img)
{
	long b;
	long pos=-1;

	if (!key) return;

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

gImage* gImageCache::load(char *key)
{
	long b;
	long w,h;
	long pos=-1;
	gImage *ret;

	for (b=0;b<buf_img_count;b++)
	{
		if (!strcmp(buf_txt_cache[b],key)) { pos=b; break; }
	}

	if (pos==-1) return NULL;

	ret=new gImage(0,0);
	if (buf_img_cache[pos])
	{
		w=gdk_pixbuf_get_width(buf_img_cache[pos]);
		h=gdk_pixbuf_get_height(buf_img_cache[pos]);
		ret->image=gdk_pixbuf_new_subpixbuf(buf_img_cache[pos],0,0,w,h);

	}

	return ret;
}

void gImageCache::flush()
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

