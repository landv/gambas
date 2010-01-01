/***************************************************************************

	cpaint_impl.cpp

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

#define __CPAINT_IMPL_CPP

#include <cairo.h>
#include <cairo-xlib.h>

#include "gambas.h"
#include "gb_common.h"
#include "widgets.h"
#include "gdesktop.h"

#include "CWindow.h"
#include "CDrawingArea.h"
#include "CPicture.h"
#include "CImage.h"
#include "cprinter.h"
#include "csvgimage.h"
#include "CFont.h"
#include "CDraw.h"
#include "cpaint_impl.h"

/**** Cairo image management *********************************************/

static void free_image(GB_IMG *img, void *image)
{
	cairo_surface_destroy((cairo_surface_t *)image);
}

static void *temp_image(GB_IMG *img)
{
	cairo_surface_t *image;

	if (!img->data)
		image = NULL; // TODO: use a static small image surface
	else
		image = cairo_image_surface_create_for_data(img->data, CAIRO_FORMAT_ARGB32, img->width, img->height, 
		                                            cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, img->width));
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.gtk.cairo",
	GB_IMAGE_BGRP,
	free_image,
	free_image,
	temp_image
	};

static cairo_surface_t *check_image(void *img)
{
	// TODO: format is endian-dependent
	return (cairo_surface_t *)IMAGE.Check((GB_IMG *)img, &_image_owner);
}

/**** Paint implementation ***********************************************/

typedef
	struct {
		cairo_t *context;
		CFONT *font;
		}
	GB_PAINT_EXTRA;

#define EXTRA(d) ((GB_PAINT_EXTRA *)d->extra)
#define CONTEXT(d) EXTRA(d)->context

static bool init_painting(GB_PAINT *d, cairo_surface_t *target, double w, double h, int rx, int ry)
{
	d->width = w;
	d->height = h;
	d->resolutionX = rx; //device->physicalDpiX();
	d->resolutionY = ry; //device->physicalDpiY();
	
	/*if (device->paintingActive())
	{
		GB.Error("Device already being painted");
		return TRUE;
	}*/
	
	if (target)
	{
		EXTRA(d)->context = cairo_create(target);
		cairo_surface_destroy(target);
	}
	
	EXTRA(d)->font = NULL;
	
	return FALSE;
}


static int Begin(GB_PAINT *d)
{
	void *device = d->device;
	cairo_surface_t *target = NULL;
	double w, h;
	int rx = 96, ry = 96;
	
	if (GB.Is(device, CLASS_Picture))
	{
		gPicture *picture = ((CPICTURE *)device)->picture;
		GdkDrawable *pixmap;
		
		if (picture->isVoid())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		
		pixmap = (GdkDrawable *)picture->getPixmap();
		w = picture->width();
		h = picture->height();
		
		target = 
			cairo_xlib_surface_create(gdk_x11_drawable_get_xdisplay(pixmap), gdk_x11_drawable_get_xid(pixmap), 
				gdk_x11_visual_get_xvisual(gdk_drawable_get_visual(pixmap)), w, h);
	}
	else if (GB.Is(device, CLASS_Image))
	{
		target = check_image(device);
		cairo_surface_reference(target);
		w = ((GB_IMG *)device)->width;
		h = ((GB_IMG *)device)->height;
		if (!target)
		{
			GB.Error("Bad image");
			return TRUE;
		}
	}
	else if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)device)->widget;
		GdkDrawable *dr;
		
		w = wid->width();
		h = wid->height();

		if (wid->cached())
		{
			wid->resizeCache(); // Why is it needed?
			dr = wid->buffer;
		}
		else
		{
			dr = GTK_LAYOUT(wid->widget)->bin_window; 
		}

		/*target = 
			cairo_xlib_surface_create(gdk_x11_drawable_get_xdisplay(dr), gdk_x11_drawable_get_xid(dr), 
				gdk_x11_visual_get_xvisual(gdk_drawable_get_visual(dr)), w, h);*/
		
		d->resolutionX = gDesktop::resolution(); //device->physicalDpiX();
		d->resolutionY = gDesktop::resolution(); //device->physicalDpiY();
		
		EXTRA(d)->context = gdk_cairo_create(dr);
	}
	else if (GB.Is(device, CLASS_Printer))
	{
		GtkPrintContext *context = ((CPRINTER *)device)->context;
		
		if (!context)
		{
			GB.Error("Printer is not printing");
			return TRUE;
		}
		
		EXTRA(d)->context = gtk_print_context_get_cairo_context(context);
		cairo_reference(CONTEXT(d));
		w = gtk_print_context_get_width(context);
		h = gtk_print_context_get_height(context);
		rx = (int)gtk_print_context_get_dpi_x(context);
		ry = (int)gtk_print_context_get_dpi_y(context);
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		CSVGIMAGE *svgimage = ((CSVGIMAGE *)device);
		target = SVGIMAGE_init(svgimage);
		if (!target)
		{
			GB.Error("SvgImage size is not defined");
			return TRUE;
		}
		cairo_surface_reference(target);
		w = svgimage->width;
		h = svgimage->height;
		rx = ry = 72;
	}
	else
		return TRUE;
	
	return init_painting(d, target, w, h, rx, ry);
}

