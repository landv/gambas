/***************************************************************************

	CDraw.cpp

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
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qdrawutil.h>
//Added by qt3to4:
#include <QStyle>
#include <QStyleOption>
#include <QVector>
#include <QTextDocument>
#include <QColor>
#include <QPen>
#include <QBrush>

#ifndef NO_X_WINDOW
#include <QX11Info>
#endif

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

#define EXTRA(d) ((GB_DRAW_EXTRA *)(d->extra))
#define DP(d) (EXTRA(d)->p)
#define DPM(d) (EXTRA(d)->pm)

#define COLOR_TO_INT(color) ((color).rgba() ^ 0xFF000000)
#define MASK_COLOR(col) ((col & 0xFF000000) ? Qt::color0 : Qt::color1)

DRAW_INTERFACE DRAW EXPORT;

static void set_background(GB_DRAW *d, int col);
static void set_foreground(GB_DRAW *d, int col);
static void set_fill_color(GB_DRAW *d, int col);

void DRAW_init()
{
	GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
}

static bool init_drawing(GB_DRAW *d, QPaintDevice *device, int w, int h, int dpi = 0)
{
	if (device->paintingActive())
	{
		GB.Error("Device already being painted");
		return true;
	}
	
	EXTRA(d)->p = new QPainter(device);
	EXTRA(d)->pm = 0;
	EXTRA(d)->mask = 0;
	EXTRA(d)->fg = COLOR_DEFAULT;
	EXTRA(d)->fillColor = COLOR_DEFAULT;
	d->width = w;
	d->height = h;
	
	if (dpi)
		d->resolution = dpi;
	else
		#ifdef NO_X_WINDOW
			d->resolution = 72;
		#else
			d->resolution = QX11Info::appDpiY();
		#endif
	
	return FALSE;
}

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
				col = wid->palette().color(QPalette::Window).rgb() & 0xFFFFFF;
			else
				col = wid->palette().color(QPalette::WindowText).rgb() & 0xFFFFFF;
		}
		else
			col = bg ? 0xFFFFFF : 0x000000;
	}
	
	return col;
}

static Qt::Alignment get_horizontal_alignment(Qt::Alignment align)
{
	align &= Qt::AlignHorizontal_Mask;
	switch (align)
	{
		case Qt::AlignLeft:
			if (QApplication::isRightToLeft())
				return Qt::AlignRight;
			break;
			
		case Qt::AlignRight:
			if (QApplication::isRightToLeft())
				return Qt::AlignLeft;
			break;
			
		default:
			break;
	}

	return align & ~Qt::AlignAbsolute;
}

static int begin(GB_DRAW *d)
{
	void *device = d->device;
	
	if (GB.Is(device, CLASS_Window))
	{
		MyMainWindow *win = (MyMainWindow *)((CWIDGET *)device)->widget;
		 // How to paint unclip ???
		if (init_drawing(d, win, win->width(), win->height()))
			return TRUE;
	}
	else if (GB.Is(device, CLASS_Picture))
	{
		CPICTURE *pict = (CPICTURE *)device;

		if (pict->pixmap->isNull())
		{
			GB.Error("Bad picture");
			return TRUE;
		}

		if (init_drawing(d, pict->pixmap, pict->pixmap->width(), pict->pixmap->height()))
			return TRUE;

		if (!pict->pixmap->mask().isNull())
		{
			QPen pen;
			QBrush brush;

			EXTRA(d)->mask = new QBitmap(pict->pixmap->mask());
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
		bool ret;

		if (wid->isCached())
			ret = init_drawing(d, wid->background(), wid->background()->width(), wid->background()->height());
		else if (wid->cache)
			ret = init_drawing(d, wid->cache, wid->width(), wid->height());
		else
			ret = init_drawing(d, wid, wid->width(), wid->height());
			
		if (ret)
			return TRUE;
		
		wid->drawn++;
	}
	
	//DP(d)->setRenderHint(QPainter::Antialiasing, true);
	
	return FALSE;
}

static void end(GB_DRAW *d)
{
	void *device = d->device;
	
	if (GB.Is(device, CLASS_Picture))
	{
		if (DPM(d))
		{
			DPM(d)->end();
			DP(d)->end();
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
				wid->refreshBackground();
	
			wid->drawn--;
		}
	}

	delete DP(d);
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
	return COLOR_TO_INT(DP(d)->background().color());
}

static void set_background(GB_DRAW *d, int col)
{
	col = get_color(d, col, true);
	
	DP(d)->setBackground(QColor((QRgb)col));
	if (DPM(d))
		DPM(d)->setBackground(MASK_COLOR(col));
}

static int get_foreground(GB_DRAW *d)
{
	return EXTRA(d)->fg; //COLOR_TO_INT(DP(d)->pen().color());
}

static void set_foreground(GB_DRAW *d, int col)
{
	QPen pen = DP(d)->pen();
	col = get_color(d, col, false);
	EXTRA(d)->fg = col;
	
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
	#if QT_VERSION >= QT_VERSION_CHECK(4, 5, 0)
	return DP(d)->compositionMode() == QPainter::RasterOp_SourceXorDestination; //QPainter::CompositionMode_Xor;
	#else
	return false;
	#endif
}

static void set_inverted(GB_DRAW *d, int inverted)
{
	#if QT_VERSION >= QT_VERSION_CHECK(4, 5, 0)
	DP(d)->setCompositionMode(inverted ? QPainter::RasterOp_SourceXorDestination : QPainter::CompositionMode_SourceOver);
	#else
	fprintf(stderr, "gb.qt4: warning: Draw.Invert needs Qt 4.5\n");
	#endif
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

static void get_picture_info(GB_DRAW *d, GB_PICTURE picture, GB_PICTURE_INFO *info)
{
	QPixmap *p = ((CPICTURE *)picture)->pixmap;
	
	info->width = p->width();
	info->height = p->height();
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
	DP(d)->setPen(QPen(QColor(EXTRA(d)->fg), pen.width(), (Qt::PenStyle)style));
	if (DPM(d))
		DPM(d)->setPen(QPen(MASK_COLOR(EXTRA(d)->fg), pen.width(), (Qt::PenStyle)style));
}

static int get_fill_color(GB_DRAW *d)
{
	return EXTRA(d)->fillColor; //COLOR_TO_INT(DP(d)->brush().color());
}

static void set_fill_color(GB_DRAW *d, int col)
{
	QBrush brush = DP(d)->brush();
	
	EXTRA(d)->fillColor = col;
	col = get_color(d, col, false);
	
	DP(d)->setBrush(QBrush(QColor((QRgb)col), brush.style()));
	if (DPM(d))
		DPM(d)->setBrush(QBrush(MASK_COLOR(col), brush.style()));
}

static int get_fill_style(GB_DRAW *d)
{
	return DP(d)->brush().style();
}

static void set_fill_style(GB_DRAW *d, int style)
{
	QBrush brush(QColor(EXTRA(d)->fillColor), (Qt::BrushStyle)style);
	DP(d)->setBrush(brush);
	if (DPM(d))
	{
		QBrush brushm(MASK_COLOR(EXTRA(d)->fillColor), (Qt::BrushStyle)style);
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

#define FIX_RECT(_d, _w, _h) \
	if (DP(_d)->pen().style() != Qt::NoPen) \
	{ \
		if (_w > 0) (_w--); \
		if (_h > 0) (_h--); \
	}

static void draw_rect(GB_DRAW *d, int x, int y, int w, int h)
{
	FIX_RECT(d, w, h);
	
	DP(d)->drawRect(x, y, w, h);
	if (DPM(d))
		DPM(d)->drawRect(x, y, w, h);
}

static void draw_ellipse(GB_DRAW *d, int x, int y, int w, int h, double start, double end)
{
	FIX_RECT(d, w, h);
	
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

#if 0
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
			//DPM(d)->setCompositionMode(QPainter::CompositionMode_SourceOver);
			DPM(d)->drawPixmap(x, y, p->mask(), sx, sy, sw, sh);
		}
		else
			DPM(d)->fillRect(x, y, sw, sh, Qt::color1);

		DPM(d)->restore();
	}
}

static void draw_image(GB_DRAW *d, GB_IMAGE image, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	bool xform;
	QImage *img = CIMAGE_get((CIMAGE *)image);

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
		QPixmap p = QPixmap::fromImage(*img);

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
			//DPM(d)->setRasterOp(Qt::OrROP);
			DPM(d)->drawPixmap(x, y, p.mask(), sx, sy, sw, sh);
		}
		else
			DPM(d)->fillRect(x, y, sw, sh, Qt::color1);

		DPM(d)->restore();
	}
	else
		DP(d)->drawImage(x, y, *img, sx, sy, sw, sh);

	DP(d)->restore();
}
#endif

static void draw_picture(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	bool xform;
	QPixmap *p = ((CPICTURE *)picture)->pixmap;

	DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, p->width(), p->height());
	xform = (w != sw || h != sh);

	if (!xform)
	{
		DP(d)->drawPixmap(x, y, *p, sx, sy, sw, sh);
		
		if (DPM(d))
		{
			if (p->hasAlpha())
			{
				//bitBlt(EXTRA(d)->mask, x, y, p->mask(), sx, sy, sw, sh, Qt::OrROP);
				qDebug("Draw.Picture() not implemented on transparent Picture");
			}
			else
				DPM(d)->fillRect(x, y, sw, sh, Qt::color1);
		}
	}
	else
	{
		if (DPM(d))
		{
			QImage img = p->convertToImage();
			
			if (sw < p->width() || sh < p->height())
				img = img.copy(sx, sy, sw, sh);
			
			if (w != sw || h != sh)
				img = img.smoothScale(w, h);
			
			DP(d)->drawImage(x, y, img);

			if (p->hasAlpha())
			{
				QBitmap m;
				m.convertFromImage(img.createAlphaMask());
				//bitBlt(EXTRA(d)->mask, x, y, &m, 0, 0, w, h, Qt::OrROP);
				qDebug("Draw.Picture() not implemented on transparent Picture");
			}
			else
				DPM(d)->fillRect(x, y, w, h, Qt::color1);
		}
		else
		{
			DP(d)->save();
			DP(d)->translate(x, y);
			x = y = 0;
			DP(d)->scale((double)w / p->width(), (double)h / p->height());
			DP(d)->drawPixmap(x, y, *p, sx, sy, sw, sh);
			DP(d)->restore();
		}
	}
}

static void draw_image(GB_DRAW *d, GB_IMAGE image, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	bool xform;
	QImage *p = CIMAGE_get((CIMAGE *)image);

	DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, p->width(), p->height());
	xform = (w != sw || h != sh);

	if (!xform)
	{
		DP(d)->drawImage(x, y, *p, sx, sy, sw, sh);
		
		if (DPM(d))
		{
			if (p->hasAlphaBuffer())
			{
				QBitmap m;
				m.convertFromImage(p->createAlphaMask());
				//bitBlt(EXTRA(d)->mask, x, y, &m, sx, sy, sw, sh, Qt::OrROP);
				qDebug("Draw.Image() not implemented on transparent Picture");
			}
			else
				DPM(d)->fillRect(x, y, sw, sh, Qt::color1);
		}
	}
	else
	{
		if (DPM(d))
		{
			QImage img = *p;
			
			if (sw < p->width() || sh < p->height())
				img = img.copy(sx, sy, sw, sh);
			
			if (w != sw || h != sh)
				img = img.smoothScale(w, h);
			
			DP(d)->drawImage(x, y, img);

			if (p->hasAlphaBuffer())
			{
				QBitmap m;
				m.convertFromImage(img.createAlphaMask());
				//bitBlt(EXTRA(d)->mask, x, y, &m, 0, 0, w, h, Qt::OrROP);
				qDebug("Draw.Image() not implemented on transparent Picture");
			}
			else
				DPM(d)->fillRect(x, y, w, h, Qt::color1);
		}
		else
		{
			DP(d)->save();
			DP(d)->translate(x, y);
			x = y = 0;
			DP(d)->scale((double)w / p->width(), (double)h / p->height());
			DP(d)->drawImage(x, y, *p, sx, sy, sw, sh);
			DP(d)->restore();
		}
	}
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
			//DPM(d)->setRasterOp(Qt::OrROP);
			DPM(d)->drawTiledPixmap(x, y, w, h, p->mask(), -DP(d)->brushOrigin().x(), -DP(d)->brushOrigin().y());
			DPM(d)->restore();
		}
		else
			DPM(d)->fillRect(x, y, w, h, Qt::color1);
	}
}

static QStringList text_sl;
static QVector<int> text_w;
static int text_line;

static int get_text_width(QPainter *dp, QString &s)
{
	int w, width = 0;
	int i;

	text_sl = s.split('\n', QString::KeepEmptyParts);

	text_w.resize(text_sl.count());

	for (i = 0; i < (int)text_sl.count(); i++)
	{
		w = dp->fontMetrics().width(text_sl[i]);
		if (w > width) width = w;
		text_w[i] = w;
	}

	return width;
}

static int get_text_height(QPainter *dp, QString &s)
{
	text_line = dp->fontMetrics().height();
	return text_line * (1 + s.count('\n'));
}

void DRAW_text(QPainter *p, const QString &text, float x, float y, float w, float h, int align, QPainter *p2)
{
	QPen pen, penm;
	QString t = text;
	int xx, ww;
	int tw, th;
	int i;

	tw = get_text_width(p, t);
	th = get_text_height(p, t);

	if (w < 0) w = tw;
	if (h < 0) h = th;
	
	y += p->fontMetrics().ascent();

	switch(align & Qt::AlignVertical_Mask)
	{
		case Qt::AlignBottom: y += h - th; break;
		case Qt::AlignVCenter: y += (h - th) / 2; break;
		default: break;
	}

	align = get_horizontal_alignment((Qt::Alignment)align);
	

	/*pen = DP(d)->pen();
	DP(d)->setPen(QColor(EXTRA(d)->fg));
	if (DPM(d))
	{
		penm = DPM(d)->pen();
		DPM(d)->setPen(Qt::color1);
	}*/

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

		//(*callback)(xx, y, t);
		p->drawText(xx, y, t);
		if (p2) 
			p2->drawText(xx, y, t);

		y += text_line;
	}
	
	/*DP(d)->setPen(pen);
	if (DPM(d)) DPM(d)->setPen(penm);*/
}


