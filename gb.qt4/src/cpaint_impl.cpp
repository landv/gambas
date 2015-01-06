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

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QVector>
#include <QTextDocument>

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
#include "cprinter.h"
#include "csvgimage.h"
#include "cpaint_impl.h"

/*class ClipInfo
{
public:
	QPainterPath *path;
	GB_RECT *rect;
	
	ClipInfo() { path = NULL; rect = NULL; }
	~ClipInfo() { delete path; delete rect; }
};*/

typedef
	struct {
		QPainter *painter;
		QPainterPath *path;
		int fillRule;
		QTransform *init;
		float bx, by;
		//QPainterPath *clip;
		//GB_RECT *clipRect;
		//QList<ClipInfo *> *clipStack;
	}
	QT_PAINT_EXTRA;
	
#define EXTRA(d) ((QT_PAINT_EXTRA *)d->extra)

#define COLOR_TO_INT(color) ((color).rgba() ^ 0xFF000000)
#define MASK_COLOR(col) ((col & 0xFF000000) ? Qt::color0 : Qt::color1)

#define PAINTER(d) EXTRA(d)->painter
#define PATH(d) EXTRA(d)->path
//#define CLIP(d) EXTRA(d)->clip

static inline qreal to_deg(float angle)
{
	return (qreal)(angle * 180 / M_PI);
}

static bool init_painting(GB_PAINT *d, QPaintDevice *device)
{
	QPen pen;
	
	d->width = device->width();
	d->height = device->height();
	d->resolutionX = device->physicalDpiX();
	d->resolutionY = device->physicalDpiY();
	
	if (!PAINTER(d)) 
	{
		if (device->paintingActive())
		{
			GB.Error("Device already being painted");
			return TRUE;
		}
		
		EXTRA(d)->painter = new QPainter(device);
	}
	
	MyPaintEngine *engine = (MyPaintEngine *)device->paintEngine();
	engine->patchFeatures();
	
	//EXTRA(d)->path = NULL;
	//EXTRA(d)->clip = NULL;
	//EXTRA(d)->clipRect = NULL;
	
	EXTRA(d)->init = new QTransform();
	*(EXTRA(d)->init) = PAINTER(d)->worldTransform();
	
	PAINTER(d)->setRenderHints(QPainter::Antialiasing, true);
	PAINTER(d)->setRenderHints(QPainter::TextAntialiasing, true);
	PAINTER(d)->setRenderHints(QPainter::SmoothPixmapTransform, true);
	
	pen = PAINTER(d)->pen();
	pen.setCapStyle(Qt::FlatCap);
	pen.setJoinStyle(Qt::MiterJoin);
	pen.setMiterLimit(10.0);
	pen.setWidthF(1.0);
	PAINTER(d)->setPen(pen);
	PAINTER(d)->setBrush(Qt::black);
	
	return FALSE;
}

static QColor get_color(GB_PAINT *d, GB_COLOR col)
{
	if (col == GB_COLOR_DEFAULT)
	{
		if (GB.Is(d->device, CLASS_Control))
			col = CWIDGET_get_real_background((CWIDGET *)d->device);
		else
			col = 0xFFFFFF;
	}

	return CCOLOR_make(col);
}

/*static void begin_clipping(GB_PAINT *d)
{
	if (CLIP(d))
	{
		GB_RECT *r;
		QTransform save = PAINTER(d)->worldTransform();
		PAINTER(d)->resetTransform();
		r = EXTRA(d)->clipRect;
		if (r)
			PAINTER(d)->setClipRect(r->x, r->y, r->w, r->h);
		else
			PAINTER(d)->setClipPath(*CLIP(d));
		PAINTER(d)->setWorldTransform(save);
	}
}

static void end_clipping(GB_PAINT *d)
{
	if (CLIP(d))
		PAINTER(d)->setClipping(false);
}*/

#define begin_clipping(_d)
#define end_clipping(_d)

//---------------------------------------------------------------------------

static int Begin(GB_PAINT *d)
{
	void *device = d->device;
	QPaintDevice *target = NULL;
	
	if (GB.Is(device, CLASS_Picture))
	{
		QPixmap *pixmap = ((CPICTURE *)device)->pixmap;
		
		if (pixmap->isNull())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		
		target = pixmap;
	}
	else if (GB.Is(device, CLASS_Image))
	{
		QImage *image = CIMAGE_get((CIMAGE *)device);
		
		if (image->isNull())
		{
			GB.Error("Bad image");
			return TRUE;
		}
		
		target = image;
	}
	else if (GB.Is(device, CLASS_DrawingArea))
	{
		MyDrawingArea *wid;
		
		wid = (MyDrawingArea *)(((CWIDGET *)device)->widget);

		if (wid->isCached())
			target = wid->getBackgroundPixmap();
		else if (wid->cache)
			target = wid->cache;
		else
		{
			if (!wid->inDrawEvent())
			{
				GB.Error("Cannot paint outside of Draw event handler");
				return TRUE;
			}
			
			target = wid;
		}
			
		wid->drawn++;
		
		if (init_painting(d, target))
			return TRUE;
		
		if (wid->isCached())
			PAINTER(d)->initFrom(wid);
		
		d->width = wid->width();
		d->height = wid->height();
		return FALSE;
	}
	else if (GB.Is(device, CLASS_Printer))
	{
		CPRINTER *printer = (CPRINTER *)device;
		
		if (!printer->printing)
		{
			GB.Error("Printer is not printing");
			return TRUE;
		}
		
		target = printer->printer;
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		CSVGIMAGE *svgimage = (CSVGIMAGE *)device;
		target = SVGIMAGE_begin(svgimage, &EXTRA(d)->painter);
		if (!target)
		{
			GB.Error("SvgImage size is not defined");
			return TRUE;
		}
	}
	
	return init_painting(d, target);
}