static void End(GB_PAINT *d)
{
	void *device = d->device;

	GB.Unref(POINTER(&EXTRA(d)->font));
	
	cairo_destroy(EXTRA(d)->context);

	if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)device)->widget;
		if (wid->cached())
			wid->setCache();
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		CSVGIMAGE *svgimage = ((CSVGIMAGE *)device);
		cairo_surface_finish(svgimage->surface);
	}
}

static void Save(GB_PAINT *d)
{
	cairo_save(CONTEXT(d));
}

static void Restore(GB_PAINT *d)
{
	cairo_restore(CONTEXT(d));
}

// Font is used by X11!
static void Paint_Font(GB_PAINT *d, int set, GB_FONT *font)
{
	if (!EXTRA(d)->font)
	{
		EXTRA(d)->font = CFONT_create(new gFont());
		GB.Ref(EXTRA(d)->font);
	}
		
	if (set)
	{
		GB.Ref(*font);
		GB.Unref(POINTER(&EXTRA(d)->font));
		EXTRA(d)->font = (CFONT *)(*font);
	}
	else
		*font = (GB_FONT)EXTRA(d)->font;
}

/*static void init_path(GB_PAINT *d)
{
	switch (EXTRA(d)->fillRule)
	{
		case GB_PAINT_FILL_RULE_WINDING:
			PATH(d)->setFillRule(Qt::WindingFill); 
			break;
		case GB_PAINT_FILL_RULE_EVEN_ODD: 
		default:
			PATH(d)->setFillRule(Qt::OddEvenFill);
	}
}*/

static void Clip(GB_PAINT *d, int preserve)
{
	if (preserve)
		cairo_clip_preserve(CONTEXT(d));
	else
		cairo_clip(CONTEXT(d));
}

static void ResetClip(GB_PAINT *d)
{
	cairo_reset_clip(CONTEXT(d));
}

static void ClipExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	double x1, y1, x2, y2;
	cairo_clip_extents(CONTEXT(d), &x1, &y1, &x2, &y2);
	
	ext->x1 = (float)x1;
	ext->y1 = (float)y1;
	ext->x2 = (float)x2;
	ext->y2 = (float)y2;
}
	
static void Fill(GB_PAINT *d, int preserve)
{
	if (preserve)
		cairo_fill_preserve(CONTEXT(d));
	else
		cairo_fill(CONTEXT(d));
}

static void Stroke(GB_PAINT *d, int preserve)
{
	if (preserve)
		cairo_stroke_preserve(CONTEXT(d));
	else
		cairo_stroke(CONTEXT(d));
}
		
static void PathExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	double x1, y1, x2, y2;
	cairo_path_extents(CONTEXT(d), &x1, &y1, &x2, &y2);
	
	ext->x1 = (float)x1;
	ext->y1 = (float)y1;
	ext->x2 = (float)x2;
	ext->y2 = (float)y2;
}

static int PathContains(GB_PAINT *d, float x, float y)
{
	return cairo_in_fill(CONTEXT(d), (double)x, (double)y);
}

