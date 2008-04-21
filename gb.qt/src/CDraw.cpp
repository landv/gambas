/***************************************************************************

  CDraw.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CDRAW_CPP

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qapplication.h>
#include <qpaintdevicemetrics.h>
#include <qpicture.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qsimplerichtext.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qdrawutil.h>

#include "gambas.h"

#include "CConst.h"
#include "CFont.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CImage.h"
#include "CDrawingArea.h"
#include "CColor.h"
#include "CDraw.h"

typedef
	QT_DRAW_EXTRA GB_DRAW_EXTRA;

#define EXTRA(d) ((GB_DRAW_EXTRA *)(&(d->extra)))
#define DP(d) (EXTRA(d)->p)
#define DPM(d) (EXTRA(d)->pm)

#define COLOR_TO_INT(color) ((color).rgb() ^ 0xFF000000)
#define MASK_COLOR(col) ((col & 0xFF000000) ? Qt::color0 : Qt::color1)

DRAW_INTERFACE DRAW EXPORT;

static bool _init = FALSE;

void DRAW_init()
{
	if (_init)
		return;
		
	GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
	_init = TRUE;
}

static void init_drawing(GB_DRAW *d, QPainter *p, int w, int h, int dpi = 0)
{
  EXTRA(d)->p = p;
  EXTRA(d)->pm = 0;
  EXTRA(d)->mask = 0;
  d->width = w;
  d->height = h;
  
	if (dpi)
		d->resolution = dpi;
	else
		#ifdef NO_X_WINDOW
			d->resolution = 72;
		#else
			d->resolution = QPaintDevice::x11AppDpiY();
		#endif
}

// bool DRAW_must_resize_font(void)
// {
//   GB_DRAW *d = DRAW.GetCurrent();
//   return (d->device != CLASS_Printer);
// }
// 
// int DRAW_status(void)
// {
//   if (!THIS)
//     return -1;
//   else
//     return (THIS - draw_stack);
// // }
// 
// void DRAW_restore(int status)
// {
//   while (DRAW_status() != status)
//     DRAW_end();
// }

static QWidget *get_widget(GB_DRAW *d)
{
	if (!GB.Is(d->device, CLASS_Control))
		return NULL;
		
	return QWIDGET(d->device);
}

static uint get_color(GB_DRAW *d, int col, bool bg)
{
	QWidget *wid = get_widget(d);
	
	if (col == COLOR_DEFAULT)
	{
		if (wid)
		{
			if (bg)
				col = wid->paletteBackgroundColor().rgb() & 0xFFFFFF;
			else
				col = wid->paletteForegroundColor().rgb() & 0xFFFFFF;
		}
		else
			col = bg ? 0xFFFFFF : 0x000000;
	}
	
	return col;
}

static int begin(GB_DRAW *d)
{
	void *device = d->device;
	
  if (GB.Is(device, CLASS_Window))
  {
    MyMainWindow *win = (MyMainWindow *)((CWIDGET *)device)->widget;
    init_drawing(d, new QPainter(win, true), win->width(), win->height());
  }
  else if (GB.Is(device, CLASS_Picture))
  {
    CPICTURE *pict = (CPICTURE *)device;

		if (pict->pixmap->isNull())
		{
			GB.Error("Bad picture");
			return TRUE;
		}

    init_drawing(d, new QPainter(pict->pixmap), pict->pixmap->width(), pict->pixmap->height());

    if (pict->pixmap->mask())
    {
      QPen pen;
      QBrush brush;

      EXTRA(d)->mask = new QBitmap(*pict->pixmap->mask());
      DPM(d) = new QPainter(EXTRA(d)->mask);

      pen = DP(d)->pen();
      DPM(d)->setPen(QPen(Qt::color1, pen.width(), pen.style()));
      brush = DP(d)->brush();
      DPM(d)->setBrush(QBrush(Qt::color1, brush.style()));
    }
  }
  else if (GB.Is(device, CLASS_DrawingArea))
  {
    MyDrawingArea *wid = (MyDrawingArea *)(((CWIDGET *)device)->widget);

    if (wid->isCached())
      init_drawing(d, new QPainter(wid->background(), wid), wid->background()->width(), wid->background()->height());
    else if (wid->cache)
      init_drawing(d, new QPainter(wid->cache, wid), wid->cache->width(), wid->cache->height());
    else
      init_drawing(d, new QPainter(wid, wid), wid->width(), wid->height());
      
		wid->drawn++;
  }
  
  return FALSE;
}

static void end(GB_DRAW *d)
{
	void *device = d->device;
  
  delete DP(d);

  if (GB.Is(device, CLASS_Picture))
  {
    if (DPM(d))
    {
      ((CPICTURE *)device)->pixmap->setMask(*(EXTRA(d)->mask));
      delete DPM(d);
      delete EXTRA(d)->mask;
    }
  }
  else if (GB.Is(device, CLASS_DrawingArea))
  {
    MyDrawingArea *wid =  (MyDrawingArea *)(((CWIDGET *)device)->widget);

		if (wid)
		{
    	if (wid->isCached())
    	{
	      wid->refreshBackground();
	      wid->update();
	    }
  
  		wid->drawn--;
		}
	}
}

static void save(GB_DRAW *d)
{
	DP(d)->save();
	if (DPM(d)) DPM(d)->save();
}

static void restore(GB_DRAW *d)
{
	DP(d)->restore();
	if (DPM(d)) DPM(d)->restore();
}

static int get_background(GB_DRAW *d)
{
	return COLOR_TO_INT(DP(d)->backgroundColor());
}

static void set_background(GB_DRAW *d, int col)
{
	col = get_color(d, col, true);
	
	DP(d)->setBackgroundColor(QColor(col));
	if (DPM(d))
		DPM(d)->setBackgroundColor(MASK_COLOR(col));
}

static int get_foreground(GB_DRAW *d)
{
	return COLOR_TO_INT(DP(d)->pen().color());
}

static void set_foreground(GB_DRAW *d, int col)
{
	col = get_color(d, col, false);
	
	QPen pen = DP(d)->pen();
	DP(d)->setPen(QPen(QColor(col), pen.width(), pen.style()));
	if (DPM(d))
		DPM(d)->setPen(QPen(MASK_COLOR(col), pen.width(), pen.style()));
}

static void apply_font(QFont &font, void *object = 0)
{
  GB_DRAW *d = DRAW.GetCurrent();

  DP(d)->setFont(font);
  if (DPM(d))
    DPM(d)->setFont(font);
}

static GB_FONT get_font(GB_DRAW *d)
{
	return CFONT_create(DP(d)->font(), apply_font);
}

static void set_font(GB_DRAW *d, GB_FONT font)
{
	apply_font(*((CFONT*)font)->font);
}

static int is_inverted(GB_DRAW *d)
{
	return DP(d)->rasterOp() == Qt::XorROP;
}

static void set_inverted(GB_DRAW *d, int inverted)
{
	DP(d)->setRasterOp(inverted ? Qt::XorROP : Qt::CopyROP);
}

static int is_transparent(GB_DRAW *d)
{
	return DP(d)->backgroundMode() == Qt::TransparentMode;
}

static void set_transparent(GB_DRAW *d, int transparent)
{
	DP(d)->setBackgroundMode(transparent ? Qt::TransparentMode : Qt::OpaqueMode);
	if (DPM(d))
		DPM(d)->setBackgroundMode(transparent ? Qt::TransparentMode : Qt::OpaqueMode);
}

static int get_line_width(GB_DRAW *d)
{
	return DP(d)->pen().width();
}

static void set_line_width(GB_DRAW *d, int width)
{
	QPen pen = DP(d)->pen();
	DP(d)->setPen(QPen(pen.color(), width, pen.style()));
	if (DPM(d))
		DPM(d)->setPen(QPen(DPM(d)->pen().color(), width, pen.style()));
}

static int get_line_style(GB_DRAW *d)
{
	return DP(d)->pen().style();
}

static void set_line_style(GB_DRAW *d, int style)
{
	QPen pen = DP(d)->pen();
	DP(d)->setPen(QPen(pen.color(), pen.width(), (Qt::PenStyle)style));
	if (DPM(d))
		DPM(d)->setPen(QPen(DPM(d)->pen().color(), pen.width(), (Qt::PenStyle)style));
}

static int get_fill_color(GB_DRAW *d)
{
	return COLOR_TO_INT(DP(d)->brush().color());
}

static void set_fill_color(GB_DRAW *d, int col)
{
	QBrush brush = DP(d)->brush();
		
	col = get_color(d, col, false);
	
	DP(d)->setBrush(QBrush(QColor(col), brush.style()));
	if (DPM(d))
		DPM(d)->setBrush(QBrush(MASK_COLOR(col), brush.style()));
}

static int get_fill_style(GB_DRAW *d)
{
	return DP(d)->brush().style();
}

static void set_fill_style(GB_DRAW *d, int style)
{
	QBrush brush(DP(d)->brush().color(), (Qt::BrushStyle)style);
	DP(d)->setBrush(brush);
	if (DPM(d))
	{
		QBrush brushm(DPM(d)->brush().color(), (Qt::BrushStyle)style);
		DPM(d)->setBrush(brushm);
	}
}

static void get_fill_origin(GB_DRAW *d, int *x, int *y)
{
	if (x) *x = DP(d)->brushOrigin().x();
	if (y) *y = DP(d)->brushOrigin().y();
}

static void set_fill_origin(GB_DRAW *d, int x, int y)
{
	DP(d)->setBrushOrigin(x, y);
	if (DPM(d))
		DPM(d)->setBrushOrigin(x, y);
}

static void draw_rect(GB_DRAW *d, int x, int y, int w, int h)
{
  DP(d)->drawRect(x, y, w, h);
  if (DPM(d))
	  DPM(d)->drawRect(x, y, w, h);
}

static void draw_ellipse(GB_DRAW *d, int x, int y, int w, int h, double start, double end)
{
	if (start == end)
	{
		DP(d)->drawEllipse(x, y, w, h);
		if (DPM(d)) DPM(d)->drawEllipse(x, y, w, h);
	}
	else
	{
		int as, al;
		
		if (end < start)
			end += M_PI * 2 * (1 + (int)((start - end) / 2 / M_PI));
		
		as = (int)(start * 180 / M_PI * 16 + 0.5);
		al = (int)((end - start) * 180 / M_PI * 16 + 0.5);
		
		DP(d)->drawPie(x, y, w, h, as, al);
		if (DPM(d)) DPM(d)->drawPie(x, y, w, h, as, al);
	}
}

static void draw_line(GB_DRAW *d, int x1, int y1, int x2, int y2)
{
	DP(d)->drawLine(x1, y1, x2, y2);
	if (DPM(d)) DPM(d)->drawLine(x1, y1, x2, y2);
}

static void draw_point(GB_DRAW *d, int x, int y)
{
	DP(d)->drawPoint(x, y);
	if (DPM(d)) DPM(d)->drawPoint(x, y);
}

static void draw_picture(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	bool xform;
  QPixmap *p = ((CPICTURE *)picture)->pixmap;

  DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, p->width(), p->height());
	xform = (w != p->width() || h != p->height());

  DP(d)->save();
  
  if (xform)
  {
	  DP(d)->translate(x, y);
	  x = y = 0;
  	DP(d)->scale((double)w / p->width(), (double)h / p->height());
	}

  DP(d)->drawPixmap(x, y, *p, sx, sy, sw, sh);

  DP(d)->restore();

  if (DPM(d))
  {
		DPM(d)->save();
		
		if (xform)
		{
			DPM(d)->translate(x, y);
			x = y = 0;
			DPM(d)->scale((double)w / p->width(), (double)h / p->height());
		}

    if (p->hasAlpha())
    {
      DPM(d)->setRasterOp(Qt::OrROP);
      DPM(d)->drawPixmap(x, y, *(p->mask()), sx, sy, sw, sh);
    }
    else
      DPM(d)->fillRect(x, y, sw, sh, Qt::color1);

    DPM(d)->restore();
  }
}

static void draw_image(GB_DRAW *d, GB_IMAGE image, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	bool xform;
  QImage *img = ((CIMAGE *)image)->image;

  DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, img->width(), img->height());
	xform = (w != img->width() || h != img->height());

  DP(d)->save();
  
  if (xform)
  {
	  DP(d)->translate(x, y);
	  x = y = 0;
  	DP(d)->scale((double)w / img->width(), (double)h / img->height());
	}

  if (DPM(d))
  {
    QPixmap p;

    p.convertFromImage(*img);

    DP(d)->drawImage(x, y, *img, sx, sy, sw, sh);

		DPM(d)->save();
		if (xform)
		{
			DPM(d)->translate(x, y);
			x = y = 0;
			DPM(d)->scale((double)w / img->width(), (double)h / img->height());
		}

    if (p.hasAlpha())
    {
      DPM(d)->setRasterOp(Qt::OrROP);
      DPM(d)->drawPixmap(x, y, *(p.mask()), sx, sy, sw, sh);
    }
    else
      DPM(d)->fillRect(x, y, sw, sh, Qt::color1);

		DPM(d)->restore();
  }
  else
    DP(d)->drawImage(x, y, *img, sx, sy, sw, sh);

	DP(d)->restore();
}

static void draw_tiled_picture(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h)
{
  QPixmap *p = ((CPICTURE *)picture)->pixmap;

  DP(d)->drawTiledPixmap(x, y, w, h, *p, -DP(d)->brushOrigin().x(), -DP(d)->brushOrigin().y());
  if (DPM(d))
  {
    if (p->hasAlpha())
    {
      DPM(d)->save();
      DPM(d)->setRasterOp(Qt::OrROP);
      DPM(d)->drawTiledPixmap(x, y, w, h, *(p->mask()), -DP(d)->brushOrigin().x(), -DP(d)->brushOrigin().y());
      DPM(d)->restore();
    }
    else
      DPM(d)->fillRect(x, y, w, h, Qt::color1);
  }
}

static QStringList text_sl;
static QMemArray<int> text_w;
static int text_line;

static int get_text_width(QPainter *dp, QString &s)
{
  int w, width = 0;
  int i;

  text_sl = QStringList::split('\n', s, true);

  QMemArray<int> tw(text_sl.count());

  for (i = 0; i < (int)text_sl.count(); i++)
  {
    w = dp->fontMetrics().width(text_sl[i]);
    if (w > width) width = w;
    tw[i] = w;
  }

  text_w = tw;

  return width;
}

static int get_text_height(QPainter *dp, QString &s)
{
  text_line = dp->fontMetrics().height();
  return text_line * (1 + s.contains('\n'));
}

static void draw_text(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align)
{
  QString t;
  int xx, ww;
  int tw, th;
  int i;

  t = QString::fromUtf8((const char *)text, len);  
  tw = get_text_width(DP(d), t);
  th = get_text_height(DP(d), t);

	if (w < 0) w = tw;
	if (h < 0) h = th;
	
	if (align == GB_DRAW_ALIGN_DEFAULT)
		align = ALIGN_TOP_NORMAL;

	align = CCONST_alignment(align, ALIGN_TOP_NORMAL, true);

  y += DP(d)->fontMetrics().ascent();

  switch(align & Qt::AlignVertical_Mask)
  {
    case Qt::AlignBottom: y += h - th; break;
    case Qt::AlignVCenter: y += (h - th) / 2; break;
    default: break;
  }

  align = qApp->horizontalAlignment(align);

  for (i = 0; i < (int)text_sl.count(); i++)
  {
    t = text_sl[i];
    ww = text_w[i];

    switch(align)
    {
      case Qt::AlignRight: xx = x + w - ww; break;
      case Qt::AlignHCenter: xx = x + (w - ww) / 2; break;
      default: xx = x; break;
    }

    DP(d)->drawText(xx, y, t);
    if (DPM(d)) DPM(d)->drawText(xx, y, t);

    y += text_line;
  }
}

static void text_size(GB_DRAW *d, char *text, int len, int *w, int *h)
{
	QString s = QString::fromUtf8((const char *)text, len);  
	if (w) *w = get_text_width(DP(d), s);
	if (h) *h = get_text_height(DP(d), s);
}

void DRAW_rich_text(QPainter *p, const QColorGroup &cg, int x, int y, int w, int h, int align, QString &text, QPainter *p2)
{
  QString a;
  QRect clip;
  int tw, th;
  QString t = text;

  switch(qApp->horizontalAlignment(align))
  {
  	case Qt::AlignRight: a = "right"; break;
  	case Qt::AlignHCenter: a = "center"; break;
  	case Qt::AlignJustify: a = "justify"; break;
  }
  
  if (a.length())
  	t = "<div align=\"" + a + "\">" + t + "</div>";
  
	QSimpleRichText rt(t, p->font()); 
  
  if (w > 0)
  	rt.setWidth(w);
  	
  tw = rt.widthUsed();
  th = rt.height();

	if (w < 0) w = tw;
	if (h < 0) h = th;
	
  switch(align & Qt::AlignVertical_Mask)
  {
    case Qt::AlignBottom: y += h - th; break;
    case Qt::AlignVCenter: y += (h - th) / 2; break;
    default: break;
  }

	rt.draw(p, x, y, clip, cg);
	if (p2) rt.draw(p2, x, y, clip, cg);
}

static void draw_rich_text(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align)
{
  QColorGroup cg;

  QString t = QString::fromUtf8((const char *)text, len);
  
	if (align == GB_DRAW_ALIGN_DEFAULT)
		align = ALIGN_TOP_NORMAL;

	align = CCONST_alignment(align, ALIGN_TOP_NORMAL, true);

	cg = QApplication::palette(get_widget(d)).active();

	DRAW_rich_text(DP(d), cg, x, y, w, h, align, t, DPM(d));

//   switch(qApp->horizontalAlignment(align))
//   {
//   	case Qt::AlignRight: a = "right"; break;
//   	case Qt::AlignCenter: a = "center"; break;
//   	case Qt::AlignJustify: a = "justify"; break;
//   }
//   
//   if (a.length())
//   	t = "<div align=\"" + a + "\">" + t + "</div>";
//   
// 	QSimpleRichText rt(t, DP(d)->font()); 
//   
//   if (w > 0)
//   	rt.setWidth(w);
//   	
//   tw = rt.widthUsed();
//   th = rt.height();
// 
// 	if (w < 0) w = tw;
// 	if (h < 0) h = th;
// 	
//   //y += DP(d)->fontMetrics().ascent();
// 
//   switch(align & Qt::AlignVertical_Mask)
//   {
//     case Qt::AlignBottom: y += h - th; break;
//     case Qt::AlignVCenter: y += (h - th) / 2; break;
//     default: break;
//   }
// 
// 	rt.draw(DP(d), x, y, clip, cg);
// 	if (DPM(d)) rt.draw(DPM(d), x, y, clip, cg);
}

static void rich_text_size(GB_DRAW *d, char *text, int len, int sw, int *w, int *h)
{
	QSimpleRichText rt(QString::fromUtf8((const char *)text, len), DP(d)->font());
	
	if (sw > 0)
		rt.setWidth(DP(d), sw);
	
	if (w) *w = rt.widthUsed();
	if (h) *h = rt.height();
}

static void draw_poly(GB_DRAW *d, bool fill, int n, int *points)
{
  int i, j;

  QPointArray p(n);

  for (i = 0, j = 0; i < n; i++, j += 2)
    p.setPoint(i, points[j], points[j + 1]);

  if (fill)
  {
    DP(d)->drawPolygon(p, true);
    if (DPM(d)) DPM(d)->drawPolygon(p, true);
  }
  else
  {
    DP(d)->drawPolyline(p);
    if (DPM(d)) DPM(d)->drawPolyline(p);
  }
}

static void draw_polyline(GB_DRAW *d, int count, int *points)
{
	draw_poly(d, false, count, points);
}

static void draw_polygon(GB_DRAW *d, int count, int *points)
{
	draw_poly(d, true, count, points);
}

static void get_clipping(GB_DRAW *d, int *x, int *y, int *w, int *h)
{
	QRect r = DP(d)->clipRegion(QPainter::CoordPainter).boundingRect();
	
	if (x) *x = r.x();
	if (y) *y = r.y();
	if (w) *w = r.width();
	if (h) *h = r.height();
}

static void set_clipping(GB_DRAW *d, int x, int y, int w, int h)
{
	DP(d)->setClipRect(x, y, w, h, QPainter::CoordPainter);
	if DPM(d) DPM(d)->setClipRect(x, y, w, h, QPainter::CoordPainter);
}

static int is_clipping_enabled(GB_DRAW *d)
{
	return DP(d)->hasClipping();
}

static void set_clipping_enabled(GB_DRAW *d, int enable)
{
	DP(d)->setClipping(enable);
	if (DPM(d)) DPM(d)->setClipping(enable);
}

static QColorGroup get_color_group(int state)
{
	switch (state)
	{
		case GB_DRAW_STATE_DISABLED: return QApplication::palette().disabled();
		default: return QApplication::palette().active();
	}
}

static QColorGroup get_color_group_mask(int state)
{
	static QColorGroup mask_cg(Qt::color1, Qt::color1, Qt::color1, Qt::color1, Qt::color1, Qt::color1, Qt::color1, Qt::color0, Qt::color0);

	return mask_cg;
}

static void style_arrow(GB_DRAW *d, int x, int y, int w, int h, int type, int state)
{
	QStyle::SFlags flags = QStyle::Style_Default;
	QStyle::PrimitiveElement pe;
	
	if (!state)
		flags |= QStyle::Style_Enabled;
	
	switch (type)
	{
		case ALIGN_NORMAL: pe = GB.System.IsRightToLeft() ? QStyle::PE_ArrowLeft : QStyle::PE_ArrowRight; break;
  	case ALIGN_LEFT: pe = QStyle::PE_ArrowLeft; break;
  	case ALIGN_RIGHT: pe = QStyle::PE_ArrowRight; break;
  	case ALIGN_TOP: pe = QStyle::PE_ArrowUp; break;
  	case ALIGN_BOTTOM: pe = QStyle::PE_ArrowDown; break;
  	default:
  		return;
	}

	QRect r(x, y, w, h);
	QApplication::style().drawPrimitive(pe, DP(d), r, get_color_group(state), flags);
	if (DPM(d)) 
	{
    DPM(d)->setRasterOp(Qt::OrROP);
		//qDebug("arrow: %p %p %d", DPM(d), DPM(d)->device(), dynamic_cast<QBitmap *>(DPM(d)->device()) != NULL);
		QApplication::style().drawPrimitive(pe, DPM(d), r, get_color_group_mask(state), flags);
    DPM(d)->setRasterOp(Qt::CopyROP);
	}
}

static void style_check(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	QStyle::SFlags flags = QStyle::Style_Default;
	QRect r(x, y, w, h);
	
	if (!state)
		flags |= QStyle::Style_Enabled;
	
	if (value == 0)
		flags |= QStyle::Style_Off;
	else if (value == 1)
		flags |= QStyle::Style_NoChange;
	else
		flags |= QStyle::Style_On;
	
	QApplication::style().drawPrimitive(QStyle::PE_Indicator, DP(d), r, get_color_group(state), flags);
	if (DPM(d)) 
	{
    DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(QStyle::PE_IndicatorMask, DPM(d), r, get_color_group_mask(state), flags);
		DPM(d)->setRasterOp(Qt::CopyROP);
	}
}

static void style_option(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	QRect r(x, y, w, h);
	QStyle::SFlags flags = QStyle::Style_Default;
	
	if (!state)
		flags |= QStyle::Style_Enabled;
	
	if (!value)
		flags |= QStyle::Style_Off;
	else
		flags |= QStyle::Style_On;
	
	QApplication::style().drawPrimitive(QStyle::PE_ExclusiveIndicator, DP(d), r, get_color_group(state), flags);
	if (DPM(d)) 
	{
		DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(QStyle::PE_ExclusiveIndicatorMask, DPM(d), r, get_color_group_mask(state), flags);
		DPM(d)->setRasterOp(Qt::CopyROP);
	}
}

static void style_separator(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	QRect r(x, y, w, h);
	QStyle::SFlags flags = QStyle::Style_Default;
	
	if (!state)
		flags |= QStyle::Style_Enabled;
	
	if (!vertical)
		flags |= QStyle::Style_Horizontal;
	
	QApplication::style().drawPrimitive(QStyle::PE_DockWindowSeparator, DP(d), r, get_color_group(0), flags);
	if (DPM(d)) 
	{
		DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(QStyle::PE_DockWindowSeparator, DPM(d), r, get_color_group_mask(0), flags);
		DPM(d)->setRasterOp(Qt::CopyROP);
	}
}

static void style_focus(GB_DRAW *d, int x, int y, int w, int h)
{
	QRect r(x, y, w, h);
	QApplication::style().drawPrimitive(QStyle::PE_FocusRect, DP(d), r, get_color_group(0));
	if (DPM(d)) 
	{
		DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(QStyle::PE_FocusRect, DPM(d), r, get_color_group_mask(0));
		DPM(d)->setRasterOp(Qt::CopyROP);
	}	
}
			
static void style_button(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	QRect r(x, y, w, h);
	QStyle::SFlags flags = QStyle::Style_Default;
	
	if (!state)
		flags |= QStyle::Style_Enabled;
	
	if (!value)
		flags |= QStyle::Style_Off;
	else
		flags |= QStyle::Style_On;
	
	QApplication::style().drawPrimitive(QStyle::PE_ButtonCommand, DP(d), r, get_color_group(state), flags);
	if (DPM(d)) 
	{
		DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(QStyle::PE_ButtonCommand, DPM(d), r, get_color_group_mask(state), flags);
		DPM(d)->setRasterOp(Qt::CopyROP);
	}
}
			
static void style_panel(GB_DRAW *d, int x, int y, int w, int h, int border, int state)
{
	QStyle::PrimitiveElement pe;
	QRect r(x, y, w, h);
	QStyle::SFlags flags = QStyle::Style_Default;
	QStyleOption opt(2, 0);	
	
	if (!state)
		flags |= QStyle::Style_Enabled;
	
	if (border == BORDER_NONE)
		return;
		
	if (border == BORDER_PLAIN)
	{
		qDrawPlainRect(DP(d), r, get_color_group(state).foreground(), 1);
		if DPM(d) qDrawPlainRect(DPM(d), r, get_color_group_mask(state).foreground(), 1);
		return;
	}
	
	if (border == BORDER_ETCHED)
	{
		pe = QStyle::PE_GroupBoxFrame;
	}
	else
	{
		if (border == BORDER_RAISED)
			flags |= QStyle::Style_Raised;
		else if (border == BORDER_SUNKEN)
			flags |= QStyle::Style_Sunken;
		
		pe = QStyle::PE_Panel;
	}
	
	QApplication::style().drawPrimitive(pe, DP(d), r, get_color_group(state), flags, opt);
	if (DPM(d)) 
	{
		DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(pe, DPM(d), r, get_color_group_mask(state), flags, opt);
		DPM(d)->setRasterOp(Qt::CopyROP);
	}
}
			
static void style_handle(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	QRect r(0, 0, w, h);
	QStyle::SFlags flags = QStyle::Style_Default;
	
	if (!state)
		flags |= QStyle::Style_Enabled;
		
	if (!vertical)
		flags |= QStyle::Style_Horizontal;
	
	// Workaround a style bug
	DP(d)->translate(x, y);
	QApplication::style().drawPrimitive(QStyle::PE_Splitter, DP(d), r, get_color_group(state), flags);
	DP(d)->translate(-x, -y);
	if (DPM(d)) 
	{
		DPM(d)->translate(x, y);
		DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style().drawPrimitive(QStyle::PE_Splitter, DPM(d), r, get_color_group_mask(state), flags);	
		DPM(d)->setRasterOp(Qt::CopyROP);
		DPM(d)->translate(-x, -y);
	}
}


GB_DRAW_DESC DRAW_Interface = {
	sizeof(GB_DRAW_EXTRA),
	begin,
	end,
	save,
	restore,
	get_background,
	set_background,
	get_foreground,
	set_foreground,
	get_font,
	set_font,
	is_inverted,
	set_inverted,
	is_transparent,
	set_transparent,
	{
		get_line_width,
		set_line_width,
		get_line_style,
		set_line_style
	},
	{
		get_fill_color,
		set_fill_color,
		get_fill_style,
		set_fill_style,
		get_fill_origin,
		set_fill_origin
	},
	{
		draw_rect,
		draw_ellipse,
		draw_line,
		draw_point,
		draw_picture,
		draw_image,
		draw_tiled_picture,
		draw_text,
		text_size,
		draw_polyline,
		draw_polygon,
		draw_rich_text,
		rich_text_size
	},
	{
		get_clipping,
		set_clipping,
		is_clipping_enabled,
		set_clipping_enabled
	},
	{
		style_arrow,
		style_check,
		style_option,
		style_separator,
		style_focus,
		style_button,
		style_panel,
		style_handle
	}
};

void DRAW_begin(void *device)
{
	DRAW.Begin(device);
}

void DRAW_end()
{
	DRAW.End();
}

QPainter *DRAW_get_current()
{
	GB_DRAW *d = DRAW.GetCurrent();
	return d ? DP(d) : NULL;
}

