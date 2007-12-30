/***************************************************************************

  CDraw.cpp

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qapplication.h>
#include <qpaintdevicemetrics.h>
#include <qpicture.h>
#include <qpixmap.h>
#include <qbitmap.h>

#include "gambas.h"

#include "CFont.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CImage.h"
#include "CDrawing.h"
#include "CDrawingArea.h"
#include "CPrinter.h"
#include "CColor.h"
#include "CDraw.h"

#define DRAW_STACK_MAX 8

typedef
  struct {
    QPainter *p;
    QPainter *pm;
    void *device;
    QBitmap *mask;
    int w;
    int h;
    int dpi;
    }
  CDRAW;

#define THIS  (draw_current)
#define DP    (THIS->p)
#define DPM   (THIS->pm)

static CDRAW draw_stack[DRAW_STACK_MAX];
static CDRAW *draw_current = 0;

//static CFONT draw_font;
//static CWIDGET *_device = 0;
//static QPainter *DP = NULL;
//static int draw_mode = DRAW_ON_NOTHING;
//static bool delete_painter = false;

#define COLOR_TO_INT(color) ((color).rgb() ^ 0xFF000000)
#define MASK_COLOR(col) ((col & 0xFF000000) ? Qt::color0 : Qt::color1)

void DRAW_begin(void *device, QPainter *p, int w, int h, int dpi)
{
  if (THIS >= &draw_stack[DRAW_STACK_MAX - 1])
  {
    GB.Error("Too many nested drawings");
    return;
  }

  if (THIS == 0)
    THIS = draw_stack;
  else
    THIS++;

  THIS->p = p;
  THIS->pm = 0;
  THIS->device = device;
  THIS->mask = 0;
  THIS->w = w;
  THIS->h = h;
  
	if (dpi)
		THIS->dpi = dpi;
	else
		THIS->dpi = QPaintDevice::x11AppDpiY();

  if (device)
    GB.Ref(device);

  //qDebug("DRAW_begin: THIS = %p", THIS);
}


void DRAW_end(void)
{
  void *device;

  if (!THIS)
    return;

  //if (THIS->mode != DRAW_ON_ITEM)
  //  delete DP;

  device = THIS->device;
  delete DP;

  if (GB.Is(device, CLASS_Picture))
  {
    if (DPM)
    {
      ((CPICTURE *)device)->pixmap->setMask(*THIS->mask);
      delete DPM;
      delete THIS->mask;
    }
  }
  else if (GB.Is(device, CLASS_DrawingArea))
  {
    MyDrawingArea *wid =  (MyDrawingArea *)(((CWIDGET *)device)->widget);

    if (wid->isCached())
      wid->refreshBackground();
  
  	wid->drawn--;
	}

  if (device)
    GB.Unref(&device);

  if (THIS == draw_stack)
    THIS = 0;
  else
    THIS--;

  //qDebug("DRAW_end: THIS = %p", THIS);
}


int DRAW_status(void)
{
  if (!THIS)
    return -1;
  else
    return (THIS - draw_stack);
}


void DRAW_restore(int status)
{
  while (DRAW_status() != status)
    DRAW_end();
}

static QWidget *get_widget(CDRAW *_object)
{
	if (!GB.Is(THIS->device, CLASS_Control))
		return 0;
		
	return QWIDGET(THIS->device);
}


static bool check_painter(void)
{
  if (!THIS)
  {
    GB.Error("No current device");
    return true;
  }
  else
    return false;
}

#define CHECK_PAINTER() if (check_painter()) return

/*
static bool check_device(void)
{
  if (!DP)
  {
    GB.Error("Unsupported command");
    return true;
  }
  else
    return false;
}

#define CHECK_DEVICE() if (check_device()) return
*/

void DRAW_set_font(QFont &f)
{
  CHECK_PAINTER();

  DP->setFont(f);
  if (DPM)
    DPM->setFont(f);
}


bool DRAW_must_resize_font(void)
{
  return (THIS->device != CLASS_Printer);
}



BEGIN_METHOD_VOID(CDRAW_init)

END_METHOD


BEGIN_METHOD_VOID(CDRAW_exit)

  DRAW_restore(-1);

END_METHOD