static void Dash(GB_PAINT *d, int set, float **dashes, int *count)
{
	int i;
	double lw;
	
	lw = cairo_get_line_width(CONTEXT(d));
	if (lw == 0) lw = 1;
	
	if (set)
	{
		if (!*dashes || *count == 0)
			cairo_set_dash(CONTEXT(d), NULL, 0, 0.0);
		else
		{
			double dd[*count];
			
			for (i = 0; i < *count; i++)
				dd[i] = (*dashes)[i] * lw;
			
			cairo_set_dash(CONTEXT(d), dd, *count, 0.0);
		}
	}
	else
	{
		*count = cairo_get_dash_count(CONTEXT(d));
		
		if (*count)
		{
			double dd[*count];
			cairo_get_dash(CONTEXT(d), dd, NULL);
			
			GB.Alloc(POINTER(dashes), sizeof(float) * *count);
			for (int i = 0; i < *count; i++)
				(*dashes)[i] = (float)dd[i] / lw;
		}
		else
		{
			*dashes = NULL;
		}
	}
}

static void DashOffset(GB_PAINT *d, int set, float *offset)
{
	double lw;
	
	lw = cairo_get_line_width(CONTEXT(d));
	if (lw == 0) lw = 1;

	if (set)
	{
		int count = cairo_get_dash_count(CONTEXT(d));
		double dashes[count];
		cairo_get_dash(CONTEXT(d), dashes, NULL);
		cairo_set_dash(CONTEXT(d), dashes, count, (double)*offset * lw);
	}
	else
	{
		double v;
		cairo_get_dash(CONTEXT(d), NULL, &v);
		*offset = (float)v / lw;
	}
}

		
static void FillRule(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_fill_rule_t v;
	
		switch(*value)
		{
			case GB_PAINT_FILL_RULE_EVEN_ODD: v = CAIRO_FILL_RULE_EVEN_ODD; break;
			case GB_PAINT_FILL_RULE_WINDING: default: v = CAIRO_FILL_RULE_WINDING;
		}
		
		cairo_set_fill_rule(CONTEXT(d), v);
	}
	else
	{
		switch(cairo_get_fill_rule(CONTEXT(d)))
		{
			case CAIRO_FILL_RULE_EVEN_ODD: *value = GB_PAINT_FILL_RULE_EVEN_ODD; break;
			case CAIRO_FILL_RULE_WINDING: default: *value = GB_PAINT_FILL_RULE_WINDING;
		}
	}
}

static void LineCap(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_line_cap_t v;
		
		switch (*value)
		{
			case GB_PAINT_LINE_CAP_ROUND: v = CAIRO_LINE_CAP_ROUND; break;
			case GB_PAINT_LINE_CAP_SQUARE: v = CAIRO_LINE_CAP_SQUARE; break;
			case GB_PAINT_LINE_CAP_BUTT: default: v = CAIRO_LINE_CAP_BUTT;
		}
		
		cairo_set_line_cap(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_line_cap(CONTEXT(d)))
		{
			case CAIRO_LINE_CAP_ROUND: *value = GB_PAINT_LINE_CAP_ROUND; break;
			case CAIRO_LINE_CAP_SQUARE: *value = GB_PAINT_LINE_CAP_SQUARE; break;
			case CAIRO_LINE_CAP_BUTT: default: *value = GB_PAINT_LINE_CAP_BUTT;
		}
	}
}

static void LineJoin(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_line_join_t v;
		
		switch (*value)
		{
			case GB_PAINT_LINE_JOIN_ROUND: v = CAIRO_LINE_JOIN_ROUND; break;
			case GB_PAINT_LINE_JOIN_BEVEL: v = CAIRO_LINE_JOIN_BEVEL; break;
			case GB_PAINT_LINE_JOIN_MITER: default: v = CAIRO_LINE_JOIN_MITER;
		}
		
		cairo_set_line_join(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_line_join(CONTEXT(d)))
		{
			case CAIRO_LINE_JOIN_ROUND: *value = GB_PAINT_LINE_JOIN_ROUND; break;
			case CAIRO_LINE_JOIN_BEVEL: *value = GB_PAINT_LINE_JOIN_BEVEL; break;
			case CAIRO_LINE_JOIN_MITER: default: *value = GB_PAINT_LINE_JOIN_MITER;
		}
	}
}