static void draw_text(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align)
{
	QPen pen, penm;

	pen = DP(d)->pen();
	DP(d)->setPen(QColor(EXTRA(d)->fg));
	if (DPM(d))
	{
		penm = DPM(d)->pen();
		DPM(d)->setPen(Qt::color1);
	}

	align = CCONST_alignment(align, ALIGN_TOP_NORMAL, true);
	DRAW_text(DP(d), QString::fromUtf8(text, len), x, y, w, h, align, DPM(d));
	
	DP(d)->setPen(pen);
	if (DPM(d)) DPM(d)->setPen(penm);
}

static void text_size(GB_DRAW *d, char *text, int len, int *w, int *h)
{
	QString s = QString::fromUtf8((const char *)text, len);  
	if (w) *w = get_text_width(DP(d), s);
	if (h) *h = get_text_height(DP(d), s);
}

void DRAW_rich_text(QPainter *p, const QString &text, float x, float y, float w, float h, int align, QPainter *p2)
{
	QString a;
	float tw, th;
	QString t = text;

	switch(get_horizontal_alignment((Qt::Alignment)align))
	{
		case Qt::AlignRight: a = "right"; break;
		case Qt::AlignHCenter: a = "center"; break;
		case Qt::AlignJustify: a = "justify"; break;
	}
	
	if (a.length())
		t = "<div align=\"" + a + "\">" + t + "</div>";
	
	QTextDocument rt;
	rt.setHtml(t);
	rt.setDefaultFont(p->font()); 
	
	if (w > 0)
		rt.setTextWidth(w);
		
	tw = rt.idealWidth();
	th = rt.size().height();

	if (w < 0) w = tw;
	if (h < 0) h = th;
	
	switch(align & Qt::AlignVertical_Mask)
	{
		case Qt::AlignBottom: y += h - th; break;
		case Qt::AlignVCenter: y += (h - th) / 2; break;
		default: break;
	}

	p->translate(x, y);
	rt.drawContents(p);
	p->translate(-x, -y);
	if (p2) 
	{
		p2->translate(x, y);
		rt.drawContents(p2);
		p2->translate(-x, -y);
	}
}

