/***************************************************************************

  cpaint_impl.cpp

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

static GB_COLOR get_color(GB_PAINT *d, GB_COLOR col)
{
	if (col == GB_COLOR_DEFAULT)
	{
		if (GB.Is(d->device, CLASS_DrawingArea))
			col = (((CWIDGET *)d->device)->widget)->realBackground(true);
		else
			col = 0xFFFFFF;
	}

	return col;
}


/**** Paint implementation ***********************************************/

typedef
	struct {
		cairo_t *context;
		GtkPrintContext *print_context;
		CFONT *font;
		CFONT **font_stack;
		cairo_matrix_t init;
		double dx;
		double dy;
		double bx;
		double by;
		bool invert;
		}
	GB_PAINT_EXTRA;

#define EXTRA(d) ((GB_PAINT_EXTRA *)d->extra)
#define CONTEXT(d) EXTRA(d)->context
//#define DX(d) EXTRA(d)->dx
//#define DY(d) EXTRA(d)->dy
#define DX(d) 0
#define DY(d) 0

static bool init_painting(GB_PAINT *d, cairo_surface_t *target, double w, double h, int rx, int ry)
{
	GB_PAINT_EXTRA *dx = EXTRA(d);
	
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
		dx->context = cairo_create(target);
		cairo_surface_destroy(target);
	}
	
	cairo_set_line_width(CONTEXT(d), 1.0);
	/*cairo_set_line_join(CONTEXT(d), CAIRO_LINE_JOIN_MITER);
	cairo_set_miter_limit(CONTEXT(d), 10.0);
	cairo_set_line_cap(CONTEXT(d), CAIRO_LINE_CAP_BUTT);*/
	
	dx->font = NULL;
	dx->font_stack = NULL;
	
	cairo_get_matrix(CONTEXT(d), &EXTRA(d)->init);

	return FALSE;
}

#if 0
static void _gtk_print_context_rotate_according_to_orientation (GtkPrintContext *context, cairo_t *cr)
{
  cairo_matrix_t matrix;
  gdouble width, height;
	GtkPageSetup *page = gtk_print_context_get_page_setup(context);

  /*width = gtk_paper_size_get_width (paper_size, GTK_UNIT_INCH);
  width = width * context->surface_dpi_x / context->pixels_per_unit_x;
  height = gtk_paper_size_get_height (paper_size, GTK_UNIT_INCH);
  height = height * context->surface_dpi_y / context->pixels_per_unit_y;*/
	
	width = gtk_print_context_get_width(context);
	height = gtk_print_context_get_height(context);
  
  switch (gtk_page_setup_get_orientation (page))
    {
    default:
    case GTK_PAGE_ORIENTATION_PORTRAIT:
      break;
    case GTK_PAGE_ORIENTATION_LANDSCAPE:
			fprintf(stderr, "rotate landscape\n");
      cairo_translate (cr, 0, height);
      cairo_matrix_init (&matrix,
			 0, -1,
			 1,  0,
			 0,  0);
      cairo_transform (cr, &matrix);
      break;
    case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
      cairo_translate (cr, width, height);
      cairo_matrix_init (&matrix,
			 -1,  0,
			  0, -1,
			  0,  0);
      cairo_transform (cr, &matrix);
      break;
    case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
      cairo_translate (cr, width, 0);
      cairo_matrix_init (&matrix,
			  0,  1,
			 -1,  0,
			  0,  0);
      cairo_transform (cr, &matrix);
      break;
    }
}
#endif