static void LineWidth(GB_PAINT *d, int set, float *value)
{
	if (set)
	{
		float *dashes;
		int count;
		float offset;
		
		Dash(d, FALSE, &dashes, &count);
		DashOffset(d, FALSE, &offset);
		
		cairo_set_line_width(CONTEXT(d), (double)*value);
		
		Dash(d, TRUE, &dashes, &count);
		DashOffset(d, TRUE, &offset);
		GB.Free(POINTER(&dashes));
	}
	else
		*value = (float)cairo_get_line_width(CONTEXT(d));
}

static void MiterLimit(GB_PAINT *d, int set, float *value)
{
	if (set)
		cairo_set_miter_limit(CONTEXT(d), (double)*value);
	else
		*value = (float)cairo_get_miter_limit(CONTEXT(d));
}


static void Operator(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_operator_t v;
		
		switch (*value)
		{
			case GB_PAINT_OPERATOR_CLEAR: v = CAIRO_OPERATOR_CLEAR; break;
			case GB_PAINT_OPERATOR_SOURCE: v = CAIRO_OPERATOR_SOURCE; break;
			case GB_PAINT_OPERATOR_IN: v = CAIRO_OPERATOR_IN; break;
			case GB_PAINT_OPERATOR_OUT: v = CAIRO_OPERATOR_OUT; break;
			case GB_PAINT_OPERATOR_ATOP: v = CAIRO_OPERATOR_ATOP; break;
			case GB_PAINT_OPERATOR_DEST: v = CAIRO_OPERATOR_DEST; break;
			case GB_PAINT_OPERATOR_DEST_OVER: v = CAIRO_OPERATOR_DEST_OVER; break;
			case GB_PAINT_OPERATOR_DEST_IN: v = CAIRO_OPERATOR_DEST_IN; break;
			case GB_PAINT_OPERATOR_DEST_OUT: v = CAIRO_OPERATOR_DEST_OUT; break;
			case GB_PAINT_OPERATOR_DEST_ATOP: v = CAIRO_OPERATOR_DEST_ATOP; break;
			case GB_PAINT_OPERATOR_XOR: v = CAIRO_OPERATOR_XOR; break;
			case GB_PAINT_OPERATOR_ADD: v = CAIRO_OPERATOR_ADD; break;
			case GB_PAINT_OPERATOR_SATURATE: v = CAIRO_OPERATOR_SATURATE; break;
			case GB_PAINT_OPERATOR_OVER: default: v = CAIRO_OPERATOR_OVER; break;
		}
		
		cairo_set_operator(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_operator(CONTEXT(d)))
		{
			case CAIRO_OPERATOR_CLEAR: *value = GB_PAINT_OPERATOR_CLEAR; break;
			case CAIRO_OPERATOR_SOURCE: *value = GB_PAINT_OPERATOR_SOURCE; break;
			case CAIRO_OPERATOR_IN: *value = GB_PAINT_OPERATOR_IN; break;
			case CAIRO_OPERATOR_OUT: *value = GB_PAINT_OPERATOR_OUT; break;
			case CAIRO_OPERATOR_ATOP: *value = GB_PAINT_OPERATOR_ATOP; break;
			case CAIRO_OPERATOR_DEST: *value = GB_PAINT_OPERATOR_DEST; break;
			case CAIRO_OPERATOR_DEST_OVER: *value = GB_PAINT_OPERATOR_DEST_OVER; break;
			case CAIRO_OPERATOR_DEST_IN: *value = GB_PAINT_OPERATOR_DEST_IN; break;
			case CAIRO_OPERATOR_DEST_OUT: *value = GB_PAINT_OPERATOR_DEST_OUT; break;
			case CAIRO_OPERATOR_DEST_ATOP: *value = GB_PAINT_OPERATOR_DEST_ATOP; break;
			case CAIRO_OPERATOR_XOR: *value = GB_PAINT_OPERATOR_XOR; break;
			case CAIRO_OPERATOR_ADD: *value = GB_PAINT_OPERATOR_ADD; break;
			case CAIRO_OPERATOR_SATURATE: *value = GB_PAINT_OPERATOR_SATURATE; break;
			case CAIRO_OPERATOR_OVER: default: *value = GB_PAINT_OPERATOR_OVER;
		}
	}
}


static void NewPath(GB_PAINT *d)
{
	cairo_new_path(CONTEXT(d));
}