static void draw_rich_text(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align)
{
	QString t = QString::fromUtf8((const char *)text, len);
	
	if (align == GB_DRAW_ALIGN_DEFAULT)
		align = ALIGN_TOP_NORMAL;

	align = CCONST_alignment(align, ALIGN_TOP_NORMAL, true);

	DRAW_rich_text(DP(d), t, x, y, w, h, align, DPM(d));
}

static void rich_text_size(GB_DRAW *d, char *text, int len, int sw, int *w, int *h)
{
	QTextDocument rt;
	
	rt.setHtml(QString::fromUtf8((const char *)text, len));
	rt.setDefaultFont(DP(d)->font());
	
	if (sw > 0)
		rt.setTextWidth(sw);
	
	if (w) *w = rt.idealWidth();
	if (h) *h = rt.size().height();
}

static void draw_poly(GB_DRAW *d, bool fill, int n, int *points)
{
	int i, j;

	QPolygon p(n);

	for (i = 0, j = 0; i < n; i++, j += 2)
		p.setPoint(i, points[j], points[j + 1]);

	if (fill)
	{
		DP(d)->drawPolygon(p);
		if (DPM(d)) DPM(d)->drawPolygon(p);
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
	QRect r = DP(d)->clipRegion().boundingRect();
	
	if (x) *x = r.x();
	if (y) *y = r.y();
	if (w) *w = r.width();
	if (h) *h = r.height();
}

static void set_clipping(GB_DRAW *d, int x, int y, int w, int h)
{
	DP(d)->setClipRect(x, y, w, h);
	if DPM(d) DPM(d)->setClipRect(x, y, w, h);
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

static void init_option(QStyleOption &opt, int x, int y, int w, int h, int state)
{
	opt.rect = QRect(x, y, w ,h);
	if (state & GB_DRAW_STATE_DISABLED)
		return;
	
	opt.state |= QStyle::State_Enabled;
	
	if (state & GB_DRAW_STATE_FOCUS)
		opt.state |= QStyle::State_HasFocus;
	if (state & GB_DRAW_STATE_HOVER)
		opt.state |= QStyle::State_MouseOver;
	if (state & GB_DRAW_STATE_ACTIVE)
		opt.state |= QStyle::State_On | QStyle::State_Sunken | QStyle::State_Active;
}

static void paint_focus(GB_DRAW *d, int x, int y, int w, int h, int state)
{
	QStyleOptionFocusRect opt;
	
	if ((state & GB_DRAW_STATE_DISABLED) || !(state & GB_DRAW_STATE_FOCUS))
		return;
	
	init_option(opt, x, y, w, h, state);
	
	QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, DP(d));
	if (DPM(d))
		QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, DPM(d));
}