static void End(GB_PAINT *d)
{
	void *device = d->device;
	QT_PAINT_EXTRA *dx = EXTRA(d);

	if (GB.Is(device, CLASS_DrawingArea))
	{
		MyDrawingArea *wid;
		
		wid = (MyDrawingArea *)(((CWIDGET *)device)->widget);

		if (wid)
		{
			if (wid->isCached())
				wid->refreshBackground();
	
			wid->drawn--;
		}
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		PAINTER(d)->end(); // ??
	}

	/*if (dx->clipStack)
	{
		while (!dx->clipStack->isEmpty())
			delete dx->clipStack->takeLast();
		delete dx->clipStack;
	}*/
	
	delete dx->init;
	delete dx->path;
	//delete dx->clip;
	delete dx->painter;
}

static void Save(GB_PAINT *d)
{
	//QT_PAINT_EXTRA *dx = EXTRA(d);
	//ClipInfo *ci;
	
	PAINTER(d)->save();
	
	/*if (!dx->clipStack)
		dx->clipStack = new QList<ClipInfo *>;
	
	ci = new ClipInfo;
	if (dx->clip)
		ci->path = new QPainterPath(*dx->clip);
	if (dx->clipRect)
	{
		ci->rect = new GB_RECT;
		*ci->rect = *dx->clipRect;
	}
	
	dx->clipStack->append(ci);*/
}

static void Restore(GB_PAINT *d)
{
	//QT_PAINT_EXTRA *dx = EXTRA(d);
	
	PAINTER(d)->restore();
	
	/*if (dx->clipStack && !dx->clipStack->isEmpty())
	{
		ClipInfo *ci = dx->clipStack->takeLast();
		
		delete dx->clip;
		dx->clip = ci->path ? new QPainterPath(*(ci->path)) : NULL;
		
		delete dx->clipRect;
		if (ci->rect)
		{
			dx->clipRect = new GB_RECT;
			*dx->clipRect = *ci->rect;
		}
		else
			dx->clipRect = NULL;
		
		delete ci;
	}*/
}
		
static void Antialias(GB_PAINT *d, int set, int *antialias)
{
	if (set)
		PAINTER(d)->setRenderHint(QPainter::Antialiasing, *antialias);
	else
		*antialias = PAINTER(d)->testRenderHint(QPainter::Antialiasing) ? 1 : 0;
}

static void apply_font(QFont &font, void *object = 0)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();

	PAINTER(d)->setFont(font);
}

static void Font(GB_PAINT *d, int set, GB_FONT *font)
{
	if (set)
	{
		QFont f(*((CFONT *)(*font))->font);
		PAINTER(d)->setFont(f);

		// Strange bug of QT. Sometimes the font does not apply (cf. DrawTextShadow)
		if (f != PAINTER(d)->font())
		{
			f.fromString(f.toString());
			PAINTER(d)->setFont(f);
		}
	}
	else
		*font = CFONT_create(PAINTER(d)->font(), apply_font);
}

static void init_path(GB_PAINT *d)
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
}

#define CHECK_PATH(_d) \
	if (!PATH(_d)) \
		return; \
	else \
		init_path(_d);

#define PRESERVE_PATH(_d, _p) \
	if (!(_p)) \
	{ \
		delete PATH(_d); \
		EXTRA(_d)->path = 0; \
	}

#define CREATE_PATH(_d) \
	if (!PATH(_d)) \
		EXTRA(_d)->path = new QPainterPath();

/*static void delete_clip_rect(GB_PAINT *d)
{
	if (EXTRA(d)->clipRect)
	{
		delete EXTRA(d)->clipRect;
		EXTRA(d)->clipRect = NULL;
	}
}*/
	
static void Clip(GB_PAINT *d, int preserve)
{
	CHECK_PATH(d);

	PAINTER(d)->setClipPath(*PATH(d), PAINTER(d)->hasClipping() ? Qt::IntersectClip : Qt::ReplaceClip);
	
	/*QPainterPath path = PAINTER(d)->worldTransform().map(*PATH(d));
	
	if (CLIP(d))
		path = CLIP(d)->intersected(path);
	
	delete EXTRA(d)->clip;
	EXTRA(d)->clip = new QPainterPath(path);
	delete_clip_rect(d);*/
	
	PRESERVE_PATH(d, preserve);
}

static void ResetClip(GB_PAINT *d)
{
	/*delete CLIP(d);
	EXTRA(d)->clip = 0;
	delete_clip_rect(d);*/
	PAINTER(d)->setClipping(false);
}