BEGIN_METHOD(CDRAW_begin, GB_OBJECT device)

  void *device = VARG(device);

  if (GB.CheckObject(device))
    return;

  if (GB.Is(device, CLASS_Window))
  {
    MyMainWindow *win = (MyMainWindow *)((CWIDGET *)device)->widget;
    //win->paintUnclip(true);
    DRAW_begin(device, new QPainter(win, true), win->width(), win->height());
  }
  else if (GB.Is(device, CLASS_Picture))
  {
    CPICTURE *pict = (CPICTURE *)device;

		if (pict->pixmap->isNull())
		{
			GB.Error("Bad picture");
			return;
		}

    DRAW_begin(device, new QPainter(pict->pixmap), pict->pixmap->width(), pict->pixmap->height());

    if (pict->pixmap->mask())
    {
      QPen pen;
      QBrush brush;

      THIS->mask = new QBitmap(*pict->pixmap->mask());
      THIS->pm = new QPainter(THIS->mask);

      pen = THIS->p->pen();
      THIS->pm->setPen(QPen(Qt::color1, pen.width(), pen.style()));
      brush = THIS->p->brush();
      THIS->pm->setBrush(QBrush(Qt::color1, brush.style()));
    }
  }
  /*else if (GB.Is(device, CLASS_Image))
  {
    CPICTURE *img = (CIMAGE *)device;

		if (img->image->isNull())
		{
			GB.Error("Bad image");
			return;
		}

    DRAW_begin(device, NULL, img->image->width(), img->image->height());
  }*/
  else if (GB.Is(device, CLASS_Drawing))
  {
    CDRAWING *drawing = (CDRAWING *)device;
    DRAW_begin(device, new QPainter(drawing->picture), drawing->picture->boundingRect().width(), drawing->picture->boundingRect().height());
  }
  else if (GB.Is(device, CLASS_DrawingArea))
  {
    MyDrawingArea *wid = (MyDrawingArea *)(((CWIDGET *)device)->widget);

    if (wid->isCached())
      DRAW_begin(device, new QPainter(wid->background(), wid), wid->background()->width(), wid->background()->height());
    else
      DRAW_begin(device, new QPainter(wid, wid), wid->width(), wid->height());
      
		wid->drawn++;
  }
  else if (device == CLASS_Printer)
  {
    CPRINTER_init();
    QPrinter *printer = CPRINTER_printer;
		QPaintDeviceMetrics pdm(printer);
    DRAW_begin(device, new QPainter(printer), pdm.width(), pdm.height(), printer->resolution());
  }
  else
    goto _ERROR;

  return;

_ERROR:

  GB.Error("Bad device");

END_METHOD


BEGIN_METHOD_VOID(CDRAW_end)

  DRAW_end();

END_METHOD


BEGIN_PROPERTY(CDRAW_device)

	
  CHECK_PAINTER();

	GB.ReturnObject(THIS->device);

END_PROPERTY

BEGIN_PROPERTY(CDRAW_width)

  CHECK_PAINTER();

	GB.ReturnInteger(THIS->w);

END_PROPERTY