static void style_arrow(GB_DRAW *d, int x, int y, int w, int h, int type, int state)
{
	QStyleOption opt;
	QStyle::PrimitiveElement pe;
	
	init_option(opt, x, y, w, h, state);
	
	switch (type)
	{
		case ALIGN_NORMAL: pe = GB.System.IsRightToLeft() ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight; break;
		case ALIGN_LEFT: pe = QStyle::PE_IndicatorArrowLeft; break;
		case ALIGN_RIGHT: pe = QStyle::PE_IndicatorArrowRight; break;
		case ALIGN_TOP: pe = QStyle::PE_IndicatorArrowUp; break;
		case ALIGN_BOTTOM: pe = QStyle::PE_IndicatorArrowDown; break;
		default:
			return;
	}

	QApplication::style()->drawPrimitive(pe, &opt, DP(d));
	if (DPM(d)) 
	{
		//DPM(d)->setRasterOp(Qt::OrROP);
		//qDebug("arrow: %p %p %d", DPM(d), DPM(d)->device(), dynamic_cast<QBitmap *>(DPM(d)->device()) != NULL);
		QApplication::style()->drawPrimitive(pe, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
	}
}

static void style_check(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	QStyleOptionButton opt;
	init_option(opt, x, y, w, h, state);
	
	if (value)
	{
		if (value == 1)
			opt.state |= QStyle::State_NoChange;
		else
			opt.state |= QStyle::State_On;
	}
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, DP(d));
	if (DPM(d)) 
	{
		//DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
	}
	
	paint_focus(d, x, y, w, h, state);
}