static void get_path_extents(QPainterPath *path, GB_EXTENTS *ext, const QTransform &transform)
{
	if (!path)
	{
		ext->x1 = ext->x2 = ext->y1 = ext->y2 = 0.0;
		return;
	}
	
	QRectF rect = transform.inverted().mapRect(path->boundingRect());
	
	ext->x1 = (float)rect.left();
	ext->y1 = (float)rect.top();
	ext->x2 = (float)rect.right();
	ext->y2 = (float)rect.bottom();
}

static void ClipExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	/*GB_RECT *rect = EXTRA(d)->clipRect;
	if (rect)
	{
		ext->x1 = (float)rect->x;
		ext->y1 = (float)rect->y;
		ext->x2 = (float)(rect->x + rect->w);
		ext->y2 = (float)(rect->y + rect->h);
	}
	else*/
	QPainterPath path = PAINTER(d)->clipPath();
	get_path_extents(&path, ext, QTransform()); //PAINTER(d)->transform());
}

static void Fill(GB_PAINT *d, int preserve)
{
	CHECK_PATH(d);
	
	//if (!CLIP(d))
		PAINTER(d)->fillPath(*PATH(d), PAINTER(d)->brush());
	/*else
	{
		QPainterPath path = PAINTER(d)->worldTransform().inverted().map(*CLIP(d));
		path = path.intersected(*PATH(d));
		PAINTER(d)->fillPath(path, PAINTER(d)->brush());
	}*/
	
	PRESERVE_PATH(d, preserve);
}

static void Stroke(GB_PAINT *d, int preserve)
{
	CHECK_PATH(d);
	
	if (PAINTER(d)->pen().widthF() > 0.0)
	{
		//if (!CLIP(d))
			PAINTER(d)->strokePath(*PATH(d), PAINTER(d)->pen());
		/*else
		{
			QPainterPathStroker stroker;
			QPen pen = PAINTER(d)->pen();
			
			stroker.setCapStyle(pen.capStyle());
			stroker.setDashOffset(pen.dashOffset());
			stroker.setDashPattern(pen.dashPattern());
			stroker.setJoinStyle(pen.joinStyle());
			stroker.setMiterLimit(pen.miterLimit());
			stroker.setWidth(pen.widthF());
			
			QPainterPath path = PAINTER(d)->worldTransform().inverted().map(*CLIP(d));
			path = path.intersected(stroker.createStroke(*PATH(d)));
			PAINTER(d)->fillPath(path, PAINTER(d)->brush());
		}*/
	}
	
	PRESERVE_PATH(d, preserve);
}
		
static void PathExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	get_path_extents(PATH(d), ext, PAINTER(d)->transform());
}

static int PathContains(GB_PAINT *d, float x, float y)
{
	if (!PATH(d))
		return FALSE;

	QPointF point((qreal)x, (qreal)y);
	return PATH(d)->contains(point);
}

static void PathOutline(GB_PAINT *d, GB_PAINT_OUTLINE_CB cb)
{
	QPainterPath *p = PATH(d);
	int i, j;

	if (!p)
		return;

	QList<QPolygonF> qoutline = p->toSubpathPolygons(QTransform());

	for (i = 0; i < qoutline.count(); i++)
	{
		QPolygonF qpolygon = qoutline.at(i);

		for (j = 0; j < qpolygon.count(); j++)
		{
			QPointF qpoint = qpolygon.at(j);
			(*cb)(j == 0 ? GB_PAINT_PATH_MOVE : GB_PAINT_PATH_LINE, qpoint.x(), qpoint.y());
		}
	}
}

#define DASH_ZERO 0.0009765625

static void Dash(GB_PAINT *d, int set, float **dashes, int *count)
{
	QPen pen = PAINTER(d)->pen();

	if (set)
	{
		if (!*count)
			pen.setStyle(Qt::SolidLine);
		else
		{
			QVector<qreal> dv;
			qreal d;

			for (int i = 0; i < *count; i++)
			{
				d = (*dashes)[i];
				if (d == 0.0)
					d = DASH_ZERO;
				dv << (qreal)d;
			}
			pen.setStyle(Qt::CustomDashLine);
			pen.setDashPattern(dv);
		}
		PAINTER(d)->setPen(pen);
	}
	else
	{
		if (pen.style() == Qt::CustomDashLine)
		{
			QVector<qreal> dv = pen.dashPattern();
			float d;
			*count = dv.count();
			GB.Alloc(POINTER(dashes), sizeof(float) * *count);
			for (int i = 0; i < *count; i++)
			{
				d = (float)dv[i];
				if (d <= DASH_ZERO)
					d = 0.0;
				(*dashes)[i] = d;
			}
		}
		else
		{
			*count = 0;
			*dashes = NULL;
		}
	}
}