BEGIN_PROPERTY(CDRAW_height)

  CHECK_PAINTER();

	GB.ReturnInteger(THIS->h);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_resolution)

  CHECK_PAINTER();

	GB.ReturnInteger(THIS->dpi);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_background)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(COLOR_TO_INT(DP->backgroundColor()));
  else
  {
  	QWidget *wid = get_widget(THIS);
    int col = VPROP(GB_INTEGER);
    
    if (col == COLOR_DEFAULT)
    {
    	if (wid)
    		col = wid->paletteBackgroundColor().rgb() & 0xFFFFFF;
			else
				col = 0xFFFFFF;
    }
    
    DP->setBackgroundColor(QColor(col));
    if (DPM)
      DPM->setBackgroundColor(MASK_COLOR(col));
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_transparent)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnBoolean(DP->backgroundMode() == Qt::TransparentMode);
  else
  {
    DP->setBackgroundMode(VPROP(GB_BOOLEAN) ? Qt::TransparentMode : Qt::OpaqueMode);
    if (DPM)
      DPM->setBackgroundMode(VPROP(GB_BOOLEAN) ? Qt::TransparentMode : Qt::OpaqueMode);
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_invert)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnBoolean(DP->rasterOp() == Qt::XorROP);
  else
    DP->setRasterOp(VPROP(GB_BOOLEAN) ? Qt::XorROP : Qt::CopyROP);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_foreground)

  QPen pen;

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(COLOR_TO_INT(DP->pen().color()));
  else
  {
  	QWidget *wid = get_widget(THIS);
    int col = VPROP(GB_INTEGER);
    
    if (col == COLOR_DEFAULT)
    {
    	if (wid)
    		col = wid->paletteForegroundColor().rgb() & 0xFFFFFF;
			else
				col = 0x000000;
    }
    
    pen = DP->pen();
    DP->setPen(QPen(QColor(col), pen.width(), pen.style()));
    if (DPM)
      DPM->setPen(QPen(MASK_COLOR(col), pen.width(), pen.style()));
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_line_width)

  QPen pen;

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(DP->pen().width());
  else
  {
    pen = DP->pen();
    DP->setPen(QPen(pen.color(), VPROP(GB_INTEGER), pen.style()));
    if (DPM)
      DPM->setPen(QPen(DPM->pen().color(), VPROP(GB_INTEGER), pen.style()));
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_line_style)

  QPen pen;

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(DP->pen().style());
  else
  {
    pen = DP->pen();
    DP->setPen(QPen(pen.color(), pen.width(), (Qt::PenStyle)VPROP(GB_INTEGER)));
    if (DPM)
      DPM->setPen(QPen(DPM->pen().color(), pen.width(), (Qt::PenStyle)VPROP(GB_INTEGER)));
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_color)

  QBrush brush;

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(COLOR_TO_INT(DP->brush().color()));
  else
  {
  	QWidget *wid = get_widget(THIS);
    int col = VPROP(GB_INTEGER);
    
    if (col == COLOR_DEFAULT)
    {
    	if (wid)
    		col = wid->paletteBackgroundColor().rgb() & 0xFFFFFF;
			else
				col = 0xFFFFFF;
    }
    
    brush = DP->brush();
    DP->setBrush(QBrush(QColor(col), brush.style()));
    if (DPM)
      DPM->setBrush(QBrush(MASK_COLOR(col), brush.style()));
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_style)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(DP->brush().style());
  else
  {
    //qDebug("CDRAW_fill_style: THIS = %p  DP = %p  DPM = %p", THIS, DP, DPM);
    QBrush brush(DP->brush().color(), (Qt::BrushStyle)VPROP(GB_INTEGER));
    DP->setBrush(brush);
    if (DPM)
    {
      QBrush brushm(DPM->brush().color(), (Qt::BrushStyle)VPROP(GB_INTEGER));
      DPM->setBrush(brushm);
    }
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_x)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(DP->brushOrigin().x());
  else
  {
    DP->setBrushOrigin(VPROP(GB_INTEGER), DP->brushOrigin().y());
    if (DPM)
      DPM->setBrushOrigin(VPROP(GB_INTEGER), DPM->brushOrigin().y());
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_y)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnInteger(DP->brushOrigin().y());
  else
  {
    DP->setBrushOrigin(DP->brushOrigin().x(), VPROP(GB_INTEGER));
    if (DPM)
      DPM->setBrushOrigin(DPM->brushOrigin().x(), VPROP(GB_INTEGER));
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAW_font)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnObject(CFONT_create(DP->font(), CFONT_DRAW));
  else
  {
    CFONT *font = (CFONT *)VPROP(GB_OBJECT);
    DRAW_set_font(*font->font);
  }

END_PROPERTY