static void style_option(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	QStyleOptionButton opt;
	init_option(opt, x, y, w, h, state);
	
	if (value)
		opt.state |= QStyle::State_On;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &opt, DP(d));
	if (DPM(d)) 
	{
		//DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
	}
	
	paint_focus(d, x, y, w, h, state);
}

static void style_separator(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	QStyleOption opt;
	init_option(opt, x, y, w, h, state);
	
	if (vertical)
		opt.state |= QStyle::State_Horizontal;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, DP(d));
	if (DPM(d)) 
	{
		//DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
	}
}


static void style_button(GB_DRAW *d, int x, int y, int w, int h, int value, int state, int flat)
{
	if (flat)
	{
		QStyleOptionToolButton opt;
		
		init_option(opt, x, y, w, h, state);
		
		//opt.state |= QStyle::State_Raised;
		
		if (value)
			opt.state |= QStyle::State_On;

		//opt.state &= ~QStyle::State_HasFocus;
		opt.state |= QStyle::State_AutoRaise;
		if (opt.state & QStyle::State_MouseOver)
			opt.state |= QStyle::State_Raised;
		
		if (opt.state & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver))
		{
			QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, DP(d));
			if (DPM(d)) 
				QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, DPM(d));
		}
	}
	else
	{
		QStyleOptionButton opt;
	
		init_option(opt, x, y, w, h, state);
		
		opt.state |= QStyle::State_Raised;
		
		if (value)
			opt.state |= QStyle::State_On;
	
		QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, DP(d));
		if (DPM(d)) 
			QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, DPM(d));
	}
	
	paint_focus(d, x + 3, y + 3, w - 6, h - 6, state);
}
			