static void DashOffset(GB_PAINT *d, int set, float *offset)
{
	QPen pen = PAINTER(d)->pen();
	
	if (set)
	{
		pen.setDashOffset((qreal)*offset);
		PAINTER(d)->setPen(pen);
	}
	else
	{
		*offset = (float)pen.dashOffset();
	}
}

		
static void FillRule(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		EXTRA(d)->fillRule = *value;
	}
	else
		*value = EXTRA(d)->fillRule;
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
	QPen pen = PAINTER(d)->pen();
	
	if (set)
	{
		switch (*value)
		{
			case GB_PAINT_LINE_CAP_ROUND:
				pen.setCapStyle(Qt::RoundCap); break;
			case GB_PAINT_LINE_CAP_SQUARE:
				pen.setCapStyle(Qt::SquareCap); break;
			case GB_PAINT_LINE_CAP_BUTT:
			default:
				pen.setCapStyle(Qt::FlatCap);
		}
		PAINTER(d)->setPen(pen);
	}
	else
	{
		switch (pen.capStyle())
		{
			case Qt::RoundCap: *value = GB_PAINT_LINE_CAP_ROUND; break;
			case Qt::SquareCap: *value = GB_PAINT_LINE_CAP_SQUARE; break;
			case Qt::FlatCap: default: *value = GB_PAINT_LINE_CAP_BUTT;
		}
	}
}

static void LineJoin(GB_PAINT *d, int set, int *value)
{
	QPen pen = PAINTER(d)->pen();
	
	if (set)
	{
		switch (*value)
		{
			case GB_PAINT_LINE_JOIN_ROUND:
				pen.setJoinStyle(Qt::RoundJoin); break;
			case GB_PAINT_LINE_JOIN_BEVEL:
				pen.setJoinStyle(Qt::BevelJoin); break;
			case GB_PAINT_LINE_JOIN_MITER:
			default:
				pen.setJoinStyle(Qt::MiterJoin);
		}
		PAINTER(d)->setPen(pen);
	}
	else
	{
		switch (pen.joinStyle())
		{
			case Qt::RoundJoin: *value = GB_PAINT_LINE_JOIN_ROUND; break;
			case Qt::BevelJoin: *value = GB_PAINT_LINE_JOIN_BEVEL; break;
			case Qt::MiterJoin: default: *value = GB_PAINT_LINE_JOIN_MITER;
		}
	}
}

static void LineWidth(GB_PAINT *d, int set, float *value)
{
	QPen pen = PAINTER(d)->pen();
	if (set)
	{
		pen.setWidthF((qreal)*value);
		PAINTER(d)->setPen(pen);
	}
	else
		*value = (float)pen.widthF();
}

static void MiterLimit(GB_PAINT *d, int set, float *value)
{
	QPen pen = PAINTER(d)->pen();
	if (set)
	{
		pen.setMiterLimit((qreal)*value);
		PAINTER(d)->setPen(pen);
	}
	else
		*value = (float)pen.miterLimit();
}


static void Operator(GB_PAINT *d, int set, int *value)
{
	QPainter::CompositionMode mode;
	
	if (set)
	{
		switch (*value)
		{
			case GB_PAINT_OPERATOR_CLEAR: mode = QPainter::CompositionMode_Clear; break;
			case GB_PAINT_OPERATOR_SOURCE: mode = QPainter::CompositionMode_Source; break;
			case GB_PAINT_OPERATOR_IN: mode = QPainter::CompositionMode_SourceIn; break;
			case GB_PAINT_OPERATOR_OUT: mode = QPainter::CompositionMode_SourceOut; break;
			case GB_PAINT_OPERATOR_ATOP: mode = QPainter::CompositionMode_SourceAtop; break;
			case GB_PAINT_OPERATOR_DEST: mode = QPainter::CompositionMode_Destination; break;
			case GB_PAINT_OPERATOR_DEST_OVER: mode = QPainter::CompositionMode_DestinationOver; break;
			case GB_PAINT_OPERATOR_DEST_IN: mode = QPainter::CompositionMode_DestinationIn; break;
			case GB_PAINT_OPERATOR_DEST_OUT: mode = QPainter::CompositionMode_DestinationOut; break;
			case GB_PAINT_OPERATOR_DEST_ATOP: mode = QPainter::CompositionMode_DestinationAtop; break;
			case GB_PAINT_OPERATOR_XOR: mode = QPainter::CompositionMode_Xor; break;
			case GB_PAINT_OPERATOR_ADD: mode = QPainter::CompositionMode_Plus; break;
			case GB_PAINT_OPERATOR_SATURATE: mode = QPainter::CompositionMode_Multiply; break;
			case GB_PAINT_OPERATOR_OVER: default: mode = QPainter::CompositionMode_SourceOver;
		}
		PAINTER(d)->setCompositionMode(mode);
	}
	else
	{
		switch (PAINTER(d)->compositionMode())
		{
			case QPainter::CompositionMode_Clear: *value = GB_PAINT_OPERATOR_CLEAR; break;
			case QPainter::CompositionMode_Source: *value = GB_PAINT_OPERATOR_SOURCE; break;
			case QPainter::CompositionMode_SourceIn: *value = GB_PAINT_OPERATOR_IN; break;
			case QPainter::CompositionMode_SourceOut: *value = GB_PAINT_OPERATOR_OUT; break;
			case QPainter::CompositionMode_SourceAtop: *value = GB_PAINT_OPERATOR_ATOP; break;
			case QPainter::CompositionMode_Destination: *value = GB_PAINT_OPERATOR_DEST; break;
			case QPainter::CompositionMode_DestinationOver: *value = GB_PAINT_OPERATOR_DEST_OVER; break;
			case QPainter::CompositionMode_DestinationIn: *value = GB_PAINT_OPERATOR_DEST_IN; break;
			case QPainter::CompositionMode_DestinationOut: *value = GB_PAINT_OPERATOR_DEST_OUT; break;
			case QPainter::CompositionMode_DestinationAtop: *value = GB_PAINT_OPERATOR_DEST_ATOP; break;
			case QPainter::CompositionMode_Xor: *value = GB_PAINT_OPERATOR_XOR; break;
			case QPainter::CompositionMode_Plus: *value = GB_PAINT_OPERATOR_ADD; break;
			case QPainter::CompositionMode_Multiply: *value = GB_PAINT_OPERATOR_SATURATE; break;
			case QPainter::CompositionMode_SourceOver: default: *value = GB_PAINT_OPERATOR_OVER;
		}
	}
}