BEGIN_METHOD(CDRAW_rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CHECK_PAINTER();

  DP->drawRect(VARG(x), VARG(y), VARG(w), VARG(h));
  if (DPM)
    DPM->drawRect(VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD


BEGIN_METHOD(CDRAW_round_rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_FLOAT round)

	int r;

  CHECK_PAINTER();

	if (MISSING(round))
		r = 25;
	else
		r = (int)(VARG(round) * 100);

  DP->drawRoundRect(VARG(x), VARG(y), VARG(w), VARG(h), r, r);
  if (DPM)
    DPM->drawRoundRect(VARG(x), VARG(y), VARG(w), VARG(h), r, r);

END_METHOD


BEGIN_METHOD(CDRAW_ellipse, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_FLOAT start; GB_FLOAT len)

  CHECK_PAINTER();

  if (MISSING(start) || MISSING(len))
  {
    DP->drawEllipse(VARG(x), VARG(y), VARG(w), VARG(h));
    if (DPM)
      DPM->drawEllipse(VARG(x), VARG(y), VARG(w), VARG(h));
  }
  else
  {
    DP->drawPie(VARG(x), VARG(y), VARG(w), VARG(h), (long)(VARG(start) * 16 + 0.5), (long)(VARG(len) * 16 + 0.5));
    if (DPM)
      DPM->drawPie(VARG(x), VARG(y), VARG(w), VARG(h), (long)(VARG(start) * 16 + 0.5), (long)(VARG(len) * 16 + 0.5));
  }

END_METHOD


BEGIN_METHOD(CDRAW_line, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2)

  CHECK_PAINTER();

  DP->drawLine(VARG(x1), VARG(y1), VARG(x2), VARG(y2));
  if (DPM)
    DPM->drawLine(VARG(x1), VARG(y1), VARG(x2), VARG(y2));

END_METHOD


BEGIN_METHOD(CDRAW_point, GB_INTEGER x; GB_INTEGER y)

  CHECK_PAINTER();

  DP->drawPoint(VARG(x), VARG(y));
  if (DPM)
    DPM->drawPoint(VARG(x), VARG(y));

END_METHOD

#define NORMALIZE(x, y, sx, sy, sw, sh, object) \
  if (sx >= (object)->width() || sy >= (object)->height() || sw <= 0 || sh <= 0) \
    return; \
  if (sx < 0) x -= sx, sx = 0; \
  if (sy < 0) y -= sy, sy = 0; \
  if (sw > ((object)->width() - sx)) \
    sw = ((object)->width() - sx); \
  if (sh > ((object)->height() - sy)) \
    sh = ((object)->height() - sy);


BEGIN_METHOD(CDRAW_picture, GB_OBJECT pict; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  long x, y, w, h, sx, sy, sw, sh;
  CPICTURE *picture = (CPICTURE *)VARG(pict);
  QPixmap *p;

  CHECK_PAINTER();

  if (GB.CheckObject(picture))
    return;

  p = picture->pixmap;

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, p->width());
  sh = VARGOPT(sh, p->height());

  NORMALIZE(x, y, sx, sy, sw, sh, p);

  DP->save();
  if ((w > 0) && (h > 0))
  {
	  DP->translate(x, y);
	  x = y = 0;
  	DP->scale((double)w / p->width(), (double)h / p->height());
	}

  DP->drawPixmap(x, y, *p, sx, sy, sw, sh);

  DP->restore();

  if (DPM)
  {
		DPM->save();
		if ((w > 0) && (h > 0))
		{
			DPM->translate(x, y);
			x = y = 0;
			DPM->scale((double)w / p->width(), (double)h / p->height());
		}

    if (p->hasAlpha())
    {
      DPM->setRasterOp(Qt::OrROP);
      DPM->drawPixmap(x, y, *(p->mask()), sx, sy, sw, sh);
    }
    else
      DPM->fillRect(x, y, sw, sh, Qt::color1);

    DPM->restore();
  }

END_METHOD


BEGIN_METHOD(CDRAW_image, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  //static bool warn = false;
  long x, y, w, h, sx, sy, sw, sh;
  CIMAGE *image = (CIMAGE *)VARG(img);
  QImage *img;

  CHECK_PAINTER();

  if (GB.CheckObject(image))
    return;

	img = image->image;

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, img->width());
  sh = VARGOPT(sh, img->height());

  NORMALIZE(x, y, sx, sy, sw, sh, img);

  DP->save();
  if ((w > 0) && (h > 0))
  {
	  DP->translate(x, y);
	  x = y = 0;
  	DP->scale((double)w / img->width(), (double)h / img->height());
	}

  if (DPM)
  {
    QPixmap p;

    p.convertFromImage(*img);

    DP->drawImage(x, y, *img, sx, sy, sw, sh);

		DPM->save();
		if ((w > 0) && (h > 0))
		{
			DPM->translate(x, y);
			x = y = 0;
			DPM->scale((double)w / img->width(), (double)h / img->height());
		}

    if (p.hasAlpha())
    {
      DPM->setRasterOp(Qt::OrROP);
      DPM->drawPixmap(x, y, *(p.mask()), sx, sy, sw, sh);
    }
    else
      DPM->fillRect(x, y, sw, sh, Qt::color1);

		DPM->restore();
  }
  else
    DP->drawImage(x, y, *img, sx, sy, sw, sh);

	DP->restore();

END_METHOD

/*
static QColor blend_on_white(QRgb a)
{
	// c0 = ca x 1 + cb x (1 - aa)
	// C0 = c0 x a0 = c0
	// C0 = Ca x aa + Cb x ab x (1 - aa)
	// C0 = Ca x aa + (FF,FF,FF) x (1 - aa)

	QRgb b = qRgba(255, 255, 255, 255);
	QColor c;

	if (qAlpha(a) == 255)
		c.setRgb(qRed(a), qGreen(a), qBlue(a));
	else
	{
		c.setRgb(
			qRed(a) * qAlpha(a) / 255 + qRed(b) * qAlpha(b) / 255 * (255 - qAlpha(a)) / 255,
			qGreen(a) * qAlpha(a) / 255 + qGreen(b) * qAlpha(b) / 255 * (255 - qAlpha(a)) / 255,
			qBlue(a) * qAlpha(a) / 255 + qBlue(b) * qAlpha(b) / 255 * (255 - qAlpha(a)) / 255
			);
	}

	return c;
}
*/

BEGIN_METHOD(CDRAW_zoom, GB_OBJECT img; GB_INTEGER zoom; GB_INTEGER x; GB_INTEGER y; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  long x, y, sx, sy, sw, sh;
  CIMAGE *img = (CIMAGE *)VARG(img);
  uint col;
  long xr, yr, zoom, size, i, j;
  bool border;

  CHECK_PAINTER();

  if (GB.CheckObject(img))
    return;

  zoom = VARG(zoom);
  if (zoom < 1)
  {
    GB.Error("Bad zoom factor");
    return;
  }

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, img->image->width());
  sh = VARGOPT(sh, img->image->height());

  NORMALIZE(x, y, sx, sy, sw, sh, img->image);

  border = DP->pen().style() != Qt::NoPen;

  if (zoom == 1 && !border)
  {
    DP->drawImage(x, y, *(img->image), sx, sy, sw, sh);
    if (DPM)
      DPM->drawImage(x, y, *(img->image), sx, sy, sw, sh, Qt::ThresholdDither | Qt::MonoOnly);
  }
  else
  {
    QBrush draw(Qt::black, Qt::SolidPattern);

    DP->save();
    //DP->setPen(QPen(Qt::NoPen));
    size = zoom;
    if (border) size++;

    for (j = sy, yr = y; j < (sy + sh); j++, yr += zoom)
      for (i = sx, xr = x; i < (sx + sw); i++, xr += zoom)
      {
        //if (!img->image->valid(i, j))
        //  continue;
        col = img->image->pixel(i, j);
        if ((col >> 24) != 0xFF)
          continue;
        //draw.setColor(blend_on_white((QRgb)col));
        draw.setColor(QColor(col));
        DP->setBrush(draw);
        DP->drawRect(xr, yr, size, size);
      }

    DP->restore();

    if (DPM)
    {
      DPM->save();
      //DPM->setPen(QPen(Qt::NoPen));

      for (j = sy, yr = y; j < (sy + sh); j++, yr += zoom)
        for (i = sx, xr = x; i < (sx + sw); i++, xr += zoom)
        {
          //if (!img->image->valid(i, j))
          //  continue;
          col = img->image->pixel(i, j);
          if ((col >> 24) != 0xFF)
            continue;
          draw.setColor(QColor(col));
          DPM->setBrush(draw);
          DPM->drawRect(xr, yr, size, size);
        }

      DPM->restore();
    }
  }

END_METHOD


BEGIN_METHOD(CDRAW_drawing, GB_OBJECT drawing; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  static bool warn = false;
  long x, y, w, h;
  long sx, sy, sw, sh;
  CDRAWING *drawing = (CDRAWING *)VARG(drawing);
  QPicture *pic;

  CHECK_PAINTER();

  if (GB.CheckObject(drawing))
    return;

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, -1);
  sh = VARGOPT(sh, -1);

  pic = drawing->picture;

  DP->save();
  if ((sw > 0) && (sh > 0))
    DP->setClipRect(x + sx, y + sy, sw, sh);
  DP->translate(x, y);
  if ((w > 0) && (h > 0))
  	DP->scale((double)w / pic->boundingRect().width(), (double)h / pic->boundingRect().height());
  DP->drawPicture(0, 0, *pic);
  DP->restore();

  if (DPM)
  {
		DPM->save();
    if ((sw > 0) && (sh > 0))
      DPM->setClipRect(x + sx, y + sy, sw, sh);
		DPM->translate(x, y);
		if ((w > 0) && (h > 0))
			DPM->scale((double)w / pic->boundingRect().width(), (double)h / pic->boundingRect().height());
		DPM->drawPicture(0, 0, *pic);
		DPM->restore();
    if (!warn)
    {
      qDebug("WARNING: Draw.Drawing() on transparent devices partially implemented.");
      warn = true;
    }
  }

END_METHOD


BEGIN_METHOD(CDRAW_tile, GB_OBJECT pict; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CPICTURE *picture = (CPICTURE *)VARG(pict);
  QPixmap *p;

  CHECK_PAINTER();
  if (GB.CheckObject(picture))
    return;

  p = picture->pixmap;

  DP->drawTiledPixmap(VARG(x), VARG(y), VARG(w), VARG(h), *p, -DP->brushOrigin().x(), -DP->brushOrigin().y());
  if (DPM)
  {
    if (p->hasAlpha())
    {
      DPM->save();
      DPM->setRasterOp(Qt::OrROP);
      DPM->drawTiledPixmap(VARG(x), VARG(y), VARG(w), VARG(h), *(p->mask()), -DP->brushOrigin().x(), -DP->brushOrigin().y());
      DPM->restore();
    }
    else
      DPM->fillRect(VARG(x), VARG(y), VARG(w), VARG(h), Qt::color1);
  }

END_METHOD

static QStringList text_sl;
static QMemArray<int> text_w;
static int text_line;

static int text_width(QPainter *dp, QString &s)
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


static int text_height(QPainter *dp, QString &s)
{
  text_line = dp->fontMetrics().height();
  return text_line * (1 + s.contains('\n'));
}


BEGIN_METHOD(CDRAW_text, GB_STRING text; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER align)

  //static bool warn = false;
  QString text;
  int x, y, w, h, align, xx, ww;
  int tw, th;
  int i;

  CHECK_PAINTER();

  text = QSTRING_ARG(text);
  x = VARG(x);
  y = VARG(y);

  /*if (MISSING(w) && MISSING(h) && MISSING(align))
  {
    DP->drawText(x, y, text);
    return;
  }*/

  tw = text_width(DP, text);
  th = text_height(DP, text);

  w = VARGOPT(w, tw);
  h = VARGOPT(h, th);
  align = VARGOPT(align, Qt::AlignAuto + Qt::AlignTop);

  y += DP->fontMetrics().ascent();

  switch(align & Qt::AlignVertical_Mask)
  {
    case Qt::AlignBottom: y += h - th; break;
    case Qt::AlignVCenter: y += (h - th) / 2; break;
    default: break;
  }

  align = qApp->horizontalAlignment(align);

  for (i = 0; i < (int)text_sl.count(); i++)
  {
    text = text_sl[i];
    ww = text_w[i];

    switch(align)
    {
      case Qt::AlignRight: xx = x + w - ww; break;
      case Qt::AlignHCenter: xx = x + (w - ww) / 2; break;
      default: xx = x; break;
    }

    DP->drawText(xx, y, text);
    if (DPM)
      DPM->drawText(xx, y, text);
    /*
    if (DPM && !warn)
    {
      qDebug("WARNING: Draw.Text() on transparent devices not implemented.");
      warn = true;
    }
    */

    y += text_line;
  }

END_METHOD


BEGIN_METHOD(CDRAW_rect_text, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_STRING text; GB_INTEGER align)

  //static bool warn = false;
  int align;

  CHECK_PAINTER();

  align = Qt::AlignAuto | Qt::AlignVCenter;
  if (!MISSING(align))
    align = VARG(align) & ALIGN_MASK;

  align |= Qt::WordBreak;

  DP->drawText(VARG(x), VARG(y), VARG(w), VARG(h), align, QSTRING_ARG(text));
  if (DPM)
    DPM->drawText(VARG(x), VARG(y), VARG(w), VARG(h), align, QSTRING_ARG(text));
  /*
  if (DPM && !warn)
  {
    qDebug("WARNING: Draw.RectText() on transparent devices not implemented.");
    warn = true;
  }
  */

END_METHOD


BEGIN_METHOD(CDRAW_text_width, GB_STRING text)

  QString s;

  CHECK_PAINTER();

  s = QSTRING_ARG(text);
  GB.ReturnInteger(text_width(DP, s));

END_METHOD


BEGIN_METHOD(CDRAW_text_height, GB_STRING text)

  QString s;

  CHECK_PAINTER();

  s = QSTRING_ARG(text);
  GB.ReturnInteger(text_height(DP, s));

END_METHOD


static void draw_poly(bool fill, GB_ARRAY points)
{
  uint i, j, n;
  int x, y;

  CHECK_PAINTER();

  n = GB.Array.Count(points) / 2;
  if (n == 0)
    return;

  QPointArray p(n);

  for (i = 0, j = 0; i < n; i++, j += 2)
  {
    x = *((int *)GB.Array.Get(points, j));
    y = *((int *)GB.Array.Get(points, j + 1));
    p.setPoint(i, x, y);
  }

  if (fill)
  {
    DP->drawPolygon(p, true);
    if (DPM)
      DPM->drawPolygon(p, true);
  }
  else
  {
    DP->drawPolyline(p);
    if (DPM)
      DPM->drawPolyline(p);
  }
}


BEGIN_METHOD(CDRAW_polyline, GB_OBJECT points)

  draw_poly(false, VARG(points));

END_METHOD


BEGIN_METHOD(CDRAW_polygon, GB_OBJECT points)

  draw_poly(true, VARG(points));

END_METHOD


BEGIN_PROPERTY(CDRAW_clip_x)

  CHECK_PAINTER();

  GB.ReturnInteger(DP->clipRegion(QPainter::CoordPainter).boundingRect().x());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_y)

  CHECK_PAINTER();

  GB.ReturnInteger(DP->clipRegion(QPainter::CoordPainter).boundingRect().y());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_w)

  CHECK_PAINTER();

  GB.ReturnInteger(DP->clipRegion(QPainter::CoordPainter).boundingRect().width());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_h)

  CHECK_PAINTER();

  GB.ReturnInteger(DP->clipRegion(QPainter::CoordPainter).boundingRect().height());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_enabled)

  CHECK_PAINTER();

  if (READ_PROPERTY)
    GB.ReturnBoolean(DP->hasClipping());
  else
  {
    DP->setClipping(VPROP(GB_BOOLEAN));
    if (DPM)
      DPM->setClipping(VPROP(GB_BOOLEAN));
  }