static void ClosePath(GB_PAINT *d)
{
	cairo_close_path(CONTEXT(d));
}

		
static void Arc(GB_PAINT *d, float xc, float yc, float radius, float angle, float length)
{
	cairo_new_sub_path(CONTEXT(d));
	angle = - angle;
	if (length >= 0.0)
		cairo_arc_negative(CONTEXT(d), xc, yc, radius, angle, angle - length);
	else
		cairo_arc(CONTEXT(d), xc, yc, radius, angle, angle - length);
}

static void Rectangle(GB_PAINT *d, float x, float y, float width, float height)
{
	cairo_rectangle(CONTEXT(d), x, y, width, height);
}

static void GetCurrentPoint(GB_PAINT *d, float *x, float *y)
{
	double cx, cy;
	
	cairo_get_current_point(CONTEXT(d), &cx, &cy);
	*x = (float)cx;
	*y = (float)cy;
}

static void MoveTo(GB_PAINT *d, float x, float y)
{
	cairo_move_to(CONTEXT(d), x, y);
}

static void LineTo(GB_PAINT *d, float x, float y)
{
	cairo_line_to(CONTEXT(d), x, y);
}

static void CurveTo(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3)
{
	cairo_curve_to(CONTEXT(d), x1, y1, x2, y2, x3, y3);
}

static void draw_text(GB_PAINT *d, bool rich, const char *text, int len, float w, float h, int align)
{
	char *html;
  PangoLayout *layout;
	CFONT *font;
	float tw, th, offx, offy;

  layout = pango_cairo_create_layout(CONTEXT(d));
  
	if (rich)
	{
		html = gt_html_to_pango_string(text, len, false);
		pango_layout_set_markup(layout, html, -1);
		if (w > 0)
			pango_layout_set_width(layout, (int)(w * PANGO_SCALE));
	}
	else
		pango_layout_set_text(layout, text, len);
	
	Paint_Font(d, FALSE, (GB_FONT *)&font);
	gt_add_layout_from_font(layout, font->font);
	
	if (w > 0 && h > 0)
		gt_layout_alignment(layout, w, h, &tw, &th, align, &offx, &offy);
	else
	{
		offx = 0;
		offy = -(font->font->ascentF());
	}
	
	cairo_rel_move_to(CONTEXT(d), offx, offy);
  pango_cairo_layout_path(CONTEXT(d), layout);

	g_object_unref(layout);	
	if (rich)
		g_free(html);
}

static void Text(GB_PAINT *d, const char *text, int len, float w, float h, int align)
{
	draw_text(d, FALSE, text, len, w, h, align);
}

/*static void RichText(GB_PAINT *d, const char *text, int len, float w, float h, int align)
{
	draw_text(d, TRUE, text, len, w, h, align);
}*/


static void TextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext)
{
	PangoLayout *layout;
	CFONT *font;
	PangoRectangle rect;
	PangoRectangle logrect;
	
	layout = pango_cairo_create_layout(CONTEXT(d));
	Paint_Font(d, FALSE, (GB_FONT *)&font);
	gt_add_layout_from_font(layout, font->font);
	pango_layout_set_text(layout, text, len);
	pango_layout_get_extents(layout, &rect, &logrect);
	
	ext->x2 = rect.width;
	ext->y2 = rect.height;
	g_object_unref(layout);

}


		
static void Matrix(GB_PAINT *d, int set, GB_TRANSFORM matrix)
{
	cairo_matrix_t *t = (cairo_matrix_t *)matrix;
	
	if (set)
	{
		if (t)
			cairo_set_matrix(CONTEXT(d), t);
		else
			cairo_identity_matrix(CONTEXT(d));
	}
	else
		cairo_get_matrix(CONTEXT(d), t);
}

		
static void SetBrush(GB_PAINT *d, GB_BRUSH brush)
{
	cairo_set_source(CONTEXT(d), (cairo_pattern_t *)brush);
}

		
static void BrushFree(GB_BRUSH brush)
{
	// Should I release the surface associated with an image brush? Apparently no.
	cairo_pattern_destroy((cairo_pattern_t *)brush);
}