static void NewPath(GB_PAINT *d)
{
	PRESERVE_PATH(d, FALSE);
}

static void ClosePath(GB_PAINT *d)
{
	CHECK_PATH(d);
	PATH(d)->closeSubpath();
}

		
static void Arc(GB_PAINT *d, float xc, float yc, float radius, float angle, float length, bool pie)
{
	CREATE_PATH(d);

	QRectF rect;
	rect.setCoords((qreal)(xc - radius), (qreal)(yc - radius), (qreal)(xc + radius), (qreal)(yc + radius));
	
	angle = - angle;
	length = - length;
	
	if (pie)
		PATH(d)->moveTo(xc, yc);
	else
		PATH(d)->arcMoveTo(rect, to_deg(angle));
	
	PATH(d)->arcTo(rect, to_deg(angle), to_deg(length));

	if (pie)
		PATH(d)->closeSubpath();
}

static void Ellipse(GB_PAINT *d, float x, float y, float width, float height, float angle, float length, bool pie)
{
	CREATE_PATH(d);

	QRectF rect;
	rect.setCoords((qreal)x, (qreal)y, (qreal)x + width, (qreal)y + height);
	
	angle = - angle;
	length = - length;
	
	if (pie)
		PATH(d)->moveTo(x + width / 2, y + height / 2);
	else
		PATH(d)->arcMoveTo(rect, to_deg(angle));
	
	PATH(d)->arcTo(rect, to_deg(angle), to_deg(length));
	if (pie)
		//PATH(d)->lineTo(x + width / 2, y + height / 2);
		PATH(d)->closeSubpath();
}

static void Rectangle(GB_PAINT *d, float x, float y, float width, float height)
{
	CREATE_PATH(d);
	PATH(d)->addRect((qreal)x, (qreal)y, (qreal)width, (qreal)height);
}

static void ClipRect(GB_PAINT *d, int x, int y, int w, int h)
{
	//GB_RECT *rect;
	ResetClip(d);
	Rectangle(d, x, y, w, h);
	Clip(d, FALSE);
	
	/*rect = new GB_RECT;
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
	EXTRA(d)->clipRect = rect;*/
}
	
static void GetCurrentPoint(GB_PAINT *d, float *x, float *y)
{
	if (!PATH(d))
	{
		*x = 0;
		*y = 0;
		return;
	}
	
	QPointF pt = PATH(d)->currentPosition();
	*x = (float)pt.x();
	*y = (float)pt.y();
}

static void MoveTo(GB_PAINT *d, float x, float y)
{
	CREATE_PATH(d);
	PATH(d)->moveTo((qreal)x, (qreal)y);
}

static void LineTo(GB_PAINT *d, float x, float y)
{
	CREATE_PATH(d);
	PATH(d)->lineTo((qreal)x, (qreal)y);
}