static int Begin(GB_PAINT *d)
{
	void *device = d->device;
	cairo_surface_t *target = NULL;
	double w, h;
	int rx = 96, ry = 96;
	
	EXTRA(d)->print_context = NULL;
	EXTRA(d)->dx = EXTRA(d)->dy = 0;
	
	if (GB.Is(device, CLASS_Picture))
	{
		gPicture *picture = ((CPICTURE *)device)->picture;

		if (picture->isVoid())
		{
			GB.Error("Bad picture");
			return TRUE;
		}

		w = picture->width();
		h = picture->height();

#ifdef GTK3
		target = picture->getSurface();
		cairo_surface_reference(target);
#else
		GdkDrawable *pixmap = (GdkDrawable *)picture->getPixmap();
		
		target = 
			cairo_xlib_surface_create(gdk_x11_drawable_get_xdisplay(pixmap), gdk_x11_drawable_get_xid(pixmap), 
				gdk_x11_visual_get_xvisual(gdk_drawable_get_visual(pixmap)), w, h);
#endif
	}
	else if (GB.Is(device, CLASS_Image))
	{
		target = check_image(device);
		if (!target)
		{
			GB.Error("Bad image");
			return TRUE;
		}

		cairo_surface_reference(target);
		w = ((GB_IMG *)device)->width;
		h = ((GB_IMG *)device)->height;
	}
	else if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)device)->widget;
		double dx = 0, dy = 0;
		
		w = wid->width();
		h = wid->height();

#ifdef GTK3
		if (wid->cached())
		{
			EXTRA(d)->context = cairo_create(wid->buffer);
		}
		else
		{
			if (!wid->inDrawEvent())
			{
				GB.Error("Cannot paint outside of Draw event handler");
				return TRUE;
			}
			
			EXTRA(d)->context = ((CDRAWINGAREA *)device)->context;
			cairo_reference(CONTEXT(d));

			/*GtkAllocation a;
			gtk_widget_get_allocation(wid->border, &a);
			dx = a.x;
			dy = a.y;*/

		}

		d->resolutionX = gDesktop::resolution(); //device->physicalDpiX();
		d->resolutionY = gDesktop::resolution(); //device->physicalDpiY();

#else
		GdkDrawable *dr;

		if (wid->cached())
		{
			wid->resizeCache(); // Why is it needed?
			dr = wid->buffer;
		}
		else
		{
			if (!wid->inDrawEvent())
			{
				GB.Error("Cannot paint outside of Draw event handler");
				return TRUE;
			}
			
			GtkAllocation *a = &wid->widget->allocation;
			dx = a->x;
			dy = a->y;
			dr = gtk_widget_get_window(wid->widget);
		}

		d->resolutionX = gDesktop::resolution(); //device->physicalDpiX();
		d->resolutionY = gDesktop::resolution(); //device->physicalDpiY();
		
		EXTRA(d)->context = gdk_cairo_create(dr);
#endif

		EXTRA(d)->dx = dx;
		EXTRA(d)->dy = dy;

		cairo_translate(CONTEXT(d), dx, dy);
	}
	else if (GB.Is(device, CLASS_Printer))
	{
		GtkPrintContext *context = ((CPRINTER *)device)->context;
		
		if (!context)
		{
			GB.Error("Printer is not printing");
			return TRUE;
		}
		
		EXTRA(d)->print_context = context;
		EXTRA(d)->context = gtk_print_context_get_cairo_context(context);
		
		//_gtk_print_context_rotate_according_to_orientation(context, CONTEXT(d));
		
		/*fprintf(stderr, "Paint.Begin: orientation = %d w = %g h = %g\n", gtk_page_setup_get_orientation(gtk_print_context_get_page_setup(context)),
			gtk_page_setup_get_paper_width(gtk_print_context_get_page_setup(context), GTK_UNIT_MM),
			gtk_page_setup_get_paper_height(gtk_print_context_get_page_setup(context), GTK_UNIT_MM)
		);*/
		
		cairo_reference(CONTEXT(d));
		
		cairo_surface_set_fallback_resolution(cairo_get_target(CONTEXT(d)), 1200, 1200);
		
		w = gtk_print_context_get_width(context);
		h = gtk_print_context_get_height(context);
		//fprintf(stderr, "Paint.Begin: w = %g h = %g\n", w, h);
		
		rx = (int)gtk_print_context_get_dpi_x(context);
		ry = (int)gtk_print_context_get_dpi_y(context);
		
		//cairo_get_matrix(CONTEXT(d), &EXTRA(d)->init);
		/*cairo_identity_matrix(CONTEXT(d));
		cairo_get_matrix(CONTEXT(d), &t);
		fprintf(stderr, "matrix: [%g %g %g]\n        [%g %g %g]\n", t.xx, t.xy, t.x0, t.yx, t.yy, t.y0);*/
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		CSVGIMAGE *svgimage = ((CSVGIMAGE *)device);
		target = SVGIMAGE_begin(svgimage);
		if (!target)
			return TRUE;
		
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
	GB_PAINT_EXTRA *dx = EXTRA(d);

	if (dx->font_stack)
		GB.FreeArray(POINTER(&dx->font_stack));
	GB.Unref(POINTER(&dx->font));
	
	if (GB.Is(device, CLASS_Picture))
	{
		gPicture *picture = ((CPICTURE *)device)->picture;
		picture->invalidate();
	}
	else if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)device)->widget;
		if (wid && wid->cached())
			wid->setCache();
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		CSVGIMAGE *svgimage = ((CSVGIMAGE *)device);
		SVGIMAGE_end(svgimage);
	}
	
	cairo_destroy(dx->context);
}