END_PROPERTY


BEGIN_METHOD(CDRAW_clip, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CHECK_PAINTER();

  DP->setClipRect(VARG(x), VARG(y), VARG(w), VARG(h), QPainter::CoordPainter);
  if (DPM)
    DPM->setClipRect(VARG(x), VARG(y), VARG(w), VARG(h), QPainter::CoordPainter);

END_PROPERTY


BEGIN_METHOD_VOID(CDRAW_reset)

	DP->resetXForm();
	if (DPM)
		DPM->resetXForm();

END_METHOD


BEGIN_METHOD_VOID(CDRAW_push)

	DP->save();
	if (DPM)
		DPM->save();

END_METHOD


BEGIN_METHOD_VOID(CDRAW_pop)

	DP->restore();
	if (DPM)
		DPM->restore();

END_METHOD


BEGIN_METHOD(CDRAW_translate, GB_FLOAT dx; GB_FLOAT dy)

	double dx = VARG(dx);
	double dy = VARG(dy);

	DP->translate(dx, dy);
	if (DPM)
		DPM->translate(dx, dy);

END_METHOD


BEGIN_METHOD(CDRAW_scale, GB_FLOAT dx; GB_FLOAT dy)

	double dx = VARG(dx);
	double dy = VARG(dy);

	DP->scale(dx, dy);
	if (DPM)
		DPM->scale(dx, dy);

END_METHOD


BEGIN_METHOD(CDRAW_rotate, GB_FLOAT a)

	double a = VARG(a);

	DP->rotate(a);
	if (DPM)
		DPM->rotate(a);

END_METHOD


#include "CDraw_desc.h"