static void CurveTo(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3)
{
	CREATE_PATH(d);
	PATH(d)->cubicTo(QPointF((qreal)x1, (qreal)y1), QPointF((qreal)x2, (qreal)y2), QPointF((qreal)x3, (qreal)y3));
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

static QPainterPath *_draw_path;
static float _draw_x, _draw_y;

static void draw_text(GB_PAINT *d, bool rich, const char *text, int len, float w, float h, int align, bool draw)
{
	QPointF pos;
	
	GetCurrentPoint(d, &_draw_x, &_draw_y);
	
	if (w < 0 && h < 0)
		_draw_y -= PAINTER(d)->fontMetrics().ascent();
		
	if (draw)
	{
		begin_clipping(d);
		
		if (rich)
			DRAW_rich_text(PAINTER(d), QString::fromUtf8(text, len), _draw_x, _draw_y, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		else
			DRAW_text(PAINTER(d), QString::fromUtf8(text, len), _draw_x, _draw_y, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		
		end_clipping(d);
	}
	else
	{
		CREATE_PATH(d);
	
		_draw_path = PATH(d);

		MyPaintDevice device;
		QPainter p(&device);
		
		p.setFont(PAINTER(d)->font());
		p.setPen(PAINTER(d)->pen());
		p.setBrush(PAINTER(d)->brush());
		
		if (rich)
			DRAW_rich_text(&p, QString::fromUtf8(text, len), 0, 0, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		else
			DRAW_text(&p, QString::fromUtf8(text, len), 0, 0, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		
		p.end();
		_draw_path = NULL;
	}
}

static void Text(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw)
{
	draw_text(d, false, text, len, w, h, align, draw);
}

static void RichText(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw)
{
	draw_text(d, true, text, len, w, h, align, draw);
}

static void get_text_extents(GB_PAINT *d, bool rich, const char *text, int len, GB_EXTENTS *ext, float width)
{
	QPainterPath path;
	MyPaintDevice device;
	QPainter p(&device);
	
	p.setFont(PAINTER(d)->font());
	_draw_path = &path;
	GetCurrentPoint(d, &_draw_x, &_draw_y);
	_draw_y -= PAINTER(d)->fontMetrics().ascent();
	
	if (rich)
		DRAW_rich_text(&p, QString::fromUtf8(text, len), 0, 0, width, -1, CCONST_alignment(ALIGN_TOP_NORMAL, ALIGN_TOP_NORMAL, true));	
	else
		DRAW_text(&p, QString::fromUtf8(text, len), 0, 0, -1, -1, CCONST_alignment(ALIGN_TOP_NORMAL, ALIGN_TOP_NORMAL, true));	
	
	p.end();
	
	get_path_extents(&path, ext, QTransform());
	_draw_path = NULL;
}

static void TextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext)
{
	get_text_extents(d, false, text, len, ext, -1);
}

static void RichTextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext, float width)
{
	get_text_extents(d, true, text, len, ext, width);
}

static void TextSize(GB_PAINT *d, const char *text, int len, float *w, float *h)
{
	QString s = QString::fromUtf8((const char *)text, len);  
	*w = get_text_width(PAINTER(d), s);
	*h = get_text_height(PAINTER(d), s);
}

static void RichTextSize(GB_PAINT *d, const char *text, int len, float sw, float *w, float *h)
{
	QTextDocument rt;
	
	rt.setDocumentMargin(0);
	rt.setHtml(QString::fromUtf8((const char *)text, len));
	rt.setDefaultFont(PAINTER(d)->font());
	
	if (sw > 0)
		rt.setTextWidth(sw);
	
	*w = rt.idealWidth();
	*h = rt.size().height();
}

static void Matrix(GB_PAINT *d, int set, GB_TRANSFORM matrix)
{
	QTransform *t = (QTransform *)matrix;
	
	if (set)
	{
		if (t)
			PAINTER(d)->setWorldTransform(*t);
		else
			PAINTER(d)->setWorldTransform(*EXTRA(d)->init);
	}
	else
		*t = PAINTER(d)->worldTransform();
}

		
static void SetBrush(GB_PAINT *d, GB_BRUSH brush)
{
	QBrush *b = (QBrush *)brush;
	PAINTER(d)->setBrush(*b);

	QPen p = PAINTER(d)->pen();
	p.setBrush(*b);
	PAINTER(d)->setPen(p);
	//PAINTER(d)->setBrushOrigin(QPointF((qreal)x, (qreal)y));
}

static void BrushOrigin(GB_PAINT *d, int set, float *x, float *y)
{
	if (set)
	{
		EXTRA(d)->bx = *x;
		EXTRA(d)->by = *y;
		PAINTER(d)->setBrushOrigin(*x, *y);
	}
	else
	{
		*x = EXTRA(d)->bx;
		*y = EXTRA(d)->by;
	}
}

static void Background(GB_PAINT *d, int set, GB_COLOR *color)
{
	if (set)
	{
		QBrush b(get_color(d, *color));
		SetBrush(d, (GB_BRUSH)&b);
	}
	else
	{
		*color = COLOR_TO_INT(PAINTER(d)->brush().color());
	}
}


static void Invert(GB_PAINT *d, int set, int *invert)
{
	if (set)
	{
		#if QT_VERSION >= QT_VERSION_CHECK(4, 5, 0)
		PAINTER(d)->setCompositionMode(*invert ? QPainter::RasterOp_SourceXorDestination : QPainter::CompositionMode_SourceOver);
		#else
		fprintf(stderr, "gb.qt4: warning: Draw.Invert needs Qt 4.5\n");
		#endif
	}
	else
	{
		#if QT_VERSION >= QT_VERSION_CHECK(4, 5, 0)
		*invert = PAINTER(d)->compositionMode() == QPainter::RasterOp_SourceXorDestination; //QPainter::CompositionMode_Xor;
		#else
		*invert = FALSE;
		#endif
	}
}

static void DrawImage(GB_PAINT *d, GB_IMAGE image, float x, float y, float w, float h, float opacity, GB_RECT *source)
{
	QImage *img = CIMAGE_get((CIMAGE *)image);
	QRectF rect(x, y, w, h);
	
	begin_clipping(d);
	
	PAINTER(d)->setOpacity(opacity);
	
	if (source)
	{
		bool smooth = PAINTER(d)->testRenderHint(QPainter::SmoothPixmapTransform);
		
		if (w >= source->w && h >= source->h && w == (int)w && h == (int)h && ((int)w % source->w) == 0 && ((int)h % source->h) == 0)
			PAINTER(d)->setRenderHint(QPainter::SmoothPixmapTransform, false);

		QRectF srect(source->x, source->y, source->w, source->h);
		PAINTER(d)->drawImage(rect, *img, srect);

		PAINTER(d)->setRenderHint(QPainter::SmoothPixmapTransform, smooth);
	}
	else
		PAINTER(d)->drawImage(rect, *img);
	
	PAINTER(d)->setOpacity(1.0);
	
	end_clipping(d);
}
		
static void DrawPicture(GB_PAINT *d, GB_PICTURE picture, float x, float y, float w, float h, GB_RECT *source)
{
	QPixmap *pix = ((CPICTURE *)picture)->pixmap;
	QRectF rect(x, y, w, h);
	QRectF srect;
	
	if (source)
		srect = QRectF(source->x, source->y, source->w, source->h);
	else
		srect = QRectF(0, 0, pix->width(), pix->height());

	begin_clipping(d);
	
	PAINTER(d)->drawPixmap(rect, *pix, srect);

	end_clipping(d);
}

static void GetPictureInfo(GB_PAINT *d, GB_PICTURE picture, GB_PICTURE_INFO *info)
{
	QPixmap *p = ((CPICTURE *)picture)->pixmap;
	
	info->width = p->width();
	info->height = p->height();
}

static void FillRect(GB_PAINT *d, float x, float y, float w, float h, GB_COLOR color)
{
	begin_clipping(d);
	PAINTER(d)->fillRect(x, y, w, h, get_color(d, color));
	end_clipping(d);
}

static void BrushFree(GB_BRUSH brush)
{
	delete (QBrush *)brush;
}

static void BrushColor(GB_BRUSH *brush, GB_COLOR color)
{
	QBrush *br = new QBrush(CCOLOR_make(color));
	*brush = (GB_BRUSH)br;
}

static void BrushImage(GB_BRUSH *brush, GB_IMAGE image)
{
	QImage img(*CIMAGE_get((CIMAGE *)image));
	
	img.detach();
	QBrush *br = new QBrush(img);
	*brush = (GB_BRUSH)br;
}

static void BrushLinearGradient(GB_BRUSH *brush, float x0, float y0, float x1, float y1, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	QLinearGradient gradient;
	int i;
	
	gradient.setStart((qreal)x0, (qreal)y0);
	gradient.setFinalStop((qreal)x1, (qreal)y1);
	
	for (i = 0; i < nstop; i++)
		gradient.setColorAt((qreal)positions[i], CCOLOR_make(colors[i]));

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT:
			gradient.setSpread(QGradient::RepeatSpread); break;
		case GB_PAINT_EXTEND_REFLECT:
			gradient.setSpread(QGradient::ReflectSpread); break;
		case GB_PAINT_EXTEND_PAD:
		default:
			gradient.setSpread(QGradient::PadSpread);
	}

	QBrush *br = new QBrush(gradient);
	*brush = br;
}

static void BrushRadialGradient(GB_BRUSH *brush, float cx, float cy, float r, float fx, float fy, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	QRadialGradient gradient;
	int i;
	
	gradient.setCenter((qreal)cx, (qreal)cy);
	gradient.setRadius((qreal)r);
	gradient.setFocalPoint((qreal)fx, (qreal)fy);
	
	for (i = 0; i < nstop; i++)
		gradient.setColorAt((qreal)positions[i], CCOLOR_make(colors[i]));

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT:
			gradient.setSpread(QGradient::RepeatSpread); break;
		case GB_PAINT_EXTEND_REFLECT:
			gradient.setSpread(QGradient::ReflectSpread); break;
		case GB_PAINT_EXTEND_PAD:
		default:
			gradient.setSpread(QGradient::PadSpread);
	}

	QBrush *br = new QBrush(gradient);
	*brush = br;
}

static void BrushMatrix(GB_BRUSH brush, int set, GB_TRANSFORM matrix)
{
	QBrush *b = (QBrush *)brush;
	QTransform *t = (QTransform *)matrix;
	
	if (set)
	{
		if (t)
			b->setTransform(*t);
		else
			b->setTransform(QTransform());
	}
	else
		*t = b->transform();
}

static void TransformCreate(GB_TRANSFORM *matrix)
{
	*matrix = (GB_TRANSFORM)(new QTransform());
}

static void TransformCopy(GB_TRANSFORM *matrix, GB_TRANSFORM copy)
{
	*matrix = (GB_TRANSFORM)(new QTransform(*(QTransform *)copy));
}

static void TransformDelete(GB_TRANSFORM *matrix)
{
	delete (QTransform *)*matrix;
	*matrix = 0;
}

static void TransformInit(GB_TRANSFORM matrix, float xx, float yx, float xy, float yy, float x0, float y0)
{
	QTransform *t = (QTransform *)matrix;
	QMatrix m((qreal)xx, (qreal)yx, (qreal)xy, (qreal)yy, (qreal)x0, (qreal)y0);
	QTransform t2(m);
	*t = t2;
}

static void TransformTranslate(GB_TRANSFORM matrix, float tx, float ty)
{
	QTransform *t = (QTransform *)matrix;
	t->translate((qreal)tx, (qreal)ty);
}

static void TransformScale(GB_TRANSFORM matrix, float sx, float sy)
{
	QTransform *t = (QTransform *)matrix;
	t->scale((qreal)sx, (qreal)sy);
}

static void TransformRotate(GB_TRANSFORM matrix, float angle)
{
	QTransform *t = (QTransform *)matrix;
	t->rotateRadians(-angle);
}

static int TransformInvert(GB_TRANSFORM matrix)
{
	QTransform *t  = (QTransform *)matrix;
	bool inv;
	QTransform it = t->inverted(&inv);
	if (inv)
	{
		*t = it;
		return FALSE;
	}
	else
		return TRUE;
}

static void TransformMultiply(GB_TRANSFORM matrix, GB_TRANSFORM matrix2)
{
	QTransform *t  = (QTransform *)matrix;
	QTransform *t2  = (QTransform *)matrix2;
	
	*t = *t * *t2;
}

static void TransformMap(GB_TRANSFORM matrix, double *x, double *y)
{
	qreal xx, yy;

	xx = *x;
	yy = *y;

	((QTransform *)matrix)->map(xx, yy, &xx, &yy);

	*x = xx;
	*y = yy;
}


GB_PAINT_DESC PAINT_Interface = 
{
	// Size of the GB_PAINT structure extra data
	sizeof(QT_PAINT_EXTRA),
	Begin,
	End,
	Save,
	Restore,
	Antialias,
	Font,
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

QPainter *PAINT_get_current()
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	return d ? PAINTER(d) : NULL;
}

void PAINT_get_current_point(float *x, float *y)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (!d)
		return;
	GetCurrentPoint(d, x, y);
}