static void Save(GB_PAINT *d)
{
	GB_PAINT_EXTRA *dx = EXTRA(d);
	CFONT **pfont;
	
	cairo_save(dx->context);
	
	if (!dx->font_stack)
		GB.NewArray(POINTER(&dx->font_stack), sizeof(void *), 0);
	
	pfont = (CFONT **)GB.Add(POINTER(&dx->font_stack));
	*pfont = dx->font;
	if (*pfont)
		GB.Ref(*pfont);
}

static void Restore(GB_PAINT *d)
{
	GB_PAINT_EXTRA *dx = EXTRA(d);
	
	cairo_restore(dx->context);
	
	if (dx->font_stack && GB.Count(dx->font_stack) > 0)
	{
		CFONT *font = dx->font_stack[GB.Count(dx->font_stack) - 1];
		GB.Unref(POINTER(&dx->font));
		dx->font = font;
		GB.Remove(POINTER(&dx->font_stack), GB.Count(dx->font_stack) - 1, 1);
	}
}

static void Antialias(GB_PAINT *d, int set, int *antialias)
{
	if (set)
		cairo_set_antialias(CONTEXT(d), *antialias ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
	else
		*antialias = (cairo_get_antialias(CONTEXT(d)) == CAIRO_ANTIALIAS_NONE) ? 0 : 1;
}

// Font is used by X11!
static void _Font(GB_PAINT *d, int set, GB_FONT *font)
{
	if (set)
	{
		GB.Unref(POINTER(&EXTRA(d)->font));
		if (*font)
		{
			EXTRA(d)->font = CFONT_create(((CFONT *)(*font))->font->copy());
			GB.Ref(EXTRA(d)->font);
		}
		else
			EXTRA(d)->font = NULL;
	}
	else
	{
		if (!EXTRA(d)->font)
		{
			if (GB.Is(d->device, CLASS_DrawingArea))
			{
				gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)d->device)->widget;
				EXTRA(d)->font = CFONT_create(wid->font()->copy());
			}
			else
			{
				EXTRA(d)->font = CFONT_create(new gFont());
			}
			
			GB.Ref(EXTRA(d)->font);
		}
			
		*font = (GB_FONT)EXTRA(d)->font;
	}
}


static void Background(GB_PAINT *d, int set, GB_COLOR *color)
{
	if (set)
	{
		int r, g, b, a;
		int col = get_color(d, *color);
		GB_COLOR_SPLIT(col, r, g, b, a);
		cairo_set_source_rgba(CONTEXT(d), r / 255.0, g / 255.0, b / 255.0, a / 255.0);
	}
	else
	{
		double r, g, b, a;
		if (cairo_pattern_get_rgba(cairo_get_source(CONTEXT(d)), &r, &g, &b, &a) != CAIRO_STATUS_SUCCESS)
			*color = 0;
		else
			*color = GB_COLOR_MAKE((int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0), (int)(a * 255.0));
	}
}