static void BrushColor(GB_BRUSH *brush, GB_COLOR color)
{
	int r, g, b, a;
	GB_COLOR_SPLIT(color, r, g, b, a);
	*brush = (GB_BRUSH)cairo_pattern_create_rgba(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

// Function partially taken from the GTK+ source code.

static cairo_surface_t *gdk_cairo_create_surface_from_pixbuf(const GdkPixbuf *pixbuf)
{
  gint width = gdk_pixbuf_get_width (pixbuf);
  gint height = gdk_pixbuf_get_height (pixbuf);
  guchar *gdk_pixels = gdk_pixbuf_get_pixels (pixbuf);
  int gdk_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  int n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  int cairo_stride;
  guchar *cairo_pixels;
  cairo_format_t format;
  cairo_surface_t *surface;
  static const cairo_user_data_key_t key = { 0 };
  int j;

  if (n_channels == 3)
    format = CAIRO_FORMAT_RGB24;
  else
    format = CAIRO_FORMAT_ARGB32;

  cairo_stride = cairo_format_stride_for_width (format, width);
  cairo_pixels = (uchar *)g_malloc (height * cairo_stride);
  surface = cairo_image_surface_create_for_data ((unsigned char *)cairo_pixels,
                                                 format,
                                                 width, height, cairo_stride);

  cairo_surface_set_user_data (surface, &key,
			       cairo_pixels, (cairo_destroy_func_t)g_free);

  for (j = height; j; j--)
    {
      guchar *p = gdk_pixels;
      guchar *q = cairo_pixels;

      if (n_channels == 3)
	{
	  guchar *end = p + 3 * width;
	  
	  while (p < end)
	    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	      q[0] = p[2];
	      q[1] = p[1];
	      q[2] = p[0];
#else	  
	      q[1] = p[0];
	      q[2] = p[1];
	      q[3] = p[2];
#endif
	      p += 3;
	      q += 4;
	    }
	}
      else
	{
	  guchar *end = p + 4 * width;
	  guint t1,t2,t3;
	    
#define MULT(d,c,a,t) G_STMT_START { t = c * a + 0x7f; d = ((t >> 8) + t) >> 8; } G_STMT_END

	  while (p < end)
	    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	      MULT(q[0], p[2], p[3], t1);
	      MULT(q[1], p[1], p[3], t2);
	      MULT(q[2], p[0], p[3], t3);
	      q[3] = p[3];
#else	  
	      q[0] = p[3];
	      MULT(q[1], p[0], p[3], t1);
	      MULT(q[2], p[1], p[3], t2);
	      MULT(q[3], p[2], p[3], t3);
#endif
	      
	      p += 4;
	      q += 4;
	    }
	  
#undef MULT
	}

      gdk_pixels += gdk_rowstride;
      cairo_pixels += cairo_stride;
    }

	return surface;
}

static void BrushImage(GB_BRUSH *brush, GB_IMAGE image)
{
	gPicture *picture = CIMAGE_get((CIMAGE *)image);
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;

	surface = gdk_cairo_create_surface_from_pixbuf(picture->getPixbuf());
	
	pattern = cairo_pattern_create_for_surface(surface);
	cairo_surface_destroy(surface);

	cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
	
	*brush = (GB_BRUSH)pattern;
}

static void handle_color_stop(cairo_pattern_t *pattern, int nstop, const double *positions, const GB_COLOR *colors)
{
	int i, r, g, b, a;
	
	for (i = 0; i < nstop; i++)
	{
		GB_COLOR_SPLIT(colors[i], r, g, b, a);
		cairo_pattern_add_color_stop_rgba(pattern, positions[i], r / 255.0, g / 255.0, b / 255.0, a / 255.0);
	}
}

static void set_pattern_extend(cairo_pattern_t *pattern, int extend)
{
	cairo_extend_t cext;

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT: cext = CAIRO_EXTEND_REPEAT; break;
		case GB_PAINT_EXTEND_REFLECT: cext = CAIRO_EXTEND_REFLECT; break;
		case GB_PAINT_EXTEND_PAD: default: cext = CAIRO_EXTEND_PAD;
	}
	
	cairo_pattern_set_extend(pattern, cext);
}

static void BrushLinearGradient(GB_BRUSH *brush, float x0, float y0, float x1, float y1, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	cairo_pattern_t *pattern;
		
	pattern = cairo_pattern_create_linear(x0, y0, x1, y1);
	
	handle_color_stop(pattern, nstop, positions, colors);
	set_pattern_extend(pattern, extend);
	
	*brush = (GB_BRUSH)pattern;
}