void PAINT_clip(int x, int y, int w, int h)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (d)
		ClipRect(d, x, y, w, h);
}

/*************************************************************************/

MyPaintEngine::MyPaintEngine() : QPaintEngine(0) {}
MyPaintEngine::~MyPaintEngine() {}

void MyPaintEngine::patchFeatures()
{
	if (type() == PostScript || type() == Pdf)
	{
		QPaintEngine::PaintEngineFeatures f = QPaintEngine::AllFeatures;
		f &= (QPaintEngine::PorterDuff | QPaintEngine::PerspectiveTransform
						| QPaintEngine::ObjectBoundingModeGradients
						| QPaintEngine::LinearGradientFill
						| QPaintEngine::RadialGradientFill
						| QPaintEngine::ConicalGradientFill);
		//qWarning("warning: patching current paint engine");
		gccaps = f; //PerspectiveTransform;
	}
}

bool MyPaintEngine::begin(QPaintDevice *pdev) 
{
	setActive(true);
	return true; 
}

bool MyPaintEngine::end() 
{
	setActive(false);
	return true;
}

void MyPaintEngine::updateState(const QPaintEngineState &state) 
{
	//qDebug("MyPaintEngine::updateState: %04X", (int)state.state());
}

void MyPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
	//qDebug("MyPaintEngine::drawRects");
}

void MyPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
	//qDebug("MyPaintEngine::drawLines");
}

void MyPaintEngine::drawEllipse(const QRectF &r)
{
	//qDebug("MyPaintEngine::drawEllipse");
}

void MyPaintEngine::drawPath(const QPainterPath &path)
{
	//qDebug("MyPaintEngine::drawPath");
}

void MyPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
	//qDebug("MyPaintEngine::drawPoints");
}

void MyPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
	//qDebug("MyPaintEngine::drawPolygon");
}


void MyPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
	//qDebug("MyPaintEngine::drawPixmap");
}

void MyPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
	//qDebug("MyPaintEngine::drawTiledPixmap");
}

void MyPaintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags)
{
	//qDebug("MyPaintEngine::drawImage");
}


//QPoint MyPaintEngine::coordinateOffset() const;

MyPaintEngine::Type MyPaintEngine::type() const
{
	return QPaintEngine::User;
}

void MyPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
	//qDebug("MyPaintEngine::drawTextItem: %g %g [%s] '%s'", p.x() + _draw_x, p.y() + _draw_y, (const char *)textItem.font().toString().toUtf8(), (const char *)textItem.text().toUtf8());
	//_draw_path->moveTo(p.x() + _draw_x, p.y() + _draw_y);
	_draw_path->addText(p.x() + _draw_x + painter()->worldTransform().dx(), p.y() + _draw_y + painter()->worldTransform().dy(), textItem.font(), textItem.text());
}

/*************************************************************************/

MyPaintEngine MyPaintDevice::engine;

MyPaintDevice::MyPaintDevice() : QPaintDevice()
{
}

QPaintEngine *MyPaintDevice::paintEngine() const
{ 
	return &engine; 
}

int MyPaintDevice::metric(PaintDeviceMetric m) const
{
	QPaintDevice *d = PAINT_get_current()->device();

	switch(m)
	{
		case PdmWidth: return d->width();
		case PdmHeight: return d->height();
		case PdmWidthMM: return d->widthMM();
		case PdmHeightMM: return d->heightMM();
		#if QT_VERSION <= QT_VERSION_CHECK(4, 6, 0)
		case PdmNumColors: return d->numColors();
		#else
		case PdmNumColors: return d->colorCount();
		#endif
		case PdmDepth: return d->depth();
		case PdmDpiX: return d->logicalDpiX();
		case PdmDpiY: return d->logicalDpiY();
		case PdmPhysicalDpiX: return d->physicalDpiX();
		case PdmPhysicalDpiY: return d->physicalDpiY();
		default: return 0;
	}
}