static void Invert(GB_PAINT *d, int set, int *invert)
{
	#if CAIRO_MAJOR >= 2 || (CAIRO_MAJOR == 1 && CAIRO_MINOR >= 10)
	if (set)
		cairo_set_operator(CONTEXT(d), *invert ? CAIRO_OPERATOR_DIFFERENCE : CAIRO_OPERATOR_OVER);
	else
		*invert = cairo_get_operator(CONTEXT(d)) == CAIRO_OPERATOR_DIFFERENCE;
	#else
	if (set)
		EXTRA(d)->invert = *invert;
	else
		*invert = EXTRA(d)->invert;
	#endif
}


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
	
	ext->x1 = (float)x1 - DX(d);
	ext->y1 = (float)y1 - DY(d);
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
	
	ext->x1 = (float)x1 - DX(d);
	ext->y1 = (float)y1 - DY(d);
	ext->x2 = (float)x2;
	ext->y2 = (float)y2;
}

static int PathContains(GB_PAINT *d, float x, float y)
{
	return cairo_in_fill(CONTEXT(d), (double)x + DX(d), (double)y + DY(d));
}

static void PathOutline(GB_PAINT *d, GB_PAINT_OUTLINE_CB cb)
{
	cairo_path_t *path;
	cairo_path_data_t *data;
	int i;

	path = cairo_copy_path_flat(CONTEXT(d));

	for (i = 0; i < path->num_data; i += path->data[i].header.length)
	{
    data = &path->data[i];
    switch (data->header.type)
		{
			case CAIRO_PATH_MOVE_TO:
				(*cb)(GB_PAINT_PATH_MOVE, data[1].point.x, data[1].point.y);
				break;

			case CAIRO_PATH_LINE_TO:
				(*cb)(GB_PAINT_PATH_LINE, data[1].point.x, data[1].point.y);
				break;

			case CAIRO_PATH_CURVE_TO:
				fprintf(stderr, "gb.gtk: warning: CAIRO_PATH_CURVE_TO not supported\n");
				break;

			case CAIRO_PATH_CLOSE_PATH:
				fprintf(stderr, "gb.gtk: warning: CAIRO_PATH_CLOSE_PATH not supported\n");
				break;
    }
	}

	cairo_path_destroy(path);
}

static void Dash(GB_PAINT *d, int set, float **dashes, int *count)
{
	int i;
	double lw;
	
	lw = cairo_get_line_width(CONTEXT(d));
	if (lw == 0) lw = 1;
	
	if (set)
	{
		if (!*count)
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
	
		switch (*value)
		{
			case GB_PAINT_FILL_RULE_EVEN_ODD: v = CAIRO_FILL_RULE_EVEN_ODD; break;
			case GB_PAINT_FILL_RULE_WINDING: default: v = CAIRO_FILL_RULE_WINDING;
		}
		
		cairo_set_fill_rule(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_fill_rule(CONTEXT(d)))
		{
			case CAIRO_FILL_RULE_EVEN_ODD: *value = GB_PAINT_FILL_RULE_EVEN_ODD; break;
			case CAIRO_FILL_RULE_WINDING: default: *value = GB_PAINT_FILL_RULE_WINDING;
		}
	}
}