static void style_panel(GB_DRAW *d, int x, int y, int w, int h, int border, int state)
{
	QStyleOptionFrame opt;
	init_option(opt, x, y, w, h, state);
	
	CCONTAINER_draw_frame(DP(d), border, opt);
	if (DPM(d))
		CCONTAINER_draw_frame(DPM(d), border, opt);
	
	/*opt.lineWidth = 2;
	opt.midLineWidth = 0;	
	
	if (border == BORDER_NONE)
		return;
		
	if (border == BORDER_PLAIN)
	{
		DP(d)->save();
		DP(d)->setPen(state == GB_DRAW_STATE_DISABLED ? QApplication::palette().color(QPalette::Active, QPalette::WindowText) : QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
		DP(d)->drawRect(x, y, w - 1, h - 1);
		DP(d)->restore();
		if DPM(d) 
		{	
			DPM(d)->save();
			DPM(d)->setPen(Qt::color1);
			DPM(d)->drawRect(x, y, w - 1, h - 1);
			DPM(d)->restore();
		}
		return;
	}
	
	if (border == BORDER_ETCHED)
	{
		pe = QStyle::PE_FrameGroupBox;
	}
	else
	{
		if (border == BORDER_RAISED)
			opt.state |= QStyle::State_Raised;
		else if (border == BORDER_SUNKEN)
			opt.state |= QStyle::State_Sunken;
		
		pe = QStyle::PE_Frame;
	}
	
	QApplication::style()->drawPrimitive(pe, &opt, DP(d));
	if (DPM(d)) 
	{
		//DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style()->drawPrimitive(pe, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
	}*/
}
			
static void style_handle(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	QStyleOption opt;
	init_option(opt, x, y, w, h, state);
	
	if (!vertical)
		opt.state |= QStyle::State_Horizontal;
	
	// Workaround a style bug
	//DP(d)->translate(x, y);
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, DP(d));
	//DP(d)->translate(-x, -y);
	if (DPM(d)) 
	{
		//DPM(d)->translate(x, y);
		//DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
		//DPM(d)->translate(-x, -y);
	}

	paint_focus(d, x, y, w, h, state);
}


static void style_box(GB_DRAW *d, int x, int y, int w, int h, int state)
{
	QStyleOptionFrame opt;
	
	init_option(opt, x, y, w, h, state);

	opt.lineWidth = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt);
	opt.midLineWidth = 0;
	opt.state |= QStyle::State_Sunken;
	//opt.features = QStyleOptionFrameV2::None;
	
  QApplication::style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, DP(d));
	if (DPM(d)) 
		QApplication::style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, DPM(d));
	
	//paint_focus(d, x, y, w, h, state);
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
	get_picture_info,
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
		style_button,
		style_panel,
		style_handle,
		style_box
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

void DRAW_aligned_pixmap(QPainter *p, const QPixmap &pix, int x, int y, int w, int h, int align)
{
	int xp, yp;
	
	if (pix.isNull() || pix.width() == 0 || pix.height() == 0)
		return;
	
	xp = x;
	switch(get_horizontal_alignment((Qt::Alignment)align))
	{
		case Qt::AlignRight: xp += w - pix.width(); break;
		case Qt::AlignHCenter: xp += (w - pix.width()) / 2; break;
		default: break;
	}
	
	yp = y;
	switch(align & Qt::AlignVertical_Mask)
	{
		case Qt::AlignBottom: yp += h - pix.height(); break;
		case Qt::AlignVCenter: yp += (h - pix.height()) / 2; break;
		default: break;
	}
	
	p->drawPixmap(xp, yp, pix);
}

void DRAW_clip(int x, int y, int w, int h)
{
	GB_DRAW *d = DRAW.GetCurrent();
	if (d) set_clipping(d, x, y, w, h);
}