static void BrushRadialGradient(GB_BRUSH *brush, float cx, float cy, float r, float fx, float fy, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	cairo_pattern_t *pattern;
		
	// I know that from librsvg sources
	pattern = cairo_pattern_create_radial(fx, fy, 0.0, cx, cy, r);

	handle_color_stop(pattern, nstop, positions, colors);
	set_pattern_extend(pattern, extend);

	*brush = (GB_BRUSH)pattern;
}

// Matrix must be inverted, so that it behaves the same way as in Qt4

static void BrushMatrix(GB_BRUSH brush, int set, GB_TRANSFORM matrix)
{
	cairo_matrix_t *t = (cairo_matrix_t *)matrix;
	cairo_pattern_t *pattern = (cairo_pattern_t *)brush;
	cairo_matrix_t actual;
	
	if (set)
	{
		if (t)
		{
			actual = *t;
			cairo_matrix_invert(&actual);
		}
		else
			cairo_matrix_init_identity(&actual);
		
		cairo_pattern_set_matrix(pattern, &actual);
	}
	else
	{
		cairo_pattern_get_matrix(pattern, t);
		cairo_matrix_invert(t);
	}
}

static void TransformCreate(GB_TRANSFORM *matrix)
{
	GB.Alloc(POINTER(matrix), sizeof(cairo_matrix_t));
	cairo_matrix_init_identity((cairo_matrix_t *)*matrix);
}

static void TransformDelete(GB_TRANSFORM *matrix)
{
	GB.Free(POINTER(matrix));
}

static void TransformInit(GB_TRANSFORM matrix, float xx, float yx, float xy, float yy, float x0, float y0)
{
	cairo_matrix_init((cairo_matrix_t *)matrix, xx, yx, xy, yy, x0, y0);
}

static void TransformTranslate(GB_TRANSFORM matrix, float tx, float ty)
{
	cairo_matrix_translate((cairo_matrix_t *)matrix, tx, ty);
}

static void TransformScale(GB_TRANSFORM matrix, float sx, float sy)
{
	cairo_matrix_scale((cairo_matrix_t *)matrix, sx, sy);
}

static void TransformRotate(GB_TRANSFORM matrix, float angle)
{
	cairo_matrix_rotate((cairo_matrix_t *)matrix, -angle);
}

static int TransformInvert(GB_TRANSFORM matrix)
{
	cairo_status_t status = cairo_matrix_invert((cairo_matrix_t *)matrix);
	return status != CAIRO_STATUS_SUCCESS;
}

static void TransformMultiply(GB_TRANSFORM matrix, GB_TRANSFORM matrix2)
{
	cairo_matrix_multiply((cairo_matrix_t *)matrix, (cairo_matrix_t *)matrix, (cairo_matrix_t *)matrix2);
}


GB_PAINT_DESC PAINT_Interface = {
	sizeof(GB_PAINT_EXTRA),
	Begin,
	End,
	Save,
	Restore,
	Paint_Font,
	Clip,
	ResetClip,
	ClipExtents,
	Fill,
	Stroke,
	PathExtents,
	PathContains,
	Dash,
	DashOffset,
	FillRule,
	LineCap,
	LineJoin,
	LineWidth,
	MiterLimit,
	Operator,
	NewPath,
	ClosePath,
	Arc,
	Rectangle,
	GetCurrentPoint,
	MoveTo,
	LineTo,
	CurveTo,
	Text,
	TextExtents,
	Matrix,
	SetBrush,
	{
		BrushFree,
		BrushColor,
		BrushImage,
		BrushLinearGradient,
		BrushRadialGradient,
		BrushMatrix,
	},
	{
		TransformCreate,
		TransformDelete,
		TransformInit,
		TransformTranslate,
		TransformScale,
		TransformRotate,
		TransformInvert,
		TransformMultiply
	}
};

void PAINT_begin(void *device)
{
	DRAW.Paint.Begin(device);
}

void PAINT_end()
{
	DRAW.Paint.End();
}

void PAINT_clip(int x, int y, int w, int h)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	cairo_rectangle(CONTEXT(d), x, y, w, h);
	cairo_clip(CONTEXT(d));
}