static void FillStyle(GB_PAINT *d, int set, int *style)
{
	/*if (set)
	{
		EXTRA(d)->fillRule = *value;
	}
	else
		*value = EXTRA(d)->fillRule;*/
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

		
static void Arc(GB_PAINT *d, float xc, float yc, float radius, float angle, float length, bool pie)
{
	xc += DX(d);
	yc += DY(d);
	
	cairo_new_sub_path(CONTEXT(d));

	if (pie)
		cairo_move_to(CONTEXT(d), 0, 0);
	
	if (length < 0.0)
		cairo_arc_negative(CONTEXT(d), xc, yc, radius, angle, angle + length);
	else
		cairo_arc(CONTEXT(d), xc, yc, radius, angle, angle + length);

	if (pie)
		cairo_close_path(CONTEXT(d));
}

static void Ellipse(GB_PAINT *d, float x, float y, float width, float height, float angle, float length, bool pie)
{
	x += DX(d);
	y += DY(d);
	
	cairo_new_sub_path(CONTEXT(d));
	
	cairo_save(CONTEXT(d));

	cairo_translate(CONTEXT(d), x + width / 2, y + height / 2);
	cairo_scale(CONTEXT(d), width / 2, height / 2);

	if (pie)
		cairo_move_to(CONTEXT(d), 0, 0);
	
	if (length < 0.0)
		cairo_arc_negative(CONTEXT(d), 0, 0, 1, angle, angle + length);
	else
		cairo_arc(CONTEXT(d), 0, 0, 1, angle, angle + length);
	
	if (pie)
		cairo_close_path(CONTEXT(d));

	cairo_restore(CONTEXT(d));
}

static void Rectangle(GB_PAINT *d, float x, float y, float width, float height)
{
	x += DX(d);
	y += DY(d);
	cairo_rectangle(CONTEXT(d), x, y, width, height);
}

static void ClipRect(GB_PAINT *d, int x, int y, int w, int h)
{
	ResetClip(d);
	Rectangle(d, x, y, w, h);
	Clip(d, FALSE);
}

static void GetCurrentPoint(GB_PAINT *d, float *x, float *y)
{
	double cx, cy;
	
	cairo_get_current_point(CONTEXT(d), &cx, &cy);
	*x = (float)cx - DX(d);
	*y = (float)cy - DY(d);
}

static void MoveTo(GB_PAINT *d, float x, float y)
{
	cairo_move_to(CONTEXT(d), x + DX(d), y + DY(d));
}

static void LineTo(GB_PAINT *d, float x, float y)
{
	cairo_line_to(CONTEXT(d), x + DX(d), y + DY(d));
}

static void CurveTo(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3)
{
	cairo_curve_to(CONTEXT(d), x1 + DX(d), y1 + DY(d), x2 + DX(d), y2 + DY(d), x3 + DX(d), y3 + DY(d));
}

static PangoLayout *create_pango_layout(GB_PAINT *d)
{
	GB_PAINT_EXTRA *dx = EXTRA(d);
	
	/*if (dx->print_context)
		return gtk_print_context_create_pango_layout(dx->print_context);
	else*/
		return pango_cairo_create_layout(dx->context);
}

static void draw_text(GB_PAINT *d, bool rich, const char *text, int len, float w, float h, int align, bool draw)
{
	char *html = NULL;
	PangoLayout *layout;
	CFONT *font;
	float tw, th, offx, offy;

	layout = create_pango_layout(d);
  
	if (rich)
	{
		html = gt_html_to_pango_string(text, len, false);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
		pango_layout_set_markup(layout, html, -1);
		if (w > 0)
			pango_layout_set_width(layout, (int)(w * PANGO_SCALE));
	}
	else
		pango_layout_set_text(layout, text, len);
	
	_Font(d, FALSE, (GB_FONT *)&font);
	gt_add_layout_from_font(layout, font->font, d->resolutionY);
	
	if (align == GB_DRAW_ALIGN_DEFAULT)
		align = ALIGN_TOP_NORMAL;
	
	if (w > 0 || h > 0)
	{
		gt_layout_alignment(layout, w, h, &tw, &th, align, &offx, &offy);
		if (rich)
			offx = 0;
	}
	else
	{
		offx = 0;
		offy = -(font->font->ascentF());
	}
	
	cairo_rel_move_to(CONTEXT(d), offx + DX(d), offy + DY(d));
	if (draw)
		pango_cairo_show_layout(CONTEXT(d), layout);
	else
		pango_cairo_layout_path(CONTEXT(d), layout);

	g_object_unref(layout);	
	
	if (html) g_free(html);
}

static void Text(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw)
{
	draw_text(d, FALSE, text, len, w, h, align, draw);
}

static void RichText(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw)
{
	draw_text(d, TRUE, text, len, w, h, align, draw);
}


static void get_text_extents(GB_PAINT *d, bool rich, const char *text, int len, GB_EXTENTS *ext, float width)
{
	char *html = NULL;
	PangoLayout *layout;
	CFONT *font;
	PangoRectangle rect;
	float x, y;
	
	layout = create_pango_layout(d);
	
	if (rich)
	{
		html = gt_html_to_pango_string(text, len, false);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
		pango_layout_set_markup(layout, html, -1);
	}
	else
		pango_layout_set_text(layout, text, len);

	_Font(d, FALSE, (GB_FONT *)&font);
	gt_add_layout_from_font(layout, font->font, d->resolutionY);

	if (width > 0)
		pango_layout_set_width(layout, width * PANGO_SCALE);

	pango_layout_get_extents(layout, &rect, NULL);
	
	GetCurrentPoint(d, &x, &y);
	
	ext->x1 = (float)rect.x / PANGO_SCALE + x;
	ext->y1 = (float)rect.y / PANGO_SCALE + y - font->font->ascentF();
	ext->x2 = ext->x1 + (float)rect.width / PANGO_SCALE;
	ext->y2 = ext->y1 + (float)rect.height / PANGO_SCALE;
	
	g_object_unref(layout);
	if (html) g_free(html);
}

static void TextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext)
{
	get_text_extents(d, FALSE, text, len, ext, -1);
}

static void RichTextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext, float width)
{
	get_text_extents(d, TRUE, text, len, ext, width);
}

static void TextSize(GB_PAINT *d, const char *text, int len, float *w, float *h)
{
	float scale;
	CFONT *font;
	_Font(d, FALSE, (GB_FONT *)&font);
	
	scale = (float)d->resolutionY / gDesktop::resolution();

	*w = font->font->width(text, len) * scale;
	*h = font->font->height(text, len) * scale;
}

static void RichTextSize(GB_PAINT *d, const char *text, int len, float sw, float *w, float *h)
{
	float scale;
	CFONT *font;
	_Font(d, FALSE, (GB_FONT *)&font);

	scale = (float)d->resolutionY / gDesktop::resolution();

	if (sw > 0)
		sw /= scale;

	font->font->richTextSize(text, len, sw, w, h);
	*w *= scale;
	*h *= scale;
}


static void Matrix(GB_PAINT *d, int set, GB_TRANSFORM matrix)
{
	cairo_matrix_t *t = (cairo_matrix_t *)matrix;
	
	if (set)
	{
		if (t)
			cairo_set_matrix(CONTEXT(d), t);
		else
		{
			//if (EXTRA(d)->print_context)
				cairo_set_matrix(CONTEXT(d), &EXTRA(d)->init);
			/*else
			{
				cairo_
				cairo_identity_matrix(CONTEXT(d));
				cairo_translate(CONTEXT(d), EXTRA(d)->dx, EXTRA(d)->dy);
			}*/
		}
	}
	else
		cairo_get_matrix(CONTEXT(d), t);
}

		
static void SetBrush(GB_PAINT *d, GB_BRUSH brush)
{
	cairo_set_source(CONTEXT(d), (cairo_pattern_t *)brush);
}

static void BrushOrigin(GB_PAINT *d, int set, float *x, float *y)
{
	if (set)
	{
		cairo_pattern_t *brush;
		cairo_matrix_t matrix;
		
		brush = cairo_get_source(CONTEXT(d));
		cairo_pattern_get_matrix(brush, &matrix);
		cairo_matrix_translate(&matrix, EXTRA(d)->bx, EXTRA(d)->by);
		cairo_matrix_translate(&matrix, (- *x), (- *y));
		cairo_pattern_set_matrix(brush, &matrix);
		
		EXTRA(d)->bx = *x;
		EXTRA(d)->by = *y;
	}
	else
	{
		*x = EXTRA(d)->bx;
		*y = EXTRA(d)->by;
	}
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

static void BrushImage(GB_BRUSH *brush, GB_IMAGE image)
{
	gPicture *picture = CIMAGE_get((CIMAGE *)image);
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;

	surface = gt_cairo_create_surface_from_pixbuf(picture->getPixbuf());
	
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

static void TransformCopy(GB_TRANSFORM *matrix, GB_TRANSFORM copy)
{
	GB.Alloc(POINTER(matrix), sizeof(cairo_matrix_t));
	*(cairo_matrix_t *)*matrix = *(cairo_matrix_t *)copy;
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

static void TransformMap(GB_TRANSFORM matrix, double *x, double *y)
{
	cairo_matrix_transform_point((cairo_matrix_t *)matrix, x, y);
}


static void DrawImage(GB_PAINT *d, GB_IMAGE image, float x, float y, float w, float h, float opacity, GB_RECT *source)
{
	cairo_t *cr = CONTEXT(d);
	cairo_surface_t *surface;
	cairo_pattern_t *pattern = NULL;
	cairo_pattern_t *save;
	cairo_matrix_t matrix;
	
	save = cairo_get_source(cr);
	cairo_pattern_reference(save);
	cairo_save(cr);

	surface = check_image(image); //picture->getSurface();

	pattern = cairo_pattern_create_for_surface(surface);

	cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
		
	if (source && w >= source->w && h >= source->h && w == (int)w && h == (int)h && ((int)w % source->w) == 0 && ((int)h % source->h) == 0)
		cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);
		
	cairo_matrix_init_identity(&matrix);
	cairo_matrix_translate(&matrix, x, y);
	if (source)
	{
		cairo_matrix_scale(&matrix, w / source->w, h / source->h);
		cairo_matrix_translate(&matrix, -source->x, -source->y);
	}
	else if (w > 0 && h > 0)
		cairo_matrix_scale(&matrix, w / cairo_image_surface_get_width(surface), h / cairo_image_surface_get_height(surface));
	
	cairo_matrix_invert(&matrix);
	cairo_pattern_set_matrix(pattern, &matrix);
	cairo_set_source(cr, pattern);
		
	cairo_rectangle(cr, x, y, w, h);
	
	if (opacity == 1.0)
	{
		cairo_fill(cr);
	}
	else
	{
		cairo_clip(cr);
		cairo_paint_with_alpha(cr, opacity);
	}
	
	cairo_restore(cr);
	cairo_set_source(cr, save);
	cairo_pattern_destroy(save);
	
	cairo_pattern_destroy(pattern);
}

#ifdef GTK3
static void DrawPicture(GB_PAINT *d, GB_PICTURE picture, float x, float y, float w, float h, GB_RECT *source)
{
	cairo_t *cr = CONTEXT(d);
	gPicture *pic = ((CPICTURE *)picture)->picture;
	cairo_surface_t *surface;
	cairo_pattern_t *pattern = NULL;
	cairo_pattern_t *save;
	cairo_matrix_t matrix;

	cairo_save(cr);

	save = cairo_get_source(cr);
	cairo_pattern_reference(save);

	surface = pic->getSurface();

	pattern = cairo_pattern_create_for_surface(surface);

	cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

	if (source && w >= source->w && h >= source->h && w == (int)w && h == (int)h && ((int)w % source->w) == 0 && ((int)h % source->h) == 0)
		cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);

	cairo_matrix_init_identity(&matrix);
	cairo_matrix_translate(&matrix, x, y);
	if (source)
	{
		cairo_matrix_scale(&matrix, w / source->w, h / source->h);
		cairo_matrix_translate(&matrix, -source->x, -source->y);
	}
	else if (w > 0 && h > 0)
		cairo_matrix_scale(&matrix, w / cairo_image_surface_get_width(surface), h / cairo_image_surface_get_height(surface));

	cairo_matrix_invert(&matrix);
	cairo_pattern_set_matrix(pattern, &matrix);
	cairo_set_source(cr, pattern);

	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);

	cairo_set_source(cr, save);
	cairo_pattern_destroy(save);

	cairo_restore(cr);

	cairo_pattern_destroy(pattern);
}
#else
static void DrawPicture(GB_PAINT *d, GB_PICTURE picture, float x, float y, float w, float h, GB_RECT *source)
{
	cairo_pattern_t *pattern, *save;
	cairo_matrix_t matrix;
	gPicture *pic = ((CPICTURE *)picture)->picture;
	
	if (pic->type() != gPicture::PIXMAP || source)
	{
		gt_cairo_draw_pixbuf(CONTEXT(d), pic->getPixbuf(), x, y, w, h, 1.0, source);
		return;
	}
	
	x += DX(d);
	y += DY(d);
	
	cairo_save(CONTEXT(d));
	save = cairo_get_source(CONTEXT(d));
	cairo_pattern_reference(save);
	
	gdk_cairo_set_source_pixmap(CONTEXT(d), pic->getPixmap(), 0, 0);
	
	pattern = cairo_get_source(CONTEXT(d));
	cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
	
	/*if (source && w >= source->w && h >= source->h && w == (int)w && h == (int)h && ((int)w % source->w) == 0 && ((int)h % source->h) == 0)
		cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);*/
	
	//gdk_cairo_set_source_pixbuf(CONTEXT(d), picture->getPixbuf(), x, y);
	
	cairo_matrix_init_identity(&matrix);
	cairo_matrix_translate(&matrix, x, y);
	cairo_matrix_scale(&matrix, w / pic->width(), h / pic->height());
	cairo_matrix_invert(&matrix);
	cairo_pattern_set_matrix(pattern, &matrix);
	
	cairo_rectangle(CONTEXT(d), x, y, w, h);
	cairo_fill(CONTEXT(d));
	
	cairo_set_source(CONTEXT(d), save);
	cairo_pattern_destroy(save);

	cairo_restore(CONTEXT(d));
}
#endif

static void GetPictureInfo(GB_PAINT *d, GB_PICTURE picture, GB_PICTURE_INFO *info)
{
	gPicture *pic = ((CPICTURE *)picture)->picture;
	
	info->width = pic->width();
	info->height = pic->height();
}

static void FillRect(GB_PAINT *d, float x, float y, float w, float h, GB_COLOR color)
{
	cairo_pattern_t *save;
	
	x += DX(d);
	y += DY(d);
	
	save = cairo_get_source(CONTEXT(d));
	cairo_pattern_reference(save);
	
	Background(d, TRUE, &color);
	cairo_rectangle(CONTEXT(d), x, y, w, h);
	cairo_fill(CONTEXT(d));
	
	cairo_set_source(CONTEXT(d), save);
	cairo_pattern_destroy(save);
}


GB_PAINT_DESC PAINT_Interface = 
{
	sizeof(GB_PAINT_EXTRA),
	Begin,
	End,
	Save,
	Restore,
	Antialias,
	_Font,
	Background,
	Invert,
	Clip,
	ResetClip,
	ClipExtents,
	ClipRect,
	Fill,
	Stroke,
	PathExtents,
	PathContains,
	PathOutline,
	Dash,
	DashOffset,
	FillRule,
	FillStyle,
	LineCap,
	LineJoin,
	LineWidth,
	MiterLimit,
	Operator,
	NewPath,
	ClosePath,
	Arc,
	Ellipse,
	Rectangle,
	GetCurrentPoint,
	MoveTo,
	LineTo,
	CurveTo,
	Text,
	TextExtents,
	TextSize,
	RichText,
	RichTextExtents,
	RichTextSize,
	Matrix,
	SetBrush,
	BrushOrigin,
	DrawImage,
	DrawPicture,
	GetPictureInfo,
	FillRect,
	{
		BrushFree,
		BrushColor,
		BrushImage,
		BrushLinearGradient,
		BrushRadialGradient,
		BrushMatrix,
	}
};

GB_PAINT_MATRIX_DESC PAINT_MATRIX_Interface =
{
	TransformCreate,
	TransformCopy,
	TransformDelete,
	TransformInit,
	TransformTranslate,
	TransformScale,
	TransformRotate,
	TransformInvert,
	TransformMultiply,
	TransformMap
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
	if (d)
	{
		cairo_rectangle(CONTEXT(d), (double)x, (double)y, (double)w, (double)h);
		cairo_clip(CONTEXT(d));
	}
}

#ifdef GTK3
#else
void PAINT_clip_region(GdkRegion *region)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (d)
	{
		gdk_cairo_region(CONTEXT(d), region);
		cairo_clip(CONTEXT(d));
	}
}
#endif

cairo_t *PAINT_get_current_context()
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (d)
		return CONTEXT(d);
	
	GB.Error("No current device");
	return NULL;
}

void *PAINT_get_current_device()
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (d)
		return d->device;
	
	GB.Error("No current device");
	return NULL;
}

bool PAINT_get_clip(int *x, int *y, int *w, int *h)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	GB_EXTENTS ext;
	
	ClipExtents(d, &ext);
	
	*x = ceilf(ext.x1);
	*y = ceilf(ext.y1);
	*w = floorf(ext.x2) - *x;
	*h = floorf(ext.y2) - *y;
	
	return *w <= 0 || *h <= 0;
}

void PAINT_apply_offset(int *x, int *y)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	*x += EXTRA(d)->dx;
	*y += EXTRA(d)->dy;
}
